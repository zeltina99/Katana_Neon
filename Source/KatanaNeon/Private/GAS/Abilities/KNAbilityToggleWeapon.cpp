// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/KNAbilityToggleWeapon.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 어빌리티 초기화 구현
UKNAbilityToggleWeapon::UKNAbilityToggleWeapon()
{
    // 어빌리티 실행 태그 등록 (C++ 컨트롤러에서 이 태그로 어빌리티를 호출합니다)
    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(KatanaNeon::Ability::Combat::ToggleWeapon);
    SetAssetTags(TagContainer);

    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}
#pragma endregion 어빌리티 초기화 구현

#pragma region 어빌리티 실행 구현
void UKNAbilityToggleWeapon::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 1. 현재 무기를 들고 있는지(WeaponDrawn 태그 존재 여부) 검사
    bool bIsDrawn = ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::WeaponDrawn);

    // 2. 상태에 맞춰 재생할 몽타주 동적 선택 (하드코딩 분기문 배제)
    UAnimMontage* MontageToPlay = bIsDrawn ? SheathMontage : DrawMontage;

    if (MontageToPlay)
    {
        // 3. GAS 비동기 태스크(Task)를 이용한 몽타주 재생
        UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, MontageToPlay, 1.0f);

        // 몽타주 재생이 끝나거나 취소되면 어빌리티도 깔끔하게 종료되도록 델리게이트 연결
        Task->OnCompleted.AddDynamic(this, &UKNAbilityToggleWeapon::K2_EndAbility);
        Task->OnInterrupted.AddDynamic(this, &UKNAbilityToggleWeapon::K2_EndAbility);
        Task->OnCancelled.AddDynamic(this, &UKNAbilityToggleWeapon::K2_EndAbility);

        Task->ReadyForActivation();
    }
    else
    {
        // 기획자가 블루프린트에서 몽타주 할당을 깜빡했을 경우 안전하게 종료
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }
}
#pragma endregion 어빌리티 실행 구현