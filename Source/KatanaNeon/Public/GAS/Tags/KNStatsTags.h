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
#pragma region 스탯 데이터 태그 선언 (GE SetByCaller 매핑용)
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
        }
    }
#pragma endregion 스탯 데이터 태그 선언 끝

#pragma region 오버클럭 상태 태그 선언 (OverclockPoint 임계값 도달 시 부여)
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
    }
#pragma endregion 오버클럭 상태 태그 선언 끝
}