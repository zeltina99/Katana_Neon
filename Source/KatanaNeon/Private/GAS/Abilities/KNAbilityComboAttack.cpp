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
    AbilityTags.AddTag(KatanaNeon::Ability::Combat::Attack);

    // 사망 상태에서 콤보 시작 불가
    // ActivationBlockedTags.AddTag(/* State.Dead 태그 */);

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

    // 콤보 윈도우 진행 중이면 다음 입력으로 허용 (스태미나 재검사는 AdvanceCombo에서 수행)
    if (bComboWindowOpen)
    {
        return true;
    }

    // 최초 1단계: 스태미나 사전 조회
    if (!ComboDataTable) return false;

    const FName FirstRow = MakeComboRowName(1, 0); // 기본은 Light 1단계로 검사
    const FKNComboAttackRow* TestRow = ComboDataTable->FindRow<FKNComboAttackRow>(FirstRow, TEXT("CanActivate"));
    if (!TestRow) return false;

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

    // ── 1단계 시작 ──
    CurrentComboStep = 1;
    CurrentAttackType = bNextIsHeavy ? 1 : 0;
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

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ComboWindowTimerHandle);
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion GAS 핵심 오버라이드 구현

#pragma region 블루프린트 / AnimNotify 연동 인터페이스 구현
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
            const float FinalDamage = BaseAttackDamage * CachedComboRow.DamageMultiplier;
            Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Health, -FinalDamage);
            TargetASC->ApplyGameplayEffectSpecToSelf(*Spec);
        }

        // ── 오버클럭 포인트 획득 — KNStatsComponent에 위임 (SRP) ──
        if (UKNStatsComponent* Stats = Owner->FindComponentByClass<UKNStatsComponent>())
        {
            Stats->GainOverclockPoint(CachedComboRow.OverclockGain);
        }
    }
}

void UKNAbilityComboAttack::OpenComboWindow()
{
    // ComboWindowTime == 0.0f : 피니셔 공격 → 즉시 종료
    if (FMath::IsNearlyZero(CachedComboRow.ComboWindowTime))
    {
        OnComboWindowExpired();
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
    // 윈도우가 열려 있을 때만 다음 단계를 Heavy로 예약
    if (bComboWindowOpen)
    {
        bNextIsHeavy = true;
    }
}
#pragma endregion 블루프린트 / AnimNotify 연동 인터페이스 구현

#pragma region 내부 헬퍼 함수 구현
FName UKNAbilityComboAttack::MakeComboRowName(int32 Step, int32 AttackType) const
{
    const FString Prefix = (AttackType == 0) ? TEXT("LightAttack") : TEXT("HeavyAttack");
    return FName(*FString::Printf(TEXT("%s_%d"), *Prefix, Step));
}

bool UKNAbilityComboAttack::LoadComboRow(const FName& RowName)
{
    if (!ComboDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[KNAbility_ComboAttack] ComboDataTable이 에디터에 할당되지 않았습니다!"));
        return false;
    }

    const FKNComboAttackRow* Row = ComboDataTable->FindRow<FKNComboAttackRow>(RowName, TEXT("LoadComboRow"));
    if (!ensureAlwaysMsgf(Row, TEXT("[KNAbility_ComboAttack] 행을 찾을 수 없음: %s"), *RowName.ToString()))
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
        // 음수 Delta → 스태미나 소모
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Stamina, -CachedComboRow.StaminaCost);
        return ASC->ApplyGameplayEffectSpecToSelf(*Spec).IsValid();
    }
    return false;
}

bool UKNAbilityComboAttack::PlayComboMontage()
{
    TArray<TObjectPtr<UAnimMontage>>& MontageArray =
        (CurrentAttackType == 0) ? LightAttackMontages : HeavyAttackMontages;

    const int32 MontageIdx = CurrentComboStep - 1; // 배열 인덱스: 0~4
    if (!MontageArray.IsValidIndex(MontageIdx) || !MontageArray[MontageIdx])
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[KNAbility_ComboAttack] 몽타주 미할당 Step=%d Type=%d"),
            CurrentComboStep, CurrentAttackType);
        return false;
    }

    UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this,
        NAME_None,
        MontageArray[MontageIdx],
        1.0f,
        NAME_None,
        false);

    // 몽타주 완전 종료 시 어빌리티도 종료
    Task->OnCompleted.AddDynamic(this, &UKNAbilityComboAttack::OnComboWindowExpired);
    Task->OnInterrupted.AddDynamic(this, &UKNAbilityComboAttack::OnComboWindowExpired);
    Task->OnCancelled.AddDynamic(this, &UKNAbilityComboAttack::OnComboWindowExpired);
    Task->ReadyForActivation();
    return true;
}

void UKNAbilityComboAttack::AdvanceCombo()
{
    constexpr int32 MaxComboStep = 5;
    ++CurrentComboStep;

    // 다음 단계가 Heavy로 예약되었는지 확인
    if (bNextIsHeavy)
    {
        CurrentAttackType = 1;
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

    EndAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo(),
        true, false);
}
#pragma endregion 내부 헬퍼 함수 구현
