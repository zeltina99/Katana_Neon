// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "KNAbilityOverclockLv3.generated.h"

#pragma region 전방 선언
class UAnimMontage;
class UNiagaraSystem;
class AWorldSettings;
class AKNCharacterBase;
class UKNStatsComponent;
class UAbilitySystemComponent;
#pragma endregion 전방 선언

/**
 * @file    KNAbilityOverclockLv3.h
 * @class   UKNAbilityOverclockLv3
 * @brief   오버클럭 3단계 — 플레이어를 제외한 월드 전체의 시간을 정지시키는 궁극기 어빌리티입니다.
 *
 * @details
 * [아키텍처 맞춤형 설계]
 * - FDataTableRowHandle을 사용하여 하드코딩을 배제한 데이터 주도적 설계(DDD)를 적용했습니다.
 * - 범용 ACharacter 캐스팅을 지양하고 프로젝트 고유의 AKNCharacterBase를 사용합니다.
 *
 * [시간 정지 흐름]
 * 1. 오버클럭 포인트 소모 → 몽타주 및 이펙트 재생
 * 2. GlobalTimeDilation = WorldTimeDilationScale (≈0.0001) → 월드 거의 정지
 * 3. 플레이어 CustomTimeDilation = 1.0 / WorldTimeDilationScale → 플레이어만 정상 속도 유지
 * 4. State.Combat.WorldTimeFrozen 태그 부여
 * 5. TimeStopDuration 경과 → 자동 해제 → EndAbility
 *
 * [정지 중 공격 처리 (SRP 준수)]
 * - 데미지 배율 적용은 이 클래스가 아닌, ComboAttack 어빌리티가 GetFrozenDamageMultiplier()를
 * 조회하여 스스로 곱하도록 책임을 철저히 분리했습니다.
 */
UCLASS()
class KATANANEON_API UKNAbilityOverclockLv3 : public UGameplayAbility
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    /**
     * @brief AbilityTag, ActivationRequiredTags, InstancingPolicy를 설정합니다.
     * @details UE 5.5 규약에 맞게 SetAssetTags를 사용합니다.
     */
    UKNAbilityOverclockLv3();
#pragma endregion 기본 생성자 및 초기화

#pragma region GAS 핵심 오버라이드
public:
    /**
     * @brief 시간 정지 궁극기 발동.
     * @details 오버클럭 소모 → 발동 연출 → 시간 배율 설정 → 자동 해제 타이머를 가동합니다.
     */
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    /**
     * @brief 시간 정지 해제 및 정리.
     * @details 모든 시간 배율을 1.0으로 복구하고 태그와 타이머를 제거합니다.
     */
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;
#pragma endregion GAS 핵심 오버라이드

#pragma region 외부 조회 인터페이스
public:
    /**
     * @brief 현재 시간 정지 중일 때의 데미지 배율을 반환합니다.
     * @details ComboAttack 어빌리티에서 WorldTimeFrozen 태그 존재 시 이 값을 조회합니다.
     * @return FrozenDamageMultiplier (시간 정지 비활성 시 1.0 반환)
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Ability|OverclockLv3")
    float GetFrozenDamageMultiplier() const;
#pragma endregion 외부 조회 인터페이스

#pragma region 에디터 설정 데이터 (DDD)
protected:
    /**
     * @brief 오버클럭 Lv3 설정 DataTable 행 핸들.
     * @details 기획자가 에디터에서 정지 시간, 배율, 소모량을 직접 주입합니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv3|DataTable")
    FDataTableRowHandle Lv3SettingRowHandle;

    /** @brief 궁극기 발동 시 재생할 애니메이션 몽타주 (에디터 할당, nullptr 가능) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv3|Montage")
    TObjectPtr<UAnimMontage> TimeFreezeUltimateMontage = nullptr;

    /** @brief 시간 정지 발동 순간 월드에 뿌려질 Niagara 이펙트 (에디터 할당, nullptr 가능) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv3|FX")
    TObjectPtr<UNiagaraSystem> TimeStopNiagara = nullptr;
#pragma endregion 에디터 설정 데이터 (DDD)

#pragma region 런타임 상태
private:
    /** @brief 시간 정지 자동 해제 타이머 핸들 */
    FTimerHandle TimeStopTimerHandle;

    /** @brief 현재 시간 정지 중인지 여부 */
    bool bIsTimeStopped = false;

    /** @brief DataTable에서 읽어와 런타임에 캐싱한 Lv3 설정 데이터 */
    FKNOverclockLv3Row CachedSetting;
#pragma endregion 런타임 상태

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief 에디터에 할당된 데이터 행을 읽어와 CachedSetting에 저장합니다.
     * @return 로드 성공 여부
     */
    bool LoadLv3Setting();

    /**
     * @brief KNStatsComponent를 통해 오버클럭 Lv3 포인트를 차감합니다.
     * @return 소모 성공 여부
     */
    bool ConsumeOverclockLevel();

    /**
     * @brief GlobalTimeDilation과 플레이어 CustomTimeDilation을 동시에 설정합니다.
     * @param WorldScale GlobalTimeDilation에 적용할 배율
     */
    void ApplyTimeDilation(float WorldScale);

    /**
     * @brief 시간 정지를 활성화합니다 (배율 적용 → 태그 부여 → 타이머 설정).
     */
    void ActivateTimeStop();

    /**
     * @brief 시간 정지 자동 해제 타이머 콜백 — EndAbility를 안전하게 호출합니다.
     */
    UFUNCTION()
    void OnTimeStopExpired();
#pragma endregion 내부 헬퍼 함수
};
