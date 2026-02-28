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

    /** @brief 일반 콤보 어택 적중 시 획득량 */
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