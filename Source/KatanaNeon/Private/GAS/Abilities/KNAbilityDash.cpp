// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/KNAbilityDash.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DataTable.h"
#include "Characters/Base/KNCharacterBase.h"
#include "GAS/Attributes/KNAttributeSet.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 기본 생성자 및 초기화 구현
UKNAbilityDash::UKNAbilityDash()
{
    FGameplayTagContainer TempTags;
    TempTags.AddTag(KatanaNeon::Ability::Combat::Dash);
    SetAssetTags(TempTags);

    // 무적 중 재대시 불가 — 연속 무적 남용 방지
    ActivationBlockedTags.AddTag(KatanaNeon::State::Combat::Invincible);

    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 핵심 오버라이드 구현
bool UKNAbilityDash::CanActivateAbility(
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

    // 에러 해결 1: ActionCostDataTable 대신 FDataTableRowHandle 검사
    if (!ActionCostRowHandle.DataTable) return false;

    // 하드코딩된 TEXT("Default") 대신 에디터에 세팅된 행 번호를 가져옴
    const FKNActionCostRow* CostRow = ActionCostRowHandle.GetRow<FKNActionCostRow>(TEXT("CanActivateDash"));
    if (!CostRow) return false;

    if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
    {
        if (const UKNAttributeSet* AttrSet = ASC->GetSet<UKNAttributeSet>())
        {
            return AttrSet->GetStamina() >= CostRow->DashStaminaCost;
        }
    }
    return false;
}

void UKNAbilityDash::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // ── 1. DataTable 비용 캐싱 ──
    if (!LoadActionCostRow())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── 2. 스태미나 소모 ──
    if (!ConsumeStamina())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 에러 해결 2: ACharacter가 아닌 우리 고유의 AKNCharacterBase로 캐스팅
    AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo());
    if (!Owner)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── 3. 무적 태그 부여 ──
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        GrantInvincible(ASC);
    }

    // ── 4. 대시 이동 ──
    LaunchDash(Owner);

    // ── 5. 대시 몽타주 재생 (옵션) ──
    if (DashMontageTable)
    {
        const FName RowName = GetDodgeDirectionRowName(Owner);
        if (const FKNDashMontageRow* Row = DashMontageTable->FindRow<FKNDashMontageRow>(RowName, TEXT("DashMontage")))
        {
            const bool bIsDrawn = GetAbilitySystemComponentFromActorInfo()
                ->HasMatchingGameplayTag(KatanaNeon::State::Combat::WeaponDrawn);

            UAnimMontage* MontageToPlay = bIsDrawn ? Row->DrawnMontage : Row->SheathMontage;
            if (MontageToPlay)
            {
                Owner->PlayAnimMontage(MontageToPlay);
            }
        }
    }

    // ── 6. 무적 만료 타이머 ──
    GetWorld()->GetTimerManager().SetTimer(
        InvincibleTimerHandle,
        this,
        &UKNAbilityDash::OnInvincibleExpired,
        InvincibleDuration,
        false);
}

void UKNAbilityDash::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(InvincibleTimerHandle);
    }

    // 태그가 남아있는 경우를 대비한 안전 제거
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::Invincible))
        {
            ASC->RemoveLooseGameplayTag(KatanaNeon::State::Combat::Invincible);
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion GAS 핵심 오버라이드 구현

#pragma region 내부 헬퍼 함수 구현
FName UKNAbilityDash::GetDodgeDirectionRowName(AKNCharacterBase* Character) const
{
    if (!Character) return TEXT("Forward");

    FVector Input = Character->GetLastMovementInputVector();
    if (Input.IsNearlyZero()) return TEXT("Back");

    Input.Z = 0.f;
    Input.Normalize();

    const float F = FVector::DotProduct(Input, Character->GetActorForwardVector());
    const float R = FVector::DotProduct(Input, Character->GetActorRightVector());

    if (F >= 0.71f)  return TEXT("Forward");
    if (F <= -0.71f)
    {
        if (R < -0.38f) return TEXT("BackLeft");
        if (R > 0.38f) return TEXT("BackRight");
        return TEXT("Back");
    }
    return (R < 0.f)
        ? ((F > 0.f) ? TEXT("ForwardLeft") : TEXT("BackLeft"))
        : ((F > 0.f) ? TEXT("ForwardRight") : TEXT("BackRight"));
}

bool UKNAbilityDash::LoadActionCostRow()
{
    // 에러 해결 1: ActionCostDataTable 대신 FDataTableRowHandle 검사
    if (!ActionCostRowHandle.DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[KNAbility_Dash] ActionCostRowHandle이 에디터에 할당되지 않았습니다!"));
        return false;
    }

    // 하드코딩 제거
    const FKNActionCostRow* Row = ActionCostRowHandle.GetRow<FKNActionCostRow>(TEXT("LoadDashCost"));

    if (!ensureAlwaysMsgf(Row, TEXT("[KNAbility_Dash] 지정된 ActionCost 행을 찾을 수 없습니다!")))
    {
        return false;
    }

    CachedActionCost = *Row;
    return true;
}

bool UKNAbilityDash::ConsumeStamina()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC || !StaminaCostGEClass) return false;

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle    SpecHandle = ASC->MakeOutgoingSpec(StaminaCostGEClass, 1.0f, Context);

    if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
    {
        // 캐싱된 비용(엑셀 데이터)만큼 차감
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Stamina, -CachedActionCost.DashStaminaCost);
        return ASC->ApplyGameplayEffectSpecToSelf(*Spec).IsValid();
    }
    return false;
}

void UKNAbilityDash::GrantInvincible(UAbilitySystemComponent* ASC)
{
    if (!ASC) return;
    // Loose 태그 방식: 타이머로 직접 제거하여 I-Frame 타이밍을 정밀하게 제어합니다.
    ASC->AddLooseGameplayTag(KatanaNeon::State::Combat::Invincible);
}

// 에러 해결 2: 헤더와 동일하게 파라미터를 AKNCharacterBase로 변경
void UKNAbilityDash::LaunchDash(AKNCharacterBase* Character)
{
    if (!Character) return;

    // 마지막 입력 방향 우선, 없으면 캐릭터 전방
    FVector Dir = Character->GetLastMovementInputVector();
    if (Dir.IsNearlyZero())
    {
        Dir = Character->GetActorForwardVector();
    }

    Dir.Z = 0.0f; // 수평 대시만 허용
    Dir = Dir.GetSafeNormal();

    // DashImpulse 변수 참조 오류 해결 완료
    Character->LaunchCharacter(Dir * DashImpulse, true, false);
}

void UKNAbilityDash::OnInvincibleExpired()
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::Invincible))
        {
            ASC->RemoveLooseGameplayTag(KatanaNeon::State::Combat::Invincible);
        }
    }

    EndAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo(),
        true, false);
}
#pragma endregion 내부 헬퍼 함수 구현
