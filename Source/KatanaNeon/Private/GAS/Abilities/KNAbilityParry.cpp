// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/KNAbilityParry.h"
#include "AbilitySystemComponent.h"
#include "Characters/Base/KNCharacterBase.h" // ACharacter 교체
#include "GameFramework/WorldSettings.h"
#include "Engine/DataTable.h"
#include "GAS/Attributes/KNAttributeSet.h"
#include "GAS/Components/KNStatsComponent.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 기본 생성자 및 초기화 구현
UKNAbilityParry::UKNAbilityParry()
{
    // ── UE 5.5 경고 제거 규약 ──
    FGameplayTagContainer TempTags;
    TempTags.AddTag(KatanaNeon::Ability::Combat::Parry);
    SetAssetTags(TempTags);

    ActivationBlockedTags.AddTag(KatanaNeon::State::Combat::FlurryRush);

    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 핵심 오버라이드 구현
bool UKNAbilityParry::CanActivateAbility(
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

    // 하드코딩 제거: RowHandle 검사
    if (!ActionCostRowHandle.DataTable) return false;

    const FKNActionCostRow* CostRow = ActionCostRowHandle.GetRow<FKNActionCostRow>(TEXT("CanActivateParry"));
    if (!CostRow) return false;

    if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
    {
        if (const UKNAttributeSet* AttrSet = ASC->GetSet<UKNAttributeSet>())
        {
            return AttrSet->GetStamina() >= CostRow->ParryStaminaCost;
        }
    }
    return false;
}

void UKNAbilityParry::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!ConsumeStamina())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        // O(1) 네이티브 태그 사용
        ASC->AddLooseGameplayTag(KatanaNeon::State::Combat::Parrying);
    }

    AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo());
    if (Owner && ParryMontage)
    {
        Owner->PlayAnimMontage(ParryMontage);
    }

    GetWorld()->GetTimerManager().SetTimer(
        ParryWindowTimerHandle,
        this,
        &UKNAbilityParry::OnParryWindowExpired,
        PerfectParryWindowTime,
        false);
}

void UKNAbilityParry::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ParryWindowTimerHandle);
        World->GetTimerManager().ClearTimer(FlurryRushTimerHandle);
    }

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        // O(1) 네이티브 태그 사용
        if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::Parrying))
        {
            ASC->RemoveLooseGameplayTag(KatanaNeon::State::Combat::Parrying);
        }
    }

    if (bIsFlurryRush)
    {
        if (UWorld* World = GetWorld())
        {
            if (AWorldSettings* WS = World->GetWorldSettings())
            {
                WS->SetTimeDilation(1.0f);
            }
        }

        if (AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo()))
        {
            Owner->CustomTimeDilation = 1.0f;
        }

        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
        {
            if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::FlurryRush))
            {
                ASC->RemoveLooseGameplayTag(KatanaNeon::State::Combat::FlurryRush);
            }
        }
        bIsFlurryRush = false;
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion GAS 핵심 오버라이드 구현

#pragma region 외부 호출 인터페이스 구현
void UKNAbilityParry::OnEnemyAttackWarningReceived()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    // O(1) 네이티브 태그 사용
    if (!ASC || !ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::Parrying)) return;

    GetWorld()->GetTimerManager().ClearTimer(ParryWindowTimerHandle);
    OnPerfectParry();
}
#pragma endregion 외부 호출 인터페이스 구현

#pragma region 내부 헬퍼 함수 구현
bool UKNAbilityParry::ConsumeStamina()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC || !StaminaCostGEClass || !ActionCostRowHandle.DataTable) return false;

    const FKNActionCostRow* CostRow = ActionCostRowHandle.GetRow<FKNActionCostRow>(TEXT("ParryConsumeStamina"));
    if (!CostRow) return false;

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle    SpecHandle = ASC->MakeOutgoingSpec(StaminaCostGEClass, 1.0f, Context);

    if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
    {
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Stamina, -CostRow->ParryStaminaCost);
        return ASC->ApplyGameplayEffectSpecToSelf(*Spec).IsValid();
    }
    return false;
}

void UKNAbilityParry::OnPerfectParry()
{
    float ParryGain = 50.0f;
    if (OverclockSettingRowHandle.DataTable)
    {
        if (const FKNOverclockSettingRow* OCRow = OverclockSettingRowHandle.GetRow<FKNOverclockSettingRow>(TEXT("PerfectParryGain")))
        {
            ParryGain = OCRow->GainPerfectParry;
        }
    }

    AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo());
    if (Owner)
    {
        if (UKNStatsComponent* Stats = Owner->FindComponentByClass<UKNStatsComponent>())
        {
            Stats->GainOverclockPoint(ParryGain);
        }

        if (PerfectParryMontage)
        {
            Owner->PlayAnimMontage(PerfectParryMontage);
        }
    }

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        // O(1) 네이티브 태그 사용
        if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::Parrying))
        {
            ASC->RemoveLooseGameplayTag(KatanaNeon::State::Combat::Parrying);
        }

        ASC->AddLooseGameplayTag(KatanaNeon::State::Combat::FlurryRush);
    }

    bIsFlurryRush = true;
    ActivateFlurryRushSlowMotion();

    GetWorld()->GetTimerManager().SetTimer(
        FlurryRushTimerHandle,
        this,
        &UKNAbilityParry::DeactivateFlurryRush,
        FlurryRushDuration,
        false);
}

void UKNAbilityParry::ActivateFlurryRushSlowMotion()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (AWorldSettings* WS = World->GetWorldSettings())
    {
        WS->SetTimeDilation(FlurrySlowMotionScale);
    }

    if (AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo()))
    {
        Owner->CustomTimeDilation = 1.0f / FlurrySlowMotionScale;
    }
}

void UKNAbilityParry::DeactivateFlurryRush()
{
    if (UWorld* World = GetWorld())
    {
        if (AWorldSettings* WS = World->GetWorldSettings())
        {
            WS->SetTimeDilation(1.0f);
        }
    }

    if (AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo()))
    {
        Owner->CustomTimeDilation = 1.0f;
    }

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::FlurryRush))
        {
            ASC->RemoveLooseGameplayTag(KatanaNeon::State::Combat::FlurryRush);
        }
    }

    bIsFlurryRush = false;

    EndAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo(),
        true, false);
}

void UKNAbilityParry::OnParryWindowExpired()
{
    EndAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo(),
        true, false);
}
#pragma endregion 내부 헬퍼 함수 구현
