// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Tags/KNStatsTags.h"

/**
 * @file    KNStatsTags.cpp
 * @brief   KNStatsTags.h에서 선언한 네이티브 게임플레이 태그들의 실제 정의부입니다.
 * @details 엔진이 초기화될 때 이곳의 매크로들을 통해 태그 매니저에 빠르고 안전하게 등록됩니다.
 */

namespace KatanaNeon
{
#pragma region 스탯 데이터 태그 정의
    namespace Data
    {
        namespace Stats
        {
                // ── 생존 및 기동 ──────────────────────────────────
                UE_DEFINE_GAMEPLAY_TAG(Health, "KatanaNeon.Data.Stats.Health")
                UE_DEFINE_GAMEPLAY_TAG(MaxHealth, "KatanaNeon.Data.Stats.MaxHealth")
                UE_DEFINE_GAMEPLAY_TAG(MovementSpeed, "KatanaNeon.Data.Stats.MovementSpeed")

                // ── 액션 자원: 스태미나 ──────────────────────────
                UE_DEFINE_GAMEPLAY_TAG(Stamina, "KatanaNeon.Data.Stats.Stamina")
                UE_DEFINE_GAMEPLAY_TAG(MaxStamina, "KatanaNeon.Data.Stats.MaxStamina")
                UE_DEFINE_GAMEPLAY_TAG(StaminaRegenRate, "KatanaNeon.Data.Stats.StaminaRegenRate")

                // ── 기믹 자원: 크로노스 ──────────────────────────
                UE_DEFINE_GAMEPLAY_TAG(Chronos, "KatanaNeon.Data.Stats.Chronos")
                UE_DEFINE_GAMEPLAY_TAG(MaxChronos, "KatanaNeon.Data.Stats.MaxChronos")

                // ── 보상 자원: 오버클럭 ───────────────────
                UE_DEFINE_GAMEPLAY_TAG(OverclockPoint, "KatanaNeon.Data.Stats.OverclockPoint")
        }
    }
#pragma endregion 스탯 데이터 태그 정의 끝

#pragma region 오버클럭 상태 태그 정의
    namespace State
    {
        namespace Overclock
        {
                UE_DEFINE_GAMEPLAY_TAG(Lv1, "KatanaNeon.State.Overclock.Lv1")
                UE_DEFINE_GAMEPLAY_TAG(Lv2, "KatanaNeon.State.Overclock.Lv2")
                UE_DEFINE_GAMEPLAY_TAG(Lv3, "KatanaNeon.State.Overclock.Lv3")
        }
    }
#pragma endregion 오버클럭 상태 태그 정의 끝
}
