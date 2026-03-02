// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KNCombatEnums.generated.h"

/**
 * @file    KNCombatEnums.h
 * @brief   KatanaNeon 프로젝트의 전투 시스템 전반에서 공통으로 사용되는 열거형(Enum) 모음입니다.
 * @details 헤더 종속성(순환 참조)을 최소화하기 위해 열거형 데이터만 독립적으로 관리합니다.
 */

#pragma region 무기 스탠스 열거형
 /**
  * @enum    EKNWeaponStance
  * @brief   플레이어의 현재 무기 상태(발도/납도)를 정의하는 열거형입니다.
  * @details 콤보 데이터 테이블 분기 및 애니메이션 스탠스 전환에 사용됩니다.
  */
UENUM(BlueprintType)
enum class EKNWeaponStance : uint8
{
    Sheathed    UMETA(DisplayName = "납도 상태 (Sheathed)"),
    Drawn       UMETA(DisplayName = "발도 상태 (Drawn)")
};
#pragma endregion 무기 스탠스 열거형

// 나중에 전투 관련 Enum이 추가로 필요해지면 모두 이곳에 모아두시면 됩니다!
// 예: 공격 타입, 피격 판정 부위 등