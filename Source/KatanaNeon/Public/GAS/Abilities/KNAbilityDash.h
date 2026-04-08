// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataTable.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "KNAbilityDash.generated.h"

#pragma region 전방 선언
class UAnimMontage;
class AKNCharacterBase;
class UAbilitySystemComponent;
class UAbilityTask_PlayMontageAndWait;
class UGameplayEffect;
#pragma endregion 전방 선언

/**
 * @file    KNAbilityDash.h
 * @class   UKNAbilityDash
 * @brief   플레이어의 순간 대시 및 무적 프레임(I-Frame) 어빌리티입니다.
 *
 * @details
 * [아키텍처 맞춤형 설계]
 * - 엔진 기본 ACharacter가 아닌, 프로젝트 고유의 AKNCharacterBase를 기준으로 작동하도록 최적화되었습니다.
 * * [데이터 주도적 설계 (DDD)]
 * - 에디터에서 FDataTableRowHandle을 통해 기획자가 대시 스태미나 소모량을 직접 주입합니다. 하드코딩 배제.
 *
 * [동작 순서]
 * 1. CanActivateAbility : 에디터에 할당된 데이터 행의 스태미나 요구치 충족 여부 확인
 * 2. ActivateAbility    : 스태미나 소모 → State.Combat.Invincible 태그 부여 → 캐릭터 발사(Launch)
 * 3. InvincibleDuration 경과 후 : 무적 태그 제거 → EndAbility
 */
UCLASS()
class KATANANEON_API UKNAbilityDash : public UGameplayAbility
{
    GENERATED_BODY()

#pragma region 기본 생성자 및 초기화
public:
    /**
     * @brief 어빌리티 기본값 초기화.
     * @details AbilityTag, ActivationBlockedTags(Invincible 중복 방지), InstancingPolicy를 설정합니다.
     */
    UKNAbilityDash();
#pragma endregion 기본 생성자 및 초기화

#pragma region GAS 핵심 오버라이드
public:
    /**
     * @brief 활성화 조건 검사.
     * @details 지정된 액션 비용 이상의 스태미나가 있을 때만 허용합니다.
     * @param Handle               어빌리티 스펙 핸들
     * @param ActorInfo            소유 액터 정보
     * @param SourceTags           소스 태그 컨테이너 (옵션)
     * @param TargetTags           타겟 태그 컨테이너 (옵션)
     * @param OptionalRelevantTags 실패 원인 출력 태그 (옵션)
     * @return 활성화 가능하면 true 반환
     */
    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr,
        const FGameplayTagContainer* TargetTags = nullptr,
        OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

    /**
     * @brief 대시 실행.
     * @details 스태미나 소모 → 무적 태그 부여 → 캐릭터 발사 → 타이머 설정 순으로 진행합니다.
     * @param Handle           어빌리티 스펙 핸들
     * @param ActorInfo        소유 액터 정보
     * @param ActivationInfo   활성화 방식 정보
     * @param TriggerEventData 트리거 이벤트 데이터 (nullptr 가능)
     */
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    /**
     * @brief 대시 종료.
     * @details 무적 태그 안전 제거 및 타이머 해제를 수행합니다.
     * @param Handle                 어빌리티 스펙 핸들
     * @param ActorInfo              소유 액터 정보
     * @param ActivationInfo         활성화 방식 정보
     * @param bReplicateEndAbility   복제 여부
     * @param bWasCancelled          취소 여부
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
     * @brief 액션 비용 DataTable 행 핸들.
     * @details 하드코딩을 배제하고 에디터에서 기획자가 직접 대시 비용 데이터를 지정합니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Dash|DataTable")
    FDataTableRowHandle ActionCostRowHandle;


    /**
     * @brief 방향 및 스탠스별 대시 몽타주 DataTable.
     * @details Row Structure: FKNDashMontageRow
     *          Row Key: Forward, ForwardLeft, ForwardRight, Left, Right, BackLeft, BackRight, Back
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Dash|DataTable")
    TObjectPtr<UDataTable> DashMontageTable = nullptr;

    /**
     * @brief 대시 발사 속도 (cm/s).
     * @details 캐릭터의 질량과 기본 이동 속도에 비례하여 조절합니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Dash|Config", meta = (ClampMin = 100.0f, ClampMax = 5000.0f))
    float DashImpulse = 1500.0f;

    /**
     * @brief 무적 프레임 지속 시간 (초).
     * @details 이 시간 동안 State.Combat.Invincible 태그가 유지됩니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Dash|Config", meta = (ClampMin = 0.05f, ClampMax = 1.0f))
    float InvincibleDuration = 0.2f;

    /** @brief 스태미나 즉시 소모용 Instant GE 클래스 (에디터 할당) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Dash|GAS")
    TSubclassOf<UGameplayEffect> StaminaCostGEClass = nullptr;
#pragma endregion 에디터 설정 데이터 (DDD)

#pragma region 런타임 상태
private:
    /** @brief 무적 해제 타이머 핸들 */
    FTimerHandle InvincibleTimerHandle;

