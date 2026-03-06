// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "KNAbilityLockOn.generated.h"

#pragma region 전방 선언
class AKNPlayerCharacter;
#pragma endregion 전방 선언

/**
 * @file    KNAbilityLockOn.h
 * @class   UKNAbilityLockOn
 * @brief   플레이어의 락온(Lock-On) 상태를 토글하는 어빌리티입니다.
 * * @details
 * [SRP 책임]
 * 컨트롤러의 입력을 받아 플레이어 캐릭터의 락온 상태(이동 방향 및 회전 제어권)를 전환하고 즉시 종료됩니다.
 */
UCLASS()
class KATANANEON_API UKNAbilityLockOn : public UGameplayAbility
{
    GENERATED_BODY()

#pragma region 초기화 및 설정
public:
    /** @brief 어빌리티 기본 생성자. 인스턴싱 및 네트워크 정책을 설정합니다. */
    UKNAbilityLockOn();
#pragma endregion 초기화 및 설정

#pragma region 어빌리티 생명주기
protected:
    /**
     * @brief 어빌리티가 활성화될 때 호출되어 락온 상태를 토글합니다.
     * @param Handle 어빌리티 핸들
     * @param ActorInfo 어빌리티 실행 액터 정보
     * @param ActivationInfo 활성화 정보
     * @param TriggerEventData 트리거 이벤트 데이터
     */
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
#pragma endregion 어빌리티 생명주기
};