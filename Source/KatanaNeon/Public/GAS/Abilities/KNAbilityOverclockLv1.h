// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "KNAbilityOverclockLv1.generated.h"

#pragma region 전방 선언
class UAnimMontage;
class UKNStatsComponent;
class AKNCharacterBase;
class UAbilitySystemComponent;
class UGameplayEffect;
#pragma endregion 전방 선언

/**
 * @file    KNAbilityOverclockLv1.h
 * @class   UKNAbilityOverclockLv1
 * @brief   오버클럭 1단계 — 일정 시간 동안 공격속도/이동속도 증가 및 스태미나 소모 면역을 부여합니다.
 *
 * @details
 * [아키텍처 맞춤형 설계]
 * - FDataTableRowHandle을 사용하여 하드코딩을 배제한 데이터 주도적 설계(DDD)를 적용했습니다.
 * - 범용 ACharacter 캐스팅을 지양하고 프로젝트 고유의 AKNCharacterBase를 사용합니다.
 *
 * [활성화 조건]
 * - ActivationRequiredTags : State.Overclock.Lv1 태그 보유 (게이지 충족 시 StatsComponent가 부여)
 *
 * [효과 — 에디터 할당 데이터 기준]
 * - Duration GE를 통한 AttackSpeed / MovementSpeed 스탯 펌핑
 * - State.Combat.OverclockTactical 및 State.Combat.StaminaImmune 태그 임시 부여
 *
 * [SRP 책임 분리]
 * - 버프 적용     : Duration GE 파이프라인 활용
 * - 오버클럭 소모 : KNStatsComponent::ConsumeOverclockLevel(1)로 역할 위임
 */
UCLASS()
class KATANANEON_API UKNAbilityOverclockLv1 : public UGameplayAbility
{
    GENERATED_BODY()

#pragma region 기본 생성자 및 초기화
public:
    /**
     * @brief AbilityTag, ActivationRequiredTags, InstancingPolicy를 설정합니다.
     * @details UE 5.5 규약에 맞게 SetAssetTags를 사용합니다.
     */
    UKNAbilityOverclockLv1();
#pragma endregion 기본 생성자 및 초기화

#pragma region GAS 핵심 오버라이드
public:
    /**
     * @brief 전술 강화 어빌리티 활성화.
     * @details 오버클럭 소모 → 상태 태그 부여 → Duration GE 적용 → 타이머 설정 순으로 진행합니다.
     */
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    /**
     * @brief 어빌리티 종료.
     * @details 상태 태그 제거 및 타이머 해제를 수행합니다. (Duration GE는 엔진이 자동 만료 처리)
     */
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;
#pragma endregion GAS 핵심 오버라이드

#pragma region 에디터 설정 데이터 (DDD)
protected:
    /**
     * @brief 오버클럭 Lv1 설정 DataTable 행 핸들.
     * @details 기획자가 지속 시간, 속도 증가량, 면역 여부 등을 에디터에서 직접 세팅합니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv1|DataTable")
    FDataTableRowHandle Lv1SettingRowHandle;

    /**
     * @brief 공격 속도 증가 Duration GE 클래스.
     * @details SetByCaller(AttackSpeed, +Delta) 방식으로 적용됩니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv1|GAS")
    TSubclassOf<UGameplayEffect> AttackSpeedBuffGEClass = nullptr;

    /**
     * @brief 이동 속도 증가 Duration GE 클래스.
     * @details SetByCaller(MovementSpeed, +Delta) 방식으로 적용됩니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv1|GAS")
    TSubclassOf<UGameplayEffect> MovementSpeedBuffGEClass = nullptr;

    /** @brief 전술 강화 발동 시 재생할 몽타주 (에디터 할당, nullptr 가능) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv1|Montage")
    TObjectPtr<UAnimMontage> TacticalMontage = nullptr;
#pragma endregion 에디터 설정 데이터 (DDD)

#pragma region 런타임 상태
private:
    /** @brief 어빌리티 자동 종료 타이머 핸들 */
    FTimerHandle DurationTimerHandle;

    /** @brief DataTable에서 읽어와 런타임에 캐싱한 Lv1 설정 데이터 */
    FKNOverclockLv1Row CachedSetting;
#pragma endregion 런타임 상태

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief 에디터에 할당된 데이터 행을 읽어와 CachedSetting에 저장합니다.
     * @return 로드 성공 여부
     */
    bool LoadLv1Setting();

    /**
     * @brief KNStatsComponent를 통해 오버클럭 Lv1 포인트를 차감합니다.
     * @return 소모 성공 여부
     */
    bool ConsumeOverclockLevel();

    /**
     * @brief 공격 속도 / 이동 속도 증가 Duration GE를 적용합니다.
     * @param ASC 소유자의 AbilitySystemComponent
     */
    void ApplyStatBuffGEs(UAbilitySystemComponent* ASC);

    /**
     * @brief 전투용 상태 태그(OverclockTactical, StaminaImmune)를 Loose로 부여합니다.
     * @param ASC 소유자의 AbilitySystemComponent
     */
    void GrantStateTags(UAbilitySystemComponent* ASC);

    /**
     * @brief 어빌리티 지속 시간 만료 콜백 — EndAbility를 안전하게 호출합니다.
     */
    UFUNCTION()
    void OnDurationExpired();
#pragma endregion 내부 헬퍼 함수
};