    /** @brief 초기화 시 DataTable에서 읽어 캐싱한 액션 비용 */
    FKNActionCostRow CachedActionCost;

    /** @brief 대시 몽타주 재생 중 여부 — 몽타주 종료 전까지 EndAbility를 지연시킵니다. */
    bool bIsDashMontageActive = false;
#pragma endregion 런타임 상태

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief ActionCostRowHandle을 통해 기획 데이터를 읽어와 CachedActionCost에 저장합니다.
     * @return 로드 성공 여부
     */
    bool LoadActionCostRow();

    /**
     * @brief CachedActionCost의 DashStaminaCost 만큼 스태미나를 GE로 즉시 소모합니다.
     * @return 소모 성공 여부
     */
    bool ConsumeStamina();

    /**
     * @brief State.Combat.Invincible 태그를 ASC에 Loose 방식으로 부여합니다.
     * @param ASC 소유자의 AbilitySystemComponent
     */
    void GrantInvincible(UAbilitySystemComponent* ASC);

    /**
     * @brief 입력 방향 또는 전방으로 캐릭터를 LaunchCharacter합니다.
     * @param Character 발사할 AKNCharacterBase 객체
     */
    void LaunchDash(AKNCharacterBase* InCharacter);

    /**
    * @brief 입력 방향과 캐릭터 전방 각도를 계산하여 DT_DashMontage 행 이름을 반환합니다.
    * @param Character 방향을 계산할 대상 캐릭터
    * @return Forward, ForwardLeft, ForwardRight, Left, Right, BackLeft, BackRight, Back 중 하나
    */
    FName GetDodgeDirectionRowName(const AKNCharacterBase* InCharacter) const;

    /**
     * @brief 현재 캐릭터 상태(공중/달리기/지상 × 발도/납도)에 맞는 몽타주를 선택합니다.
     * @details 선택 우선순위: 공중(IsFalling) > 달리기(Sprint 태그) > 지상
     * @param InRow DataTable에서 조회한 방향별 몽타주 행
     * @return 선택된 UAnimMontage, 해당 슬롯이 비어있으면 nullptr
     */
    UAnimMontage* SelectDashMontage(const FKNDashMontageRow* InRow) const;

    /**
     * @brief 무적 만료 타이머 콜백 — State.Combat.Invincible 태그 제거 후 EndAbility.
     */
    UFUNCTION()
    void OnInvincibleExpired();

    /**
    * @brief 대시 몽타주가 완전히 종료된 후 호출됩니다.
    * @details OnCompleted/OnBlendOut/OnInterrupted 모두 이 함수로 연결하여
    *          몽타주가 끝나기 전까지 어빌리티가 종료되지 않도록 보장합니다.
    */
    UFUNCTION()
    void OnDashMontageFinished();
#pragma endregion 내부 헬퍼 함수
};
