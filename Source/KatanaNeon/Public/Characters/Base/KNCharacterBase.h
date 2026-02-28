// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "KNCharacterBase.generated.h"

#pragma region 전방 선언
class UAbilitySystemComponent;
class UKNAttributeSet;
class UGameplayAbility;
#pragma endregion 전방 선언 끝

#pragma region 델리게이트 선언
/** @brief 캐릭터 사망 시 외부(UI, AI, 게임모드)에 알리는 델리게이트입니다. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKNCharacterDeath, AKNCharacterBase*, DeadCharacter);
#pragma endregion 델리게이트 선언

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

#pragma region 사망 이벤트
public:
    /** @brief 캐릭터 사망 시 외부 시스템이 구독할 델리게이트입니다. */
    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Character|Event")
    FOnKNCharacterDeath OnCharacterDeath;

protected:
    /**
     * @brief 사망 처리 진입점입니다. 파생 클래스에서 오버라이드하여 AI 정지, 래그돌 등을 처리합니다.
     * @details AttributeSet의 PostGameplayEffectExecute에서 체력 0 감지 시 호출되어야 합니다.
     */
    virtual void Die();
#pragma endregion 사망 이벤트

#pragma region 어빌리티 부여 인터페이스
protected:
    /**
     * @brief DefaultAbilities 배열에 등록된 어빌리티를 ASC에 일괄 부여합니다.
     * @details 파생 클래스의 BeginPlay에서 InitAbilityActorInfo 호출 이후에 실행됩니다.
     */
    void GiveDefaultAbilities();

    /** @brief 게임 시작 시 자동으로 부여할 기본 GameplayAbility 클래스 목록 (에디터 할당용) */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|GAS|Abilities")
    TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
#pragma endregion 어빌리티 부여 인터페이스

#pragma region GAS 핵심 컴포넌트
protected:
    /** @brief 캐릭터의 모든 스킬과 이펙트를 관장하는 핵심 컴포넌트입니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|GAS", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

    /** @brief 캐릭터의 체력, 스태미나, 크로노스 등의 수치를 들고 있는 데이터 셋입니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|GAS", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UKNAttributeSet> AttributeSet = nullptr;
#pragma endregion GAS 핵심 컴포넌트
};
