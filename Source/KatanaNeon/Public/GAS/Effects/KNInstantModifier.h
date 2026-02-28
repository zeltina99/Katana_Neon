// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"
#include "KNInstantModifier.generated.h"

#pragma region 전방 선언
class UAbilitySystemComponent;
#pragma endregion 전방 선언 끝

/**
 * @file    KNInstantModifier.h
 * @brief   즉시(Instant) 어트리뷰트 수정을 위한 GameplayEffect 및 실행 계산 클래스입니다.
 * @details
 * [사용 예시]
 * - 오버클럭 게이지 즉시 획득 (콤보 적중, 퍼펙트 패링 성공 시)
 * - 스태미나 즉시 소모 (대시, 가드 시전 시)
 * - 피격으로 인한 체력 즉시 감소
 * * [데이터 주도적 설계]
 * - C++ 하드코딩 없이 SetByCaller로 "KatanaNeon.Data.Stats.XXX" 태그와 수치만
 * 전달받아 모든 종류의 스탯 증감을 동적으로 처리합니다.
 */

#pragma region Instant Gameplay Effect 클래스
 /**
  * @class  UKNInstantModifier
  * @brief  Instant Duration 정책(Policy)을 가지는 전용 GE 래퍼 클래스입니다.
  * @details 생성자에서 생명주기(Instant) 세팅과 커스텀 Execution 연결만 수행합니다.
  */
UCLASS()
class KATANANEON_API UKNInstantModifier : public UGameplayEffect
{
    GENERATED_BODY()

public:
    /** @brief GE 정책 초기화 (Instant 지정 및 Execution 클래스 자동 등록) */
    UKNInstantModifier();
};
#pragma endregion Instant Gameplay Effect 클래스

// ────────────────────────────────────────────────────────────

#pragma region Execution Calculation 클래스
/**
 * @class  UKNInstantModifierExecution
 * @brief  UKNInstantModifier의 실제 스탯 증감 연산을 담당하는 실행 계산 클래스입니다.
 * @details 런타임 성능 최적화를 위해 실시간 리플렉션을 배제하고 정적 캐싱 맵을 활용합니다.
 */
UCLASS()
class KATANANEON_API UKNInstantModifierExecution : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

public:
    /** @brief AttributeSet의 모든 어트리뷰트를 캡처 목록에 등록하여 수정을 준비합니다. */
    UKNInstantModifierExecution();

    /**
     * @brief GE 실행 로직 – SetByCaller로 전달받은 태그를 캐시 맵에서 찾아 Additive 연산을 즉시 수행합니다.
     * @param ExecutionParams GE 실행 컨텍스트 및 파라미터 (Spec, ASC 등)
     * @param OutExecutionOutput 최종적으로 적용될 Modifier 결과를 담는 출력 객체
     */
    virtual void Execute_Implementation(
        const FGameplayEffectCustomExecutionParameters& ExecutionParams,
        FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
    /** * @brief 태그-어트리뷰트 매핑 성능을 극대화하기 위한 내부 초기화 헬퍼 함수입니다.
     * @return 최초 1회만 생성된 캐시 맵을 반환합니다.
     */
    static const TMap<FGameplayTag, FGameplayAttribute>& GetCachedAttributeMap();
};
#pragma endregion Execution Calculation 클래스
