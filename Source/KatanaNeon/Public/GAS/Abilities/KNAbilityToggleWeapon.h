// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "KNAbilityToggleWeapon.generated.h"

#pragma region 전방 선언
class UAnimMontage;
#pragma endregion 전방 선언

/**
 * @class   UKNAbilityToggleWeapon
 * @brief   플레이어의 발도(Draw) 및 납도(Sheath)를 처리하는 어빌리티입니다.
 * @details 현재 장착 상태 태그를 확인하여 알맞은 몽타주를 재생하는 단일 책임을 가집니다.
 */
UCLASS()
class KATANANEON_API UKNAbilityToggleWeapon : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
    UKNAbilityToggleWeapon();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

protected:
#pragma region 애니메이션 데이터
    /** @brief 발도 시 재생할 애니메이션 몽타주 (에디터에서 기획자가 직접 할당) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Animation")
    TObjectPtr<UAnimMontage> DrawMontage = nullptr;

    /** @brief 납도 시 재생할 애니메이션 몽타주 (에디터에서 기획자가 직접 할당) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Animation")
    TObjectPtr<UAnimMontage> SheathMontage = nullptr;
#pragma endregion 애니메이션 데이터
};
