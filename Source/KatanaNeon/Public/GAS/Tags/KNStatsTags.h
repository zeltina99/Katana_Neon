// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * @file    KNStatsTags.h
 * @brief   KatanaNeon 프로젝트 전체에서 사용하는 C++ 네이티브 게임플레이 태그(Native Gameplay Tags) 선언부입니다.
 * @details 단일 책임 원칙(SRP)에 따라 하드코딩된 문자열 태그 검색을 방지하고,
 * 메모리에 캐싱된 네이티브 태그를 제공하여 런타임 성능을 극대화(최적화)합니다.
 */

namespace KatanaNeon
{
#pragma region 스탯 데이터 태그 선언
    namespace Data
    {
        namespace Stats
        {
            // ── 생존 및 기동 ──────────────────────────────────
            /** @brief 현재 체력을 의미하는 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Health)
            /** @brief 최대 체력을 의미하는 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxHealth)
            /** @brief 캐릭터의 기본 이동 속도를 의미하는 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MovementSpeed)

            // ── 액션 자원: 스태미나 ──────────────────────────
            /** @brief 현재 스태미나를 의미하는 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stamina)
            /** @brief 최대 스태미나를 의미하는 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxStamina)
            /** @brief 스태미나 초당 회복률을 의미하는 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(StaminaRegenRate)

            // ── 기믹 자원: 크로노스 ──────────────────────────
            /** @brief 현재 크로노스(시간 감속) 게이지를 의미하는 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chronos)
            /** @brief 최대 크로노스 게이지를 의미하는 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxChronos)

            // ── 보상 자원: 오버클럭 ───────────────────
            /** @brief 누적된 오버클럭 포인트를 의미하는 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(OverclockPoint)
            /** @brief 오버클럭 포인트의 최대치 한도를 의미하는 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxOverclockPoint)

            // ── 전투 파라미터 ─────────────────────────────────
            /** @brief 공격 애니메이션 재생 배율을 의미하는 태그 (1.0 = 기본, 높을수록 빠름) */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttackSpeed)
        }
    }
#pragma endregion 스탯 데이터 태그 선언

#pragma region 오버클럭 상태 태그 선언
    namespace State
    {
        namespace Overclock
        {
            /** @brief 오버클럭 1단계 도달 상태 (전술기 사용 가능) */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Lv1)
            /** @brief 오버클럭 2단계 도달 상태 (돌파기 사용 가능) */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Lv2)
            /** @brief 오버클럭 3단계 도달 상태 (궁극기 사용 가능) */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Lv3)
        }

        namespace Combat
        {
            // ── 방어 상태 ─────────────────────────────────────
            /** @brief 대시 무적 프레임 활성 상태. 피격 처리 쪽에서 이 태그 유무로 피해를 무시합니다. */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Invincible)
            /** @brief 저스트 회피 성공 시 진입하는 플러리 러시 상태. 월드 시간이 느려지고 공격속도가 가속됩니다. */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(FlurryRush)

            namespace Combo
            {
                // ── 콤보 진행 단계 ────────────────────────────
                /** @brief 3타 콤보 1단계 진행 중 상태 */
                KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Step1)
                /** @brief 3타 콤보 2단계 진행 중 상태 */
                KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Step2)
                /** @brief 3타 콤보 3단계 진행 중 상태 */
                KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Step3)
            }
        }

        namespace Chronos
        {
            // ── 크로노스 활성 상태 ───────────────────────────
            /** @brief 시간 감속 스킬이 현재 활성화된 상태. 크로노스 게이지가 소모되는 동안 유지됩니다. */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Active)
        }
    }
#pragma endregion 오버클럭 상태 태그 선언

#pragma region 어빌리티 실행 태그 선언 (TryActivateAbilitiesByTag 연동용)
    /**
     * @namespace KatanaNeon::Ability
     * @brief 입력 콜백에서 GAS 어빌리티를 활성화할 때 사용하는 식별 태그입니다.
     * @details FGameplayTag::RequestGameplayTag(문자열) 하드코딩을 완전히 대체하여
     * 매 입력 프레임의 문자열 파싱 비용과 오타 버그를 원천 차단합니다.
     */
    namespace Ability
    {
        namespace Combat
        {
            // ── 근접 전투 어빌리티 ───────────────────────────
            /** @brief 기본 공격(3타 콤보) 어빌리티 식별 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack)
            /** @brief 패링/가드 어빌리티 식별 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Parry)
            /** @brief 대시(순간 무적 프레임) 어빌리티 식별 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dash)
        }

        // 적 전용 어빌리티 태그
        namespace Enemy
        {
            // ── 적 전용 어빌리티 실행 태그 ───────────────────
            /** @brief 근접 적 공격 어빌리티 식별 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MeleeAttack)
            /** @brief 원거리 적 발사체 공격 어빌리티 식별 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(RangedAttack)
            /** @brief 적 돌진 공격 어빌리티 식별 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ChargeAttack)
        }

        namespace Skill
        {
            // ── 특수 스킬 어빌리티 ───────────────────────────
            /** @brief 시간 감속(크로노스) 토글 어빌리티 식별 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chronos)
        }

        namespace Overclock
        {
            // ── 오버클럭 단계별 어빌리티 ─────────────────────
            /** @brief 오버클럭 1단계 전술기 어빌리티 식별 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Lv1)
            /** @brief 오버클럭 2단계 돌파기 어빌리티 식별 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Lv2)
            /** @brief 오버클럭 3단계 궁극기 어빌리티 식별 태그 */
            KATANANEON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Lv3)
        }
    }
#pragma endregion 어빌리티 실행 태그 선언
}