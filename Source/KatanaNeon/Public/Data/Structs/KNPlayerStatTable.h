// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "KNPlayerStatTable.generated.h"

/**
 * @file    KNPlayerStatTable.h
 * @brief   KatanaNeon에서 기획자가 CSV/DataTable로 조절하는 모든 '수치 데이터' 구조체 모음입니다.
 * @details 단일 책임 원칙(SRP)에 따라 하드코딩을 배제하고 오직 데이터 저장 및 제공 역할만 수행합니다.
 */

#pragma region 기본 스탯 초기값 테이블
 /**
  * @struct FKNBaseStatRow
  * @brief 플레이어 캐릭터의 기본 스탯 초기값 행 구조체입니다.
  * @details DataTable의 각 Row가 하나의 캐릭터(또는 난이도) 프리셋에 해당합니다.
  * 건강/스태미나/크로노스의 최대치 및 초기값을 설정합니다.
  */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNBaseStatRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    /** @brief 초기 최대 체력 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Survival")
    float MaxHealth = 100.0f;

    /** @brief 초기 최대 스태미나 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Stamina")
    float MaxStamina = 100.0f;

    /** @brief 스태미나 초당 자연 회복량 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Stamina")
    float StaminaRegenRate = 10.0f;

    /** @brief 기본 이동 속도 (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Movement")
    float MovementSpeed = 600.0f;

    /** @brief 초기 최대 크로노스 게이지 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Chronos")
    float MaxChronos = 100.0f;

    /** @brief 시간 감속 활성 중 초당 크로노스 소모량 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Chronos")
    float ChronosDrainRate = 20.0f;
};
#pragma endregion 기본 스탯 초기값 테이블

#pragma region 액션 비용 테이블 (스태미나 소모)
/**
 * @struct FKNActionCostRow
 * @brief 플레이어 액션별 스태미나 소모량을 정의하는 행 구조체입니다.
 * @details 각 액션마다 소모량을 분리해 기획자가 세밀하게 밸런싱할 수 있도록 캡슐화했습니다.
 */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNActionCostRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    /** @brief 대시 1회당 스태미나 소모량 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|ActionCost")
    float DashStaminaCost = 20.0f;

    /** @brief 일반 패링(가드) 시전 시 스태미나 소모량 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|ActionCost")
    float ParryStaminaCost = 15.0f;
};
#pragma endregion 액션 비용 테이블

#pragma region 점프 설정 테이블
/**
 * @struct FKNJumpSettingRow
 * @brief  1단 점프 및 더블 점프의 수치를 정의하는 행 구조체입니다.
 *
 * @details
 * [더블 점프 동작 원리]
 * UCharacterMovementComponent::JumpMaxCount를 2로 설정하면 엔진이
 * 동일한 IA_Jump 입력으로 2단 점프를 자동 처리합니다.
 * 별도의 Input Action은 필요하지 않습니다.
 *
 * [적용 위치]
 * AKNPlayerCharacter::BeginPlay에서 DataTable을 읽어
 * GetCharacterMovement()->JumpMaxCount 와 JumpZVelocity에 세팅합니다.
 * 더블 점프 스태미나 소모는 UKNAbility_Jump(추후 구현)에서 처리합니다.
 *
 * [단일 책임]
 * 본 구조체는 순수 수치 데이터 저장만 수행합니다.
 */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNJumpSettingRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    // ── 기본 점프 ────────────────────────────────────────────────────────────

    /**
     * @brief 1단 점프 Z축 초속도 (cm/s).
     * @details CharacterMovementComponent::JumpZVelocity에 적용됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Basic")
    float JumpZVelocity = 600.0f;

    /**
     * @brief 1단 점프 시 소모하는 스태미나량.
     * @details 0.0f 설정 시 스태미나 소모 없이 자유롭게 점프합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Basic")
    float JumpStaminaCost = 0.0f;

    // ── 더블 점프 ────────────────────────────────────────────────────────────

    /**
     * @brief 더블 점프 활성화 여부.
     * @details false일 경우 JumpMaxCount를 1로 유지하여 더블 점프를 막습니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Double")
    bool bDoubleJumpEnabled = true;

    /**
     * @brief 더블 점프 Z축 속도 배율. (1.0 = 1단 점프와 동일한 높이)
     * @details 실제 속도 = JumpZVelocity × DoubleJumpVelocityMultiplier.
     * 0.8f로 설정 시 2단 점프가 1단보다 살짝 낮아 자연스러운 느낌을 줍니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Double",
        meta = (ClampMin = 0.1f, ClampMax = 2.0f))
    float DoubleJumpVelocityMultiplier = 0.8f;

    /**
     * @brief 더블 점프 시 소모하는 스태미나량.
     * @details 1단 점프 소모량과 별개로 설정하여 기획 유연성을 높입니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Double")
    float DoubleJumpStaminaCost = 15.0f;

    /**
     * @brief 공중에서 더블 점프 가능 최대 횟수.
     * @details 기본값 1 = 더블 점프 1회.
     * CharacterMovementComponent::JumpMaxCount에
     * (지상 점프 1회 + 이 값)으로 계산되어 적용됩니다.
     * 예: MaxAirJumpCount = 1 → JumpMaxCount = 2 (더블 점프)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Double",
        meta = (ClampMin = 1, ClampMax = 3))
    int32 MaxAirJumpCount = 1;
};
#pragma endregion 점프 설정 테이블

#pragma region 콤보 공격 테이블
/**
 * @struct FKNComboAttackRow
 * @brief  플레이어 콤보 공격 1단계~5단계의 약/강 공격 수치를 정의하는 행 구조체입니다.
 *
 * @details
 * [Row Key 명명 규칙]
 * - 약공격 : LightAttack_1 ~ LightAttack_5
 * - 강공격 : HeavyAttack_1 ~ HeavyAttack_5
 *
 * [콤보 트리 입력 경로]
 * - L          → 약공1
 * - H          → 강공1
 * - LH         → 약공1 → 강공2
 * - LL         → 약공1 → 약공2
 * - LLH        → 약공1 → 약공2 → 강공3
 * - LLL        → 약공1 → 약공2 → 약공3
 * - LLLH       → 약공1 → 약공2 → 약공3 → 강공4
 * - LLLL       → 약공1 → 약공2 → 약공3 → 약공4
 * - LLLLH      → 약공1 → 약공2 → 약공3 → 약공4 → 강공5
 * - LLLLL      → 약공1 → 약공2 → 약공3 → 약공4 → 약공5
 *
 * [단일 책임]
 * 애니메이션 재생, 히트박스 판정, GAS 데미지 GE 적용 등의 실행 로직은
 * UKNAbility_ComboAttack(추후 구현)에서 담당합니다.
 * 본 구조체는 순수 수치 데이터 저장만 수행합니다.
 */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNComboAttackRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    /**
     * @brief 콤보 단계 (1 ~ 5).
     * @details 강공 N은 약공 N-1회 입력 직후 우클릭으로 파생됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Info",
        meta = (ClampMin = 1, ClampMax = 5))
    int32 ComboStep = 1;

    /**
     * @brief 공격 유형. 0 = 약공격(Light), 1 = 강공격(Heavy).
     * @details 어빌리티에서 이 값을 읽어 재생할 몽타주 섹션을 분기합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Info",
        meta = (ClampMin = 0, ClampMax = 1))
    int32 AttackType = 0;

    /**
     * @brief 이 공격을 시전할 때 소모하는 스태미나량.
     * @details 스태미나가 부족하면 어빌리티가 활성화되지 않습니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Cost")
    float StaminaCost = 10.0f;

    /**
     * @brief 다음 공격 입력을 받아들이는 허용 시간 윈도우 (초).
     * @details 이 시간 안에 다음 입력이 없으면 콤보가 초기화됩니다.
     * 강공격·5단계처럼 콤보를 종료하는 공격은 0.0으로 설정합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Timing")
    float ComboWindowTime = 0.8f;

    /**
     * @brief 기본 데미지 배율. (1.0 = 기준치)
     * @details 실제 데미지 = 캐릭터 기본 공격력 × DamageMultiplier.
     * SetByCaller로 GE에 전달됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Combat")
    float DamageMultiplier = 1.0f;

    /**
     * @brief 이 공격이 적중했을 때 획득하는 오버클럭 포인트.
     * @details GainOverclockPoint()를 통해 KNStatsComponent에 전달됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Combat")
    float OverclockGain = 10.0f;

    /** @brief 이 콤보 단계에서 재생할 애니메이션 몽타주 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Visual")
    TObjectPtr<UAnimMontage> ComboMontage = nullptr;

    /** @brief 몽타주 내에서 재생할 특정 섹션 이름 (예: "Light_1", "Heavy_5") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Visual")
    FName MontageSectionName = NAME_None;

    /** @brief 적에게 적중(Hit)했을 때 터뜨릴 이펙트 (VFX) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Impact")
    TObjectPtr<class UNiagaraSystem> HitVFX = nullptr;

    /** @brief 적에게 적중(Hit)했을 때 재생할 타격음 (SFX) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Impact")
    TObjectPtr<USoundBase> HitSound = nullptr;

    /**
     * @brief 히트박스가 활성화되기 시작하는 애니메이션 정규화 시간 (0.0 ~ 1.0).
     * @details AnimNotify 또는 어빌리티의 WaitGameplayEvent로 참조합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Hitbox",
        meta = (ClampMin = 0.0f, ClampMax = 1.0f))
    float HitboxStartNormTime = 0.25f;

    /**
     * @brief 히트박스가 비활성화되는 애니메이션 정규화 시간 (0.0 ~ 1.0).
     * @details HitboxStartNormTime보다 반드시 커야 합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Hitbox",
        meta = (ClampMin = 0.0f, ClampMax = 1.0f))
    float HitboxEndNormTime = 0.55f;
};
#pragma endregion 콤보 공격 테이블

#pragma region 오버클럭 게이지 설정 테이블
/**
 * @struct FKNOverclockSettingRow
 * @brief 오버클럭 게이지의 획득/감소량 및 레벨 임계값을 정의하는 행 구조체입니다.
 * @details OverclockPoint가 임계값에 도달하면 StatsComponent가 해당 레벨의 GameplayTag를 동기화합니다.
 */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNOverclockSettingRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    // ── 최대치 및 임계값 ───────────────────────────────────

    /** @brief 오버클럭 포인트 최대치 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Overclock|Threshold")
    float MaxOverclockPoint = 300.0f;

    /** @brief Lv1 진입 임계값 (전술기 해금) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Overclock|Threshold")
    float Lv1Threshold = 100.0f;

    /** @brief Lv2 진입 임계값 (돌파기 해금) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Overclock|Threshold")
    float Lv2Threshold = 200.0f;

    /** @brief Lv3 진입 임계값 (궁극기 해금) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Overclock|Threshold")
    float Lv3Threshold = 300.0f;

    // ── 획득량 ───────────────────────────────────────────

    /** @brief 일반 콤보 어택 이후 강공격 적중 시 획득량 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Overclock|Gain")
    float GainComboHit = 10.0f;

    /** @brief 퍼펙트 패링 성공 시 획득량 (하이 리스크 하이 리턴) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Overclock|Gain")
    float GainPerfectParry = 50.0f;

    /** @brief 퍼펙트 회피(저스트 회피) 성공 시 획득량 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Overclock|Gain")
    float GainPerfectDodge = 40.0f;

    // ── 감소량 ───────────────────────────────────────────

    /** @brief 전투 이탈 상태일 때 초당 게이지 감소량 (지속 공격 유도 장치) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Overclock|Decay")
    float OverclockDecayRate = 5.0f;
};
#pragma endregion 오버클럭 게이지 설정 테이블