// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "KNEnemyStatTable.generated.h"

/**
 * @file    KNEnemyStatTable.h
 * @brief   KatanaNeon 적/보스 캐릭터의 기획 수치를 CSV/DataTable로 관리하는 구조체 모음입니다.
 * @details 단일 책임 원칙(SRP)에 따라 플레이어 스탯 테이블(KNPlayerStatTable)과 완전히 분리되었습니다.
 */

#pragma region 적 기본 스탯 테이블
 /**
  * @struct FKNEnemyBaseStatRow
  * @brief 근/원거리 일반 적의 공통 기본 수치를 정의하는 행 구조체입니다.
  */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNEnemyBaseStatRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    /** @brief 최대 체력 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Enemy|Stat")
    float MaxHealth = 100.0f;

    /** @brief 기본 이동 속도 (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Enemy|Stat")
    float MoveSpeed = 400.0f;

    /** @brief 시각 감지 반경 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Enemy|Stat")
    float SightRadius = 1500.0f;

    /** @brief 공격 사거리 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Enemy|Stat")
    float AttackRange = 200.0f;

    /**
     * @brief 공격 예고(저스트 회피 판정) 윈도우 지속 시간 (초).
     * @details 이 시간 안에 플레이어가 대시하면 FlurryRush가 발동됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Enemy|Stat")
    float AttackWarningDuration = 0.5f;
};
#pragma endregion 적 기본 스탯 테이블

#pragma region 원거리 적 추가 스탯 테이블
/**
 * @struct FKNEnemyRangedStatRow
 * @brief 원거리 적 전용 추가 수치 구조체입니다.
 * @details 원거리 적의 발사체 속도와 교전 거리 등을 기획자가 조절합니다.
 */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNEnemyRangedStatRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    /** @brief 발사체(총알) 이동 속도 (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Enemy|Ranged")
    float ProjectileSpeed = 1200.0f;

    /** @brief 발사체 생존 시간 (초) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Enemy|Ranged")
    float ProjectileLifeSpan = 5.0f;

    /** @brief 최소 교전 거리 — 이 거리 안으로 들어오면 플레이어로부터 후퇴합니다 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Enemy|Ranged")
    float MinEngagementRange = 600.0f;
};
#pragma endregion 원거리 적 추가 스탯 테이블

#pragma region 보스 페이즈 테이블
/**
 * @struct FKNBossPhaseRow
 * @brief 보스의 페이즈 전환 임계값과 페이즈별 스탯 배율을 정의합니다.
 */
USTRUCT(BlueprintType)
struct KATANANEON_API FKNBossPhaseRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    /**
     * @brief 페이즈 전환 체력 비율 배열 (내림차순).
     * @details 예: {0.7f, 0.4f} → 70%, 40% 체력 도달 시 전환됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Boss|Phase")
    TArray<float> PhaseHealthThresholds = { 0.7f, 0.4f };

    /** @brief 페이즈 전환 연출 지속 시간 (초) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Boss|Phase")
    float TransitionDuration = 2.0f;
};
#pragma endregion 보스 페이즈 테이블