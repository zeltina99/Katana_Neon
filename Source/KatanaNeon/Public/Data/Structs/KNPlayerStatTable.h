// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/Enums/KNCombatEnums.h"
#include "KNPlayerStatTable.generated.h"

#pragma region 전방 선언
class UAnimMontage;
class UNiagaraSystem;
class USoundBase;
#pragma endregion 전방 선언
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

    /** @brief 기본 공격 속도 배율 (1.0 = 100%) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Combat")
    float AttackSpeed = 1.0f;

    /**
     * @brief 달리기(Sprint) 시 적용되는 최대 이동 속도 (cm/s).
     * @details IA_Sprint 입력 유지 시 이 속도로 전환됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Stats|Movement")
    float SprintSpeed = 1000.0f;

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
     * @brief 최대 점프 횟수.
     * @details CharacterMovementComponent::JumpMaxCount에 직접 적용됩니다.
     *          1 = 단일 점프, 2 = 더블 점프 허용.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Basic",
        meta = (ClampMin = 1, ClampMax = 3))
    int32 MaxJumpCount = 2;

    /**
     * @brief 1단 점프 Z축 초속도 (cm/s).
     * @details CharacterMovementComponent::JumpZVelocity에 적용됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Basic",
        meta = (ClampMin = 100.0f, ClampMax = 3000.0f))
    float JumpZVelocity = 600.0f;

    /**
     * @brief 1단 점프 시 소모하는 스태미나량.
     * @details 0.0f 설정 시 스태미나 소모 없이 자유롭게 점프합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Basic")
    float JumpStaminaCost = 0.0f;

    /**
     * @brief 더블 점프 활성화 여부.
     * @details false이면 MaxJumpCount를 1로 강제하여 더블 점프를 막습니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Basic")
    bool bDoubleJumpEnabled = true;

    // ── 더블 점프 ────────────────────────────────────────────────────────────

    /**
     * @brief 2단계(더블) 점프 시 LaunchCharacter에 전달할 Z 충격량 (cm/s).
     * @details 절댓값 방식 사용. 1단 점프보다 살짝 낮게 설정하여 자연스러운 체감을 줍니다.
     *          UKNAbility_Jump::PerformDoubleJump()에서 LaunchCharacter()에 전달됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Double",
        meta = (ClampMin = 100.0f, ClampMax = 3000.0f))
    float DoubleJumpZVelocity = 500.0f;

    /**
     * @brief 더블 점프 시 소모하는 스태미나량.
     * @details 1단 점프 소모량과 별개로 설정하여 기획 유연성을 높입니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Double")
    float DoubleJumpStaminaCost = 15.0f;

    /**
     * @brief 더블 점프 중 가로 방향 공중 제어 배율 (0.0 = 무제어, 1.0 = 지상과 동일).
     * @details UCharacterMovementComponent::AirControl을 임시로 이 값으로 덮어씁니다.
     *          착지 시 KNPlayerCharacter::Landed()에서 원래 값으로 복구됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Double",
        meta = (ClampMin = 0.0f, ClampMax = 1.0f))
    float DoubleJumpAirControl = 0.4f;

    /**
     * @brief 더블 점프 몽타주 재생 배율 (1.0 = 기본 속도).
     * @details UKNAbility_Jump::PerformDoubleJump()에서 PlayAnimMontage() 호출 시 전달됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Double",
        meta = (ClampMin = 0.1f, ClampMax = 5.0f))
    float DoubleJumpMontagePlayRate = 1.2f;

    /**
     * @brief 더블 점프 사용 후 착지 전 추가 중력 배율.
     * @details UCharacterMovementComponent::GravityScale에 임시 적용됩니다.
     *          1.0 = 기본 중력, 값이 클수록 더 빠르게 낙하합니다.
     *          착지 시 Landed()에서 1.0으로 복구됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Jump|Double",
        meta = (ClampMin = 1.0f, ClampMax = 5.0f))
    float FallGravityScale = 1.5f;
};
#pragma endregion 점프 설정 테이블

#pragma region 대시 몽타주 테이블
/**
 * @struct FKNDashMontageRow
 * @brief 대시 방향별 몽타주를 스탠스에 따라 관리하는 행 구조체입니다.
 */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNDashMontageRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    /** @brief 납도 상태 대시 몽타주 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Dash|Montage")
    TObjectPtr<UAnimMontage> SheathMontage = nullptr;

    /** @brief 발도 상태 대시 몽타주 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Dash|Montage")
    TObjectPtr<UAnimMontage> DrawnMontage = nullptr;
};
#pragma endregion 대시 몽타주 테이블

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
    // 스탠스와 배속 변수 추가 ──
    /**
     * @brief 이 콤보 공격이 요구하는 무기 스탠스입니다.
     * @details 발도 상태일 때는 Drawn 콤보만, 납도 상태일 때는 Sheathed 콤보만 발동됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Info")
    EKNWeaponStance RequiredStance = EKNWeaponStance::Drawn;

    /**
     * @brief 이 콤보 단계의 애니메이션 재생 배율 (1.0 = 기본 속도).
     * @details 발도술은 1.2 등 빠르게, 검술은 0.8 등 묵직하게 기획자가 직접 타격감을 조절합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Combo|Combat", meta = (ClampMin = 0.1f, ClampMax = 5.0f))
    float PlayRate = 1.0f;

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
    TObjectPtr<UNiagaraSystem> HitVFX = nullptr;

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

    /** @brief 게임 시작 시 오버클럭 초기 포인트 (보통 0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock")
    float InitialOverclockPoint = 0.0f;

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

#pragma region 크로노스 구체 감속 설정 테이블
/**
 * @struct FKNChronosSettingRow
 * @brief  크로노스(시간 감속 구체) 어빌리티의 수치를 정의하는 DataTable 행입니다.
 *
 * @details
 * [작동 방식]
 * - 플레이어에 부착된 KNChronosSphereComponent의 콜리전 반경 내부에 진입한
 *   적 캐릭터와 적 발사체에만 CustomTimeDilation이 적용됩니다.
 * - 플레이어 자신은 항상 CustomTimeDilation = 1.0을 유지합니다.
 * - GlobalTimeDilation은 변경하지 않습니다 (기존 크로노스 기획 변경).
 *
 * [연결 에셋]
 * DataTable 에셋: DT_ChronosSetting, Row Key: "Default"
 */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNChronosSettingRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    /**
     * @brief 플레이어에 부착되는 크로노스 구체 콜리전 반경 (cm).
     * @details 이 반경 내에 진입한 적/발사체에 시간 감속이 즉시 적용됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Chronos",
        meta = (ClampMin = 100.0f, ClampMax = 3000.0f))
    float SphereRadius = 600.0f;

    /**
     * @brief 구체 내 적 캐릭터에 적용되는 CustomTimeDilation 배율.
     * @details 0.3 = 30% 속도. 0에 가까울수록 거의 정지 상태처럼 보입니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Chronos",
        meta = (ClampMin = 0.05f, ClampMax = 0.95f))
    float EnemySlowScale = 0.3f;

    /**
     * @brief 구체 내 적 발사체에 적용되는 CustomTimeDilation 배율.
     * @details 적 본체보다 더 느리게 설정하여 플레이어에게 충분한 반응 시간을 줍니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Chronos",
        meta = (ClampMin = 0.01f, ClampMax = 0.95f))
    float ProjectileSlowScale = 0.1f;

    /**
     * @brief 크로노스 게이지 초당 소모량.
     * @details 매 DrainTickInterval마다 (DrainRatePerSecond × DrainTickInterval)만큼 소모됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Chronos",
        meta = (ClampMin = 1.0f))
    float DrainRatePerSecond = 20.0f;

    /**
     * @brief 크로노스 소모 GE 적용 주기 (초).
     * @details 0.1초 권장. 너무 짧으면 GE 호출 빈도가 높아져 퍼포먼스에 영향을 줍니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Chronos",
        meta = (ClampMin = 0.05f, ClampMax = 1.0f))
    float DrainTickInterval = 0.1f;
};
#pragma endregion 크로노스 구체 감속 설정 테이블

#pragma region 오버클럭 1단계 어빌리티 설정 테이블
/**
 * @struct FKNOverclockLv1Row
 * @brief  오버클럭 1단계(전술 강화) 어빌리티의 수치를 정의하는 DataTable 행입니다.
 *
 * @details
 * [효과 요약]
 * - Duration(10s) 동안 아래 버프를 Duration GE로 적용합니다.
 *   • AttackSpeed   : +AttackSpeedAdditive (AdditiveDelta 방식)
 *   • MovementSpeed : +MovementSpeedAdditive (cm/s 절대 추가량)
 *   • DamageMultiplier : State.Combat.OverclockTactical 태그로 ComboAttack 어빌리티가 조회
 *   • StaminaImmune : State.Combat.StaminaImmune Loose 태그 부여
 *
 * [연결 에셋]
 * DataTable 에셋: DT_OverclockLv1Setting, Row Key: "Default"
 */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNOverclockLv1Row : public FTableRowBase
{
    GENERATED_BODY()

public:
    /** @brief 전술 강화 지속 시간 (초) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv1",
        meta = (ClampMin = 1.0f, ClampMax = 60.0f))
    float Duration = 10.0f;

    /**
     * @brief 공격 속도 Additive 증가량.
     * @details AttackSpeed Attribute에 덧셈 Delta로 적용됩니다.
     *          예: 기준 1.0 + 0.4 = 최종 1.4배 → 몽타주 PlayRate도 연동됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv1",
        meta = (ClampMin = 0.0f, ClampMax = 5.0f))
    float AttackSpeedAdditive = 0.4f;

    /**
     * @brief 이동 속도 추가량 (cm/s).
     * @details CharacterMovementComponent::MaxWalkSpeed에 Additive로 적용됩니다.
     *          예: 기준 600 + 150 = 750 cm/s.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv1",
        meta = (ClampMin = 0.0f, ClampMax = 1000.0f))
    float MovementSpeedAdditive = 150.0f;

    /**
     * @brief 공격력 배율.
     * @details State.Combat.OverclockTactical 태그 존재 시 ComboAttack 어빌리티가 이 값을 조회합니다.
     *          BaseDamage × DamageMultiplier 형태로 곱해집니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv1",
        meta = (ClampMin = 1.0f, ClampMax = 10.0f))
    float DamageMultiplier = 1.5f;

    /**
     * @brief 스태미나 소모 면역 여부.
     * @details true이면 State.Combat.StaminaImmune 태그를 부여하여
     *          모든 스태미나 소모 GE의 SetByCaller 값을 0으로 처리합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv1")
    bool bStaminaImmune = true;

    /**
     * @brief 오버클럭 포인트 소모량.
     * @details KNStatsComponent::ConsumeOverclockLevel(1) 호출 시 차감됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv1",
        meta = (ClampMin = 1.0f))
    float OverclockCost = 100.0f;
};
#pragma endregion 오버클럭 1단계 어빌리티 설정 테이블

#pragma region 오버클럭 2단계 어빌리티 설정 테이블
/**
 * @struct FKNOverclockLv2Row
 * @brief  오버클럭 2단계(참격파) 어빌리티의 수치를 정의하는 DataTable 행입니다.
 *
 * @details
 * [효과 요약]
 * - 전방으로 AKNSlashProjectile 발사체를 스폰합니다.
 * - 피격 대상 ASC에 SlashDamage 즉시 적용 + State.Combat.Groggy Duration GE를 부여합니다.
 * - 그로기 상태 동안 AI/보스가 행동 불능 상태가 됩니다.
 *
 * [연결 에셋]
 * DataTable 에셋: DT_OverclockLv2Setting, Row Key: "Default"
 */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNOverclockLv2Row : public FTableRowBase
{
    GENERATED_BODY()

public:
    /** @brief 참격파 발사체 이동 속도 (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv2",
        meta = (ClampMin = 100.0f, ClampMax = 10000.0f))
    float SlashSpeed = 2500.0f;

    /** @brief 참격파 발사체가 소멸하는 최대 이동 거리 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv2",
        meta = (ClampMin = 100.0f, ClampMax = 5000.0f))
    float SlashMaxDistance = 1500.0f;

    /**
     * @brief 참격파 발사체의 Box Collision 절반 크기 (cm).
     * @details X = 전방 깊이, Y = 폭(너비), Z = 높이.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv2")
    FVector SlashBoxHalfExtent = FVector(50.0f, 80.0f, 40.0f);

    /** @brief 참격파 피격 시 즉시 적용하는 데미지 (Instant GE) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv2",
        meta = (ClampMin = 0.0f))
    float SlashDamage = 40.0f;

    /**
     * @brief 그로기(행동 불능) 상태 지속 시간 (초).
     * @details State.Combat.Groggy Duration GE의 지속 시간으로 사용됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv2",
        meta = (ClampMin = 0.5f, ClampMax = 30.0f))
    float GrogyDuration = 3.0f;

    /**
     * @brief 오버클럭 포인트 소모량.
     * @details KNStatsComponent::ConsumeOverclockLevel(2) 호출 시 차감됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv2",
        meta = (ClampMin = 1.0f))
    float OverclockCost = 200.0f;
};
#pragma endregion 오버클럭 2단계 어빌리티 설정 테이블

#pragma region 오버클럭 3단계 어빌리티 설정 테이블
/**
 * @struct FKNOverclockLv3Row
 * @brief  오버클럭 3단계(시간 파열) 궁극기 어빌리티의 수치를 정의하는 DataTable 행입니다.
 *
 * @details
 * [효과 요약]
 * - GlobalTimeDilation = WorldTimeDilationScale(≈0.0001) → 월드 거의 정지
 * - 플레이어 CustomTimeDilation = 1.0 / WorldTimeDilationScale → 플레이어는 정상 속도
 * - TimeStopDuration(실초 기준) 경과 후 자동 해제
 * - 정지 중 ComboAttack의 데미지에 FrozenDamageMultiplier 추가 적용 가능
 *
 * [FTimerManager 참고사항]
 * FTimerManager는 GlobalTimeDilation의 영향을 받지 않으므로 실제 시간 기준으로 동작합니다.
 *
 * [연결 에셋]
 * DataTable 에셋: DT_OverclockLv3Setting, Row Key: "Default"
 */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNOverclockLv3Row : public FTableRowBase
{
    GENERATED_BODY()

public:
    /**
     * @brief 시간 정지 지속 시간 (실월드 초 기준).
     * @details FTimerManager 기준이므로 GlobalTimeDilation과 무관하게 실시간으로 카운트됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv3",
        meta = (ClampMin = 1.0f, ClampMax = 30.0f))
    float TimeStopDuration = 5.0f;

    /**
     * @brief 시간 정지 중 GlobalTimeDilation 값.
     * @details 완전한 0은 엔진 물리 연산 오류를 유발하므로 0.0001을 권장합니다.
     *          플레이어 CustomTimeDilation = 1.0 / WorldTimeDilationScale으로 자동 보정됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv3",
        meta = (ClampMin = 0.0001f, ClampMax = 0.05f))
    float WorldTimeDilationScale = 0.0001f;

    /**
     * @brief 시간 정지 중 데미지 배율.
     * @details 1.0 = 원래 데미지 그대로, 1.5 = 보너스 50% 추가.
     *          ComboAttack 어빌리티가 State.Combat.WorldTimeFrozen 태그 존재 시 이 값을 조회합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv3",
        meta = (ClampMin = 0.1f, ClampMax = 10.0f))
    float FrozenDamageMultiplier = 1.0f;

    /**
     * @brief 오버클럭 포인트 소모량.
     * @details KNStatsComponent::ConsumeOverclockLevel(3) 호출 시 차감됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Overclock|Lv3",
        meta = (ClampMin = 1.0f))
    float OverclockCost = 300.0f;
};
#pragma endregion 오버클럭 3단계 어빌리티 설정 테이블


