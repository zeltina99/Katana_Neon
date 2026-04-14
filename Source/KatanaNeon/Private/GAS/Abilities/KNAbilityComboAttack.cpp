// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/KNAbilityComboAttack.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "GAS/Attributes/KNAttributeSet.h"
#include "GAS/Components/KNStatsComponent.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 기본 생성자 및 초기화 구현
UKNAbilityComboAttack::UKNAbilityComboAttack()
{
    // TryActivateAbilitiesByTag(Attack)으로 호출됩니다.
    FGameplayTagContainer TempTags;
    TempTags.AddTag(KatanaNeon::Ability::Combat::Attack);
    SetAssetTags(TempTags);

    // 연타 시 어빌리티가 강제 종료되는 현상을 방지합니다.
    // 활성 중 입력은 Controller → BufferNextInput 경로로 처리합니다.
    //bRetriggerInstancedAbility = false;

    // 콤보 상태(Step)는 인스턴스 멤버로 관리하므로 InstancedPerActor 필수
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 핵심 오버라이드 구현
bool UKNAbilityComboAttack::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    // 현재 스탠스에 맞는 DataTable 존재 여부 확인
    const bool bDrawn = IsWeaponDrawn(ActorInfo);
    if (!GetComboDataTable(bDrawn))
    {
        UE_LOG(LogTemp, Warning, TEXT("[ComboAttack] %s 스탠스 DataTable이 할당되지 않았습니다."),
            bDrawn ? TEXT("발도") : TEXT("납도"));
        return false;
    }

    // 콤보 윈도우 진행 중이면 다음 입력으로 바로 허용
    if (bComboWindowOpen) return true;

    // 1단계 시작 전 스태미나 사전 검사
    const FName FirstRow = MakeComboRowName(1, EKNComboAttackType::Light);
    const FKNComboAttackRow* TestRow =
        GetComboDataTable(bDrawn)->FindRow<FKNComboAttackRow>(FirstRow, TEXT("CanActivate"));
    if (TestRow == nullptr) return false;

    if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
    {
        if (const UKNAttributeSet* AttrSet = ASC->GetSet<UKNAttributeSet>())
        {
            return AttrSet->GetStamina() >= TestRow->StaminaCost;
        }
    }
    return false;
}

void UKNAbilityComboAttack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // ── 윈도우가 열려 있으면 다음 콤보 단계로 진행 ──
    if (bComboWindowOpen)
    {
        bComboWindowOpen = false;
        GetWorld()->GetTimerManager().ClearTimer(ComboWindowTimerHandle);
        AdvanceCombo();
        return;
    }

    // ★ 몽타주 재생 중(윈도우 전)에 입력이 들어오면 버퍼에 저장
    if (CurrentComboStep > 0)
    {
        bInputBuffered = true;
        bBufferedInputIsHeavy = false; // 좌클릭이므로 Light
        return;
    }

    // ── 1단계 시작: 이 시점의 스탠스를 콤보 끝까지 고정 ──
    bIsDrawnCombo = IsWeaponDrawn(ActorInfo);
    CurrentComboStep = 1;
    CurrentAttackType = bNextIsHeavy ? EKNComboAttackType::Heavy : EKNComboAttackType::Light;
    bNextIsHeavy = false;

    const FName RowName = MakeComboRowName(CurrentComboStep, CurrentAttackType);
    if (!LoadComboRow(RowName))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!ConsumeStamina())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!PlayComboMontage())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }
}

void UKNAbilityComboAttack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    CurrentComboStep = 0;
    bComboWindowOpen = false;
    bNextIsHeavy = false;
    bIsDrawnCombo = false;
    bInputBuffered = false;
    bBufferedInputIsHeavy = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ComboWindowTimerHandle);
    }

    // [핵심 해결책] 어빌리티 종료 시 남아있는 태스크를 안전하게 정리합니다.
    if (CurrentMontageTask != nullptr)
    {
        CurrentMontageTask->EndTask();
        CurrentMontageTask = nullptr;
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion GAS 핵심 오버라이드 구현

#pragma region 블루프린트 / AnimNotify 연동 인터페이스 구현
void UKNAbilityComboAttack::BufferNextInput(bool bIsHeavy)
{
    // 강공격(Heavy) 상태일 때 다시 강공격이 들어오는 것은 차단합니다.
    if (bIsHeavy && CurrentAttackType == EKNComboAttackType::Heavy)
    {
        return;
    }

    if (bComboWindowOpen)
    {
        bComboWindowOpen = false;
        GetWorld()->GetTimerManager().ClearTimer(ComboWindowTimerHandle);
        if (bIsHeavy && CurrentAttackType == EKNComboAttackType::Light)
        {
            bNextIsHeavy = true;
        }
        AdvanceCombo();
        return;
    }

    bInputBuffered = true;
    bBufferedInputIsHeavy = bIsHeavy;
}
void UKNAbilityComboAttack::ActivateHitbox()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC || !DamageGEClass) return;

    ACharacter* Owner = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Owner) return;

    // ── 히트박스 구체 오버랩 판정 ──
    TArray<FOverlapResult> Overlaps;
    const FVector HitCenter = Owner->GetActorLocation()
        + Owner->GetActorForwardVector() * 100.0f; // 전방 100cm

    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        HitCenter,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(80.0f));

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (!HitActor || HitActor == Owner) continue;

        UAbilitySystemComponent* TargetASC = HitActor->FindComponentByClass<UAbilitySystemComponent>();
        if (!TargetASC) continue;

        // ── 데미지 GE 적용 (SetByCaller: Health = -실제데미지) ──
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        Context.AddInstigator(Owner, Owner);

        FGameplayEffectSpecHandle DmgSpec = ASC->MakeOutgoingSpec(DamageGEClass, 1.0f, Context);
        if (FGameplayEffectSpec* Spec = DmgSpec.Data.Get())
        {
            // 오버클럭 1단계 배율
            float TacticalMultiplier = 1.0f;
            if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::OverclockTactical))
            {
                if (const FKNOverclockLv1Row* Lv1Row = OverclockLv1RowHandle.GetRow<FKNOverclockLv1Row>(TEXT("GetTacticalMultiplier")))
                {
                    TacticalMultiplier = Lv1Row->DamageMultiplier;
                }
            }

            // 오버클럭 3단계(시간 정지) 배율
            float FrozenMultiplier = 1.0f;
            if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::WorldTimeFrozen))
            {
                if (const FKNOverclockLv3Row* Lv3Row = OverclockLv3RowHandle.GetRow<FKNOverclockLv3Row>(TEXT("GetFrozenMultiplier")))
                {
                    FrozenMultiplier = Lv3Row->FrozenDamageMultiplier;
                }
            }

            // 기획자의 엑셀 데이터 3개가 곱해져서 만들어진 궁극의 최종 데미지!
            const float FinalDamage = BaseAttackDamage * CachedComboRow.DamageMultiplier * TacticalMultiplier * FrozenMultiplier;
            Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Health, -FinalDamage);

            TargetASC->ApplyGameplayEffectSpecToSelf(*Spec);
        }

        if (UKNStatsComponent* Stats = Owner->FindComponentByClass<UKNStatsComponent>())
        {
            Stats->GainOverclockPoint(CachedComboRow.OverclockGain);
        }
    }
}

void UKNAbilityComboAttack::OpenComboWindow()
{
    // Heavy 공격은 항상 피니셔 — 윈도우 없이 즉시 종료
    if (CurrentAttackType == EKNComboAttackType::Heavy)
    {
        OnComboWindowExpired();
        return;
    }

    // ComboWindowTime == 0.0f : Light 피니셔(5단계) → 즉시 종료
    if (FMath::IsNearlyZero(CachedComboRow.ComboWindowTime))
    {
        OnComboWindowExpired();
        return;
    }

    // ★ 버퍼링된 입력이 있으면 즉시 다음 콤보로 진행
    if (bInputBuffered)
    {
        bInputBuffered = false;
        if (bBufferedInputIsHeavy && CurrentAttackType == EKNComboAttackType::Light)
        {
            bNextIsHeavy = true;
        }
        bBufferedInputIsHeavy = false;
        AdvanceCombo();
        return;
    }

    bComboWindowOpen = true;
    GetWorld()->GetTimerManager().SetTimer(
        ComboWindowTimerHandle,
        this,
        &UKNAbilityComboAttack::OnComboWindowExpired,
        CachedComboRow.ComboWindowTime,
        false);
}

void UKNAbilityComboAttack::RequestHeavyAttack()
{
    if (bComboWindowOpen && CurrentAttackType == EKNComboAttackType::Light)
    {
        bNextIsHeavy = true;
    }
    // 윈도우 전 Heavy 입력도 버퍼링
    else if (CurrentComboStep > 0 && !bComboWindowOpen && CurrentAttackType == EKNComboAttackType::Light)
    {
        bInputBuffered = true;
        bBufferedInputIsHeavy = true;
    }
}
void UKNAbilityComboAttack::PrepareHeavyStart()
{
    // 콤보가 비활성일 때만 세팅 (활성 중이면 RequestHeavyAttack 사용)
    if (!IsActive())
    {
        bNextIsHeavy = true;
    }
}
#pragma endregion 블루프린트 / AnimNotify 연동 인터페이스 구현

#pragma region 내부 헬퍼 함수 구현
UDataTable* UKNAbilityComboAttack::GetComboDataTable(bool bDrawn) const
{
    return bDrawn ? DrawnComboDataTable.Get() : SheathComboDataTable.Get();
}

bool UKNAbilityComboAttack::IsWeaponDrawn(const FGameplayAbilityActorInfo* ActorInfo) const
{
    if (!ActorInfo) return false;

    const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
    if (!ASC) return false;

    return ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::WeaponDrawn);
}

FName UKNAbilityComboAttack::MakeComboRowName(int32 Step, EKNComboAttackType AttackType) const
{
    // 최적화: FString 힙 할당 없이 static 배열로 즉시 반환
    static const FName LightNames[] = {
        TEXT("LightAttack_1"), TEXT("LightAttack_2"), TEXT("LightAttack_3"),
        TEXT("LightAttack_4"), TEXT("LightAttack_5")
    };
    static const FName HeavyNames[] = {
        TEXT("HeavyAttack_1"), TEXT("HeavyAttack_2"), TEXT("HeavyAttack_3"),
        TEXT("HeavyAttack_4"), TEXT("HeavyAttack_5")
    };

    const int32 SafeIdx = FMath::Clamp(Step - 1, 0, 4);
    return (AttackType == EKNComboAttackType::Light) ? LightNames[SafeIdx] : HeavyNames[SafeIdx];
}

bool UKNAbilityComboAttack::LoadComboRow(const FName& RowName)
{
    UDataTable* Table = GetComboDataTable(bIsDrawnCombo);
    if (!Table)
    {
        UE_LOG(LogTemp, Error, TEXT("[ComboAttack] %s 스탠스 DataTable이 없습니다."),
            bIsDrawnCombo ? TEXT("발도") : TEXT("납도"));
        return false;
    }

    const FKNComboAttackRow* Row =
        Table->FindRow<FKNComboAttackRow>(RowName, TEXT("LoadComboRow"));

    if (!ensureAlwaysMsgf(Row,
        TEXT("[ComboAttack] 행을 찾을 수 없음: %s (스탠스: %s)"),
        *RowName.ToString(), bIsDrawnCombo ? TEXT("발도") : TEXT("납도")))
    {
        return false;
    }

    CachedComboRow = *Row;
    return true;
}

bool UKNAbilityComboAttack::ConsumeStamina()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC || !StaminaCostGEClass) return false;

    // 사전 잔액 검사
    if (const UKNAttributeSet* AttrSet = ASC->GetSet<UKNAttributeSet>())
    {
        if (AttrSet->GetStamina() < CachedComboRow.StaminaCost)
        {
            UE_LOG(LogTemp, Warning, TEXT("[KNAbility_ComboAttack] 스태미나 부족: 필요=%.1f, 현재=%.1f"),
                CachedComboRow.StaminaCost, AttrSet->GetStamina());
            return false;
        }
    }

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StaminaCostGEClass, 1.0f, Context);

    if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
    {
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Stamina, -CachedComboRow.StaminaCost);
        // 음수 Delta → 스태미나 소모
        ASC->ApplyGameplayEffectSpecToSelf(*Spec);
        return true;
    }
    return false;
}

bool UKNAbilityComboAttack::PlayComboMontage()
{
    // 미해제 시 L1 몽타주 종료가 EndAbility를 잘못 호출합니다.
    if (CurrentMontageTask != nullptr)
    {
        CurrentMontageTask->OnCompleted.RemoveAll(this);
        CurrentMontageTask->OnInterrupted.RemoveAll(this);
        CurrentMontageTask->OnCancelled.RemoveAll(this);
        CurrentMontageTask = nullptr;
    }

    // ★ 몽타주는 DataTable 행의 ComboMontage 필드에서 직접 읽음
    UAnimMontage* MontageToPlay = CachedComboRow.ComboMontage;
    if (!MontageToPlay)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[ComboAttack] ComboMontage가 DataTable에 할당되지 않았습니다. Step:%d Type:%d Stance:%s"),
            CurrentComboStep, CurrentAttackType, bIsDrawnCombo ? TEXT("발도") : TEXT("납도"));
        return false;
    }

    // PlayRate = DT 행의 PlayRate × AttributeSet.AttackSpeed (오버클럭 배속 연동)
    float FinalPlayRate = CachedComboRow.PlayRate;
    if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (const UKNAttributeSet* Attrs = ASC->GetSet<UKNAttributeSet>())
        {
            FinalPlayRate *= Attrs->GetAttackSpeed();
        }
    }

    UAbilityTask_PlayMontageAndWait* Task =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, MontageToPlay, FinalPlayRate, NAME_None, false);

    Task->OnCompleted.AddDynamic(this, &UKNAbilityComboAttack::OnMontageEnded);
    Task->OnInterrupted.AddDynamic(this, &UKNAbilityComboAttack::OnMontageEnded);
    Task->OnCancelled.AddDynamic(this, &UKNAbilityComboAttack::OnMontageEnded);
    Task->ReadyForActivation();
    CurrentMontageTask = Task;
    return true;
}

void UKNAbilityComboAttack::AdvanceCombo()
{
    constexpr int32 MaxComboStep = 5;
    ++CurrentComboStep;

    // 다음 단계가 Heavy로 예약되었는지 확인
    if (bNextIsHeavy)
    {
        CurrentAttackType = EKNComboAttackType::Heavy;
        bNextIsHeavy = false;
    }

    if (CurrentComboStep > MaxComboStep)
    {
        OnComboWindowExpired();
        return;
    }

    const FName RowName = MakeComboRowName(CurrentComboStep, CurrentAttackType);
    if (!LoadComboRow(RowName))
    {
        OnComboWindowExpired();
        return;
    }

    if (!ConsumeStamina())
    {
        OnComboWindowExpired();
        return;
    }

    PlayComboMontage();
}

void UKNAbilityComboAttack::OnComboWindowExpired()
{
    bComboWindowOpen = false;
}

void UKNAbilityComboAttack::OnMontageEnded()
{
    EndAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo(),
        true, false);
}
#pragma endregion 내부 헬퍼 함수 구현
