// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"
#include "KNDurationModifier.generated.h"

#pragma region 전방 선언
class UAbilitySystemComponent;
#pragma endregion 전방 선언

/**
 * @file    KNDurationModifier.h
 * @brief   시간 제한(Duration) 어트리뷰트 수정 GE 및 실행 계산 클래스입니다.
 * * [사용 예시]
 * - 오버클럭 x1 전술기로 인한 일시적 이동속도 증가 버프
 * - 시간이 지나면 GAS 엔진에 의해 완벽하게 자동 롤백(해제)되는 디버프
 * * [데이터 주도적 설계]
 * - 에디터에서 하드코딩된 Modifier를 쓰지 않고, SetByCaller 태그 매핑을 통해
 * 어떤 스탯이든 유연하게 조작할 수 있습니다.
 */
#pragma region Duration Gameplay Effect 클래스
 /**
  * @class  UKNDurationModifier
  * @brief  HasDuration 정책(Policy)을 가지는 전용 GE 래퍼 클래스입니다.
  * @details 단일 책임 원칙에 따라 Duration 세팅과 Execution 연결만 담당합니다.
  */
UCLASS()
class KATANANEON_API UKNDurationModifier : public UGameplayEffect
{
    GENERATED_BODY()

public:
    /** @brief GE 정책 초기화 (HasDuration 지정 및 Execution 클래스 자동 등록) */
    UKNDurationModifier();
};
#pragma endregion Duration Gameplay Effect 클래스

// ────────────────────────────────────────────────────────────

#pragma region Execution Calculation 클래스
/**
 * @class  UKNDurationModifierExecution
 * @brief  UKNDurationModifier의 실제 스탯 증감 연산을 담당하는 실행 계산 클래스입니다.
 * @details 런타임 성능을 극대화하기 위해 리플렉션 조회를 피하고 정적 캐싱(Static Caching)을 사용합니다.
 */
UCLASS()
class KATANANEON_API UKNDurationModifierExecution : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

public:
    /** @brief AttributeSet의 모든 어트리뷰트를 캡처 목록에 등록하여 데이터 기반 수정을 준비합니다. */
    UKNDurationModifierExecution();

    /**
     * @brief GE 실행 로직 – SetByCaller로 전달받은 태그를 캐싱된 어트리뷰트 맵에서 찾아 Additive 연산을 수행합니다.
     * @param ExecutionParams GE 실행 컨텍스트 및 파라미터
     * @param OutExecutionOutput 최종적으로 적용될 Modifier 결과를 담는 출력 객체
     */
    virtual void Execute_Implementation(
        const FGameplayEffectCustomExecutionParameters& ExecutionParams,
        FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
    /** * @brief  태그-어트리뷰트 매핑 성능을 극대화하기 위한 내부 초기화 헬퍼 함수입니다.
     * @return 최초 1회만 생성된 캐시 맵을 반환합니다.
     */
    static const TMap<FGameplayTag, FGameplayAttribute>& GetCachedAttributeMap();
};
#pragma endregion Execution Calculation 클래스
