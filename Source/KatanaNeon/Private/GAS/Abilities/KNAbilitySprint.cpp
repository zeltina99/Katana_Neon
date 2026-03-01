// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/KNAbilitySprint.h"
#include "AbilitySystemComponent.h"
#include "Framework/System/KNDataManagerSubsystem.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 기본 생성자 및 초기화 구현
UKNAbilitySprint::UKNAbilitySprint()
{
    // 어빌리티 식별 태그 등록
    FGameplayTagContainer TempTags;
    TempTags.AddTag(KatanaNeon::Ability::Movement::Sprint);
    SetAssetTags(TempTags);

    // 달리기 상태를 인스턴스에 유지해야 하므로 Instanced 필수
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 핵심 오버라이드 구현
bool UKNAbilitySprint::CanActivateAbility(
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

    // 허공(점프 중)이거나 그로기 상태일 때는 달리기 불가 등의 추가 조건을 여기에 작성 가능합니다.
    return true;
}

void UKNAbilitySprint::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    UKNDataManagerSubsystem* DataManager = GetWorld()->GetGameInstance()->GetSubsystem<UKNDataManagerSubsystem>();

    if (!ASC || !SprintBuffGEClass || !DataManager)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── 베테랑의 최적화: 서브시스템을 통한 O(1) 데이터 조회 및 동적 차이값(Delta) 계산 ──
    if (const FKNBaseStatRow* BaseStat = DataManager->GetPlayerBaseStat(TEXT("Default")))
    {
        // 목표 속도(1000) - 기본 속도(700) = 증가시킬 속도(+300)
        // 이렇게 해야 추후 오버클럭의 배율 연산 등과 완벽하게 호환됩니다.
        const float SpeedDelta = BaseStat->SprintSpeed - BaseStat->MovementSpeed;

        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SprintBuffGEClass, 1.0f, Context);

        if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
        {
            Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::MovementSpeed, SpeedDelta);
            ActiveSprintGEHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);
        }
    }
}

void UKNAbilitySprint::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // 어빌리티 종료 시 적용했던 이동 속도 증가 버프를 걷어냅니다.
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (ActiveSprintGEHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(ActiveSprintGEHandle);
            ActiveSprintGEHandle.Invalidate();
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion GAS 핵심 오버라이드 구현