// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "KNCharacterBase.generated.h"

#pragma region 전방 선언
class UAbilitySystemComponent;
class UKNAttributeSet;
class UKNStatsComponent;
#pragma endregion 전방 선언 끝

/**
 * @class  AKNCharacterBase
 * @brief  게임 내 모든 캐릭터(플레이어, 적, 보스)의 최상위 부모 클래스입니다.
 * @details IAbilitySystemInterface를 상속받아 GAS 시스템의 뼈대를 제공하며,
 * 체력, 스태미나 등의 기본 스탯과 피격/사망 공통 로직을 처리합니다.
 */
UCLASS()
class KATANANEON_API AKNCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

#pragma region 기본 생성자 및 초기화
public:
    AKNCharacterBase();

protected:
    virtual void BeginPlay() override;
#pragma endregion 기본 생성자 및 초기화 끝

#pragma region GAS 인터페이스 구현
public:
    /**
     * @brief IAbilitySystemInterface의 필수 구현 함수입니다.
     * @return 이 캐릭터가 소유한 AbilitySystemComponent를 반환합니다.
     */
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
#pragma endregion GAS 인터페이스 구현

#pragma region GAS 핵심 컴포넌트
protected:
    /** @brief 캐릭터의 모든 스킬과 이펙트를 관장하는 핵심 컴포넌트입니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|GAS", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

    /** @brief 캐릭터의 체력, 스태미나, 크로노스 등의 수치를 들고 있는 데이터 셋입니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|GAS", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UKNAttributeSet> AttributeSet = nullptr;

    /** @brief 데이터 테이블(CSV)을 읽어와 스탯을 초기화하고 UI 이벤트를 쏴주는 헬퍼 컴포넌트입니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|GAS", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UKNStatsComponent> StatsComponent = nullptr;
#pragma endregion GAS 핵심 컴포넌트
};
