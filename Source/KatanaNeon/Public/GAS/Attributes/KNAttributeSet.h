// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
// 매크로 의존성 명시: ATTRIBUTE_ACCESSORS 매크로는 아래 헤더에서 제공됩니다.
// 위치: [프로젝트 루트]/Source/KatanaNeon/GAS/System/KNGASMacros.h
#include "GAS/System/KNGASMacros.h"
#include "KNAttributeSet.generated.h"

#pragma region 전방 선언
struct FGameplayEffectModCallbackData;
#pragma endregion 전방 선언

/**
 * @class UKNAttributeSet
 * @brief KatanaNeon 프로젝트 캐릭터의 핵심 전투 및 생존 스탯을 관리하는 어트리뷰트 셋입니다.
 * @details 데이터 주도적 설계(Data-Driven Design)를 원칙으로 하여, 모든 수치의 최대치 및 기본값은
 * 에디터의 DataTable과 GameplayEffect를 통해 초기화됩니다.
 */
UCLASS()
class KATANANEON_API UKNAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    UKNAttributeSet();
#pragma endregion 기본 생성자 및 초기화

#pragma region GAS 핵심 오버라이드 함수
public:
    /**
     * @brief 어트리뷰트 값이 실질적으로 변경되기 직전에 호출되어, 최대치 제한(Clamp) 등의 전처리를 수행합니다.
     * @param Attribute 변경될 어트리뷰트 정보
     * @param NewValue 새로 적용될 값
     */
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

    /**
     * @brief GameplayEffect가 적용되어 어트리뷰트 값이 최종 변경된 후 호출되는 후처리 콜백입니다.
     * @param Data 적용된 GameplayEffect에 대한 상세 데이터
     */
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
#pragma endregion GAS 핵심 오버라이드 함수

#pragma region 생존 및 기동 어트리뷰트
public:
    /** @brief 현재 체력. 0이 되면 사망 처리 로직과 연결됩니다. */
    UPROPERTY(BlueprintReadOnly, Category = "KatanaNeon|Attributes|Survival")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UKNAttributeSet, Health)

    /** @brief 최대 체력. 데이터 테이블에서 값을 받아옵니다. */
    UPROPERTY(BlueprintReadOnly, Category = "KatanaNeon|Attributes|Survival")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UKNAttributeSet, MaxHealth)

    /** @brief 캐릭터의 기본 걷기/달리기 이동 속도 */
    UPROPERTY(BlueprintReadOnly, Category = "KatanaNeon|Attributes|Movement")
    FGameplayAttributeData MovementSpeed;
    ATTRIBUTE_ACCESSORS(UKNAttributeSet, MovementSpeed)
#pragma endregion 생존 및 기동 어트리뷰트

#pragma region 액션 자원 스태미나 어트리뷰트
public:
    /** @brief 현재 스태미나. 대시 및 패링 시 실시간으로 소모됩니다. */
    UPROPERTY(BlueprintReadOnly, Category = "KatanaNeon|Attributes|Action")
    FGameplayAttributeData Stamina;
    ATTRIBUTE_ACCESSORS(UKNAttributeSet, Stamina)

    /** @brief 스태미나의 최대치 한도 */
    UPROPERTY(BlueprintReadOnly, Category = "KatanaNeon|Attributes|Action")
    FGameplayAttributeData MaxStamina;
    ATTRIBUTE_ACCESSORS(UKNAttributeSet, MaxStamina)

    /** @brief 스태미나가 1초당 자연적으로 회복되는 양 */
    UPROPERTY(BlueprintReadOnly, Category = "KatanaNeon|Attributes|Action")
    FGameplayAttributeData StaminaRegenRate;
    ATTRIBUTE_ACCESSORS(UKNAttributeSet, StaminaRegenRate)
#pragma endregion 액션 자원 스태미나 어트리뷰트

#pragma region 기믹 자원 크로노스 어트리뷰트
public:
    /** @brief 현재 크로노스 게이지. 시간 감속 스킬 활성화 시 지속 소모됩니다. */
    UPROPERTY(BlueprintReadOnly, Category = "KatanaNeon|Attributes|Gimmick")
    FGameplayAttributeData Chronos;
    ATTRIBUTE_ACCESSORS(UKNAttributeSet, Chronos)

    /** @brief 크로노스 게이지의 최대치 한도 */
    UPROPERTY(BlueprintReadOnly, Category = "KatanaNeon|Attributes|Gimmick")
    FGameplayAttributeData MaxChronos;
    ATTRIBUTE_ACCESSORS(UKNAttributeSet, MaxChronos)
#pragma endregion 기믹 자원 크로노스 어트리뷰트

#pragma region 보상 자원 오버클럭 어트리뷰트
public:
    /**
     * @brief 콤보나 퍼펙트 패링 성공 시 누적되는 현재 오버클럭 포인트
     * @details 이 값이 특정 구간에 도달하면 외부 시스템에서 판단하여 레벨 태그(Lv1, Lv2, Lv3)를 부착합니다.
     */
    UPROPERTY(BlueprintReadOnly, Category = "KatanaNeon|Attributes|Reward")
    FGameplayAttributeData OverclockPoint;
    ATTRIBUTE_ACCESSORS(UKNAttributeSet, OverclockPoint)

    /** @brief 오버클럭 포인트의 최대치 한도 (데이터 테이블에서 초기화됨) */
    UPROPERTY(BlueprintReadOnly, Category = "KatanaNeon|Attributes|Reward")
    FGameplayAttributeData MaxOverclockPoint;
    ATTRIBUTE_ACCESSORS(UKNAttributeSet, MaxOverclockPoint)
#pragma endregion 보상 자원 오버클럭 어트리뷰트
};
