// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"
#include "KNInfiniteModifier.generated.h"

#pragma region 전방 선언
class UAbilitySystemComponent;
#pragma endregion 전방 선언

/**
 * @file    KNInfiniteModifier.h
 * @brief   영구(Infinite) 지속 어트리뷰트 수정 GE 및 실행 계산 클래스입니다.
 * @details
 * [사용 예시]
 * - 사이버웨어 장비 장착으로 인한 영구적인 스탯(최대 체력, 공격력 등) 증가
 * - 패시브 스킬로 인한 기본 이동 속도 향상
 * * [주의사항]
 * - 이 이펙트는 시간이 지나도 사라지지 않으므로, 해제하려면 반드시
 * 발급받은 FActiveGameplayEffectHandle을 통해 ASC에서 직접 Remove 해야 합니다.
 */
#pragma region Infinite Gameplay Effect 클래스
 /**
  * @class  UKNInfiniteModifier
  * @brief  Infinite Duration 정책(Policy)을 가지는 전용 GE 래퍼 클래스입니다.
  * @details 단일 책임 원칙에 따라 생명주기(Infinite) 세팅과 Execution 연결만 담당합니다.
  */
UCLASS()
class KATANANEON_API UKNInfiniteModifier : public UGameplayEffect
{
    GENERATED_BODY()

public:
    /** @brief GE 정책 초기화 (Infinite 지정 및 Execution 클래스 자동 등록) */
    UKNInfiniteModifier();
};
#pragma endregion Infinite Gameplay Effect 클래스

// ────────────────────────────────────────────────────────────

#pragma region Execution Calculation 클래스
/**
 * @class  UKNInfiniteModifierExecution
 * @brief  UKNInfiniteModifier의 실제 스탯 증감 연산을 담당하는 실행 계산 클래스입니다.
 * @details 런타임 성능 최적화를 위해 실시간 리플렉션을 배제하고 정적 캐싱 맵을 활용합니다.
 */
UCLASS()
class KATANANEON_API UKNInfiniteModifierExecution : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

public:
    /** @brief AttributeSet의 모든 어트리뷰트를 캡처 목록에 등록하여 수정을 준비합니다. */
    UKNInfiniteModifierExecution();

    /**
     * @brief GE 실행 로직 – SetByCaller로 전달받은 태그를 캐시 맵에서 찾아 Additive 연산을 수행합니다.
     * @param ExecutionParams GE 실행 컨텍스트 및 파라미터
     * @param OutExecutionOutput 최종적으로 적용될 Modifier 결과를 담는 출력 객체
     */
    virtual void Execute_Implementation(
        const FGameplayEffectCustomExecutionParameters& ExecutionParams,
        FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
#pragma endregion Execution Calculation 클래스
