// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "KNAbilityParry.generated.h"

#pragma region 전방 선언
class UAnimMontage;
class AKNCharacterBase;
class UAbilitySystemComponent;
class UGameplayEffect;
#pragma endregion 전방 선언

/**
 * @file    KNAbilityParry.h
 * @class   UKNAbilityParry
 * @brief   플레이어의 패링 및 퍼펙트 패링(FlurryRush) 어빌리티입니다.
 *
 * @details
 * [아키텍처 맞춤형 설계]
 * - 엔진 기본 ACharacter가 아닌, 프로젝트 고유의 AKNCharacterBase를 기준으로 최적화되었습니다.
 * - FDataTableRowHandle을 사용하여 하드코딩을 완전히 배제한 데이터 주도적 설계(DDD)를 적용했습니다.
 *
 * [패링 판정 흐름]
 * 1. Input_Parry 입력 시 어빌리티 활성화 (에디터 지정 스태미나 소모)
 * 2. PerfectParryWindowTime 동안 State.Combat.Parrying 태그 유지
 * 3. 이 시간 내에 적 AttackWarning 이벤트가 도달하면 → 퍼펙트 패링 판정 (FlurryRush 돌입)
 */
UCLASS()
class KATANANEON_API UKNAbilityParry : public UGameplayAbility
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    /** @brief AbilityTag, InstancingPolicy, 패링 판정 타이머 기본값을 설정합니다. */
    UKNAbilityParry();
#pragma endregion 기본 생성자 및 초기화

#pragma region GAS 핵심 오버라이드
public:
    /**
     * @brief 활성화 조건 검사.
     * @details DT_ActionCost.ParryStaminaCost (= 15) 이상 스태미나가 있어야 합니다.
     */
    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr,
        const FGameplayTagContainer* TargetTags = nullptr,
        OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

    /**
     * @brief 패링 실행.
     * @details 스태미나 소모 → Parrying 태그 부여 → 퍼펙트 판정 창 오픈.
     */
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    /**
     * @brief 패링 종료.
     * @details 모든 태그 제거, 타이머 해제, FlurryRush 중이면 SlowMotion도 복구합니다.
     */
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;
#pragma endregion GAS 핵심 오버라이드

#pragma region 외부 호출 인터페이스 (적 공격 예고 수신)
public:
    /**
     * @brief 적 공격 예고(OnAttackWarning) 이벤트를 수신했을 때 플레이어 컨트롤러가 호출합니다.
     * @details Parrying 태그가 활성화된 상태에서만 퍼펙트 패링이 성립합니다.
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Ability|Parry")
    void OnEnemyAttackWarningReceived();
#pragma endregion 외부 호출 인터페이스

#pragma region 에디터 설정 데이터 (DDD)
protected:
    /** @brief 액션 비용 DataTable 행 핸들 (ParryStaminaCost 참조) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Parry|DataTable")
    FDataTableRowHandle ActionCostRowHandle;

    /** @brief 오버클럭 설정 DataTable 행 핸들 (GainPerfectParry 참조) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Parry|DataTable")
    FDataTableRowHandle OverclockSettingRowHandle;

    /** @brief 패링 애니메이션 몽타주 (에디터 할당) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Parry|Montage")
    TObjectPtr<UAnimMontage> ParryMontage = nullptr;

    /** @brief 퍼펙트 패링 성공 시 재생할 몽타주 (에디터 할당) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Parry|Montage")
    TObjectPtr<UAnimMontage> PerfectParryMontage = nullptr;

    /** @brief 스태미나 즉시 소모 Instant GE 클래스 (에디터 할당) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Parry|GAS")
    TSubclassOf<UGameplayEffect> StaminaCostGEClass = nullptr;

    /** @brief 퍼펙트 패링 판정 허용 시간 창 (초) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Parry|Config", meta = (ClampMin = 0.1f, ClampMax = 1.0f))
    float PerfectParryWindowTime = 0.3f;

    /** @brief FlurryRush 슬로우 모션 배율 (0.0 ~ 1.0) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Parry|Config", meta = (ClampMin = 0.05f, ClampMax = 1.0f))
    float FlurrySlowMotionScale = 0.2f;

    /** @brief FlurryRush 슬로우 모션 지속 시간 (초) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Parry|Config", meta = (ClampMin = 0.5f, ClampMax = 5.0f))
    float FlurryRushDuration = 2.5f;
#pragma endregion 에디터 설정 데이터 (DDD)

#pragma region 런타임 상태
private:
    /** @brief 패링 판정 창 만료 타이머 핸들 */
    FTimerHandle ParryWindowTimerHandle;

    /** @brief FlurryRush 해제 타이머 핸들 */
    FTimerHandle FlurryRushTimerHandle;

    /** @brief 현재 FlurryRush 상태인지 여부 */
    bool bIsFlurryRush = false;
#pragma endregion 런타임 상태

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief ParryStaminaCost 만큼 스태미나를 즉시 소모합니다.
     * @return 소모 성공 여부
     */
    bool ConsumeStamina();

    /**
     * @brief 퍼펙트 패링 성공 처리.
     * @details FlurryRush 태그 부여, 슬로우 모션 적용, 오버클럭 포인트 획득.
     */
    void OnPerfectParry();

    /**
     * @brief FlurryRush 슬로우 모션을 활성화합니다.
     * @details GlobalTimeDilation = FlurrySlowMotionScale,
     *          Player.CustomTimeDilation = 1.0 / FlurrySlowMotionScale
     */
    void ActivateFlurryRushSlowMotion();

    /**
     * @brief FlurryRush 슬로우 모션을 해제하고 어빌리티를 종료합니다.
     */
    UFUNCTION()
    void DeactivateFlurryRush();

    /**
     * @brief 패링 판정 창 만료 콜백 — 퍼펙트 패링 없이 일반 패링 종료.
     */
    UFUNCTION()
    void OnParryWindowExpired();
#pragma endregion 내부 헬퍼 함수
};
