// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "KNAbilityChronos.generated.h"

#pragma region 전방 선언
class UKNChronosSphereComponent;
class AKNCharacterBase;
class UAbilitySystemComponent;
class UGameplayEffect;
#pragma endregion 전방 선언

/**
 * @file    KNAbilityChronos.h
 * @class   UKNAbilityChronos
 * @brief   플레이어 주변 구체 범위 내의 적과 적 발사체의 시간을 감속시키는 토글 어빌리티입니다.
 *
 * @details
 * [아키텍처 맞춤형 설계]
 * - FDataTableRowHandle을 사용하여 하드코딩을 완전히 배제한 데이터 주도적 설계(DDD)를 적용했습니다.
 * - 엔진 기본 ACharacter가 아닌, 프로젝트 고유의 AKNCharacterBase를 기준으로 작동하도록 최적화되었습니다.
 *
 * [작동 방식]
 * - 최초 활성화 : State.Combat.ChronosActive 태그 부여 → 구체 콜리전 활성화 → 게이지 소모 시작
 * - 재활성화(토글 OFF) : 게이지 소모 중지 → 구체 비활성화 → 태그 제거 → EndAbility
 *
 * [SRP 책임 분리]
 * - 구체 범위 감속  : UKNChronosSphereComponent에 완전 위임
 * - 게이지 소모     : Instant GE 주기 적용
 * - 어빌리티 생애   : ActivateAbility / EndAbility
 */
UCLASS()
class KATANANEON_API UKNAbilityChronos : public UGameplayAbility
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    /**
     * @brief AbilityTag, InstancingPolicy를 설정합니다.
     * @details UE 5.5 규약에 맞게 SetAssetTags를 사용합니다.
     */
    UKNAbilityChronos();
#pragma endregion 기본 생성자 및 초기화

#pragma region GAS 핵심 오버라이드
public:
    /**
     * @brief 활성화 조건 검사.
     * @details 크로노스 게이지가 0보다 클 때, 또는 이미 활성화(토글 OFF 진행) 중일 때 허용합니다.
     * @param Handle                    어빌리티 스펙 핸들
     * @param ActorInfo                 소유 액터 정보
     * @param SourceTags                소스 태그 컨테이너
     * @param TargetTags                타겟 태그 컨테이너
     * @param OptionalRelevantTags      실패 원인 출력 태그
     * @return 활성화 가능하면 true
     */
    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr,
        const FGameplayTagContainer* TargetTags = nullptr,
        OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

    /**
     * @brief 크로노스 ON/OFF 토글 처리.
     * @details bIsChronosActive 플래그로 현재 상태를 구분합니다.
     * @param Handle            어빌리티 스펙 핸들
     * @param ActorInfo         소유 액터 정보
     * @param ActivationInfo    활성화 방식 정보
     * @param TriggerEventData  트리거 이벤트 데이터
     */
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    /**
     * @brief 크로노스 어빌리티 종료.
     * @details 구체 비활성화, 소모 타이머 해제, 태그 제거를 일괄 처리합니다.
     * @param Handle                    어빌리티 스펙 핸들
     * @param ActorInfo                 소유 액터 정보
     * @param ActivationInfo            활성화 방식 정보
     * @param bReplicateEndAbility      복제 여부
     * @param bWasCancelled             취소 여부
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
     * @brief 크로노스 설정 DataTable 행 핸들.
     * @details 기획자가 에디터에서 구체 반경, 감속 배율, 소모량 데이터를 직접 주입합니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Chronos|DataTable")
    FDataTableRowHandle ChronosSettingRowHandle;

    /**
     * @brief 크로노스 게이지 주기 소모용 Instant GE 클래스 (에디터 할당).
     * @details SetByCaller(Chronos, -소모량)으로 주기마다 적용됩니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Chronos|GAS")
    TSubclassOf<UGameplayEffect> ChronosDrainGEClass = nullptr;
#pragma endregion 에디터 설정 데이터 (DDD)

#pragma region 런타임 상태
private:
    /** @brief 현재 크로노스가 활성화 상태인지 여부 (토글 판별) */
    bool bIsChronosActive = false;

    /** @brief 게이지 소모 주기 타이머 핸들 */
    FTimerHandle DrainTimerHandle;

    /** @brief DataTable에서 로드한 설정 캐시 */
    FKNChronosSettingRow CachedSetting;

    /**
     * @brief 플레이어 캐릭터에 부착된 크로노스 구체 컴포넌트 약참조.
     * @details 메모리 누수를 방지하기 위해 TWeakObjectPtr를 사용합니다.
     */
    TWeakObjectPtr<UKNChronosSphereComponent> WeakSphereComp = nullptr;
#pragma endregion 런타임 상태

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief 에디터에 할당된 데이터 행을 읽어와 CachedSetting에 저장합니다.
     * @return 로드 성공 여부
     */
    bool LoadChronosSetting();

    /**
     * @brief 소유 캐릭터에서 KNChronosSphereComponent를 찾아 WeakSphereComp를 초기화합니다.
     * @return 컴포넌트 탐색 성공 여부
     */
    bool ResolveSphereComponent();

    /**
     * @brief 크로노스 게이지 소모 주기 타이머 콜백.
     * @details DrainTickInterval마다 호출되어 게이지를 소모하고, 0 도달 시 어빌리티를 자동 종료합니다.
     */
    UFUNCTION()
    void OnDrainTick();
#pragma endregion 내부 헬퍼 함수
};
