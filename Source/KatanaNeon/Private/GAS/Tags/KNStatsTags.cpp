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
            UE_DEFINE_GAMEPLAY_TAG(MaxOverclockPoint, "KatanaNeon.Data.Stats.MaxOverclockPoint")

            // ── 전투 파라미터 ─────────────────────────────────
            UE_DEFINE_GAMEPLAY_TAG(AttackSpeed, "KatanaNeon.Data.Stats.AttackSpeed")
        }
    }
#pragma endregion 스탯 데이터 태그 정의

#pragma region 오버클럭 상태 태그 정의
    namespace State
    {
        namespace Overclock
        {
            UE_DEFINE_GAMEPLAY_TAG(Lv1, "KatanaNeon.State.Overclock.Lv1")
            UE_DEFINE_GAMEPLAY_TAG(Lv2, "KatanaNeon.State.Overclock.Lv2")
            UE_DEFINE_GAMEPLAY_TAG(Lv3, "KatanaNeon.State.Overclock.Lv3")
        }
        namespace Combat
        {
            // ── 방어 상태 ─────────────────────────────────────
            UE_DEFINE_GAMEPLAY_TAG(Invincible, "KatanaNeon.State.Combat.Invincible")
            UE_DEFINE_GAMEPLAY_TAG(FlurryRush, "KatanaNeon.State.Combat.FlurryRush")

            namespace Combo
            {
                // ── 콤보 진행 단계 ────────────────────────────
                UE_DEFINE_GAMEPLAY_TAG(Step1, "KatanaNeon.State.Combat.Combo.Step1")
                UE_DEFINE_GAMEPLAY_TAG(Step2, "KatanaNeon.State.Combat.Combo.Step2")
                UE_DEFINE_GAMEPLAY_TAG(Step3, "KatanaNeon.State.Combat.Combo.Step3")
            }
        }
        namespace Chronos
        {
            // ── 크로노스 활성 상태 ───────────────────────────
            UE_DEFINE_GAMEPLAY_TAG(Active, "KatanaNeon.State.Chronos.Active")
        }
    }
#pragma endregion 오버클럭 상태 태그 정의

#pragma region 어빌리티 실행 태그 정의
    namespace Ability
    {
        namespace Combat
        {
            // ── 근접 전투 어빌리티 ───────────────────────────
            UE_DEFINE_GAMEPLAY_TAG(Attack, "KatanaNeon.Ability.Combat.Attack")
            UE_DEFINE_GAMEPLAY_TAG(Parry, "KatanaNeon.Ability.Combat.Parry")
            UE_DEFINE_GAMEPLAY_TAG(Dash, "KatanaNeon.Ability.Combat.Dash")
        }
        // 적 전용 어빌리티 태그
        namespace Enemy
        {
            // ── 적 전용 어빌리티 실행 태그 ───────────────────
            UE_DEFINE_GAMEPLAY_TAG(MeleeAttack, "KatanaNeon.Ability.Enemy.MeleeAttack")
            UE_DEFINE_GAMEPLAY_TAG(RangedAttack, "KatanaNeon.Ability.Enemy.RangedAttack")
            UE_DEFINE_GAMEPLAY_TAG(ChargeAttack, "KatanaNeon.Ability.Enemy.ChargeAttack")
        }
        namespace Skill
        {
            // ── 특수 스킬 어빌리티 ───────────────────────────
            UE_DEFINE_GAMEPLAY_TAG(Chronos, "KatanaNeon.Ability.Skill.Chronos")
        }
        namespace Overclock
        {
            // ── 오버클럭 단계별 어빌리티 ─────────────────────
            UE_DEFINE_GAMEPLAY_TAG(Lv1, "KatanaNeon.Ability.Overclock.Lv1")
            UE_DEFINE_GAMEPLAY_TAG(Lv2, "KatanaNeon.Ability.Overclock.Lv2")
            UE_DEFINE_GAMEPLAY_TAG(Lv3, "KatanaNeon.Ability.Overclock.Lv3")
        }
    }
#pragma endregion 어빌리티 실행 태그 정의
}
