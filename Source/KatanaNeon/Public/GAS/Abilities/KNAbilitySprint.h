// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "KNAbilitySprint.generated.h"

#pragma region 전방 선언
class UGameplayEffect;
#pragma endregion 전방 선언

/**
 * @file    KNAbilitySprint.h
 * @class   UKNAbilitySprint
 * @brief   플레이어의 달리기(Sprint) 상태를 관리하는 토글형 어빌리티입니다.
 * @details 데이터 매니저를 통해 엑셀의 달리기 속도를 읽어오고,
 * 활성화 시 이동 속도 증가 버프를 부여하며 종료 시 회수합니다. (단일 책임 원칙 준수)
 */
UCLASS()
class KATANANEON_API UKNAbilitySprint : public UGameplayAbility
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    UKNAbilitySprint();
#pragma endregion 기본 생성자 및 초기화

#pragma region GAS 핵심 오버라이드
public:
    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr,
        const FGameplayTagContainer* TargetTags = nullptr,
        OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;
#pragma endregion GAS 핵심 오버라이드

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 이동 속도를 올려줄 Infinite Gameplay Effect 클래스.
     * @details 에디터에서 SetByCaller(MovementSpeed)를 지원하는 범용 GE를 할당합니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Sprint")
    TSubclassOf<UGameplayEffect> SprintBuffGEClass = nullptr;
#pragma endregion 에디터 설정 데이터

#pragma region 런타임 상태 관리
private:
    /** @brief 활성화된 달리기 버프를 추적하고 해제하기 위한 핸들 */
    FActiveGameplayEffectHandle ActiveSprintGEHandle;
#pragma endregion 런타임 상태 관리
};
