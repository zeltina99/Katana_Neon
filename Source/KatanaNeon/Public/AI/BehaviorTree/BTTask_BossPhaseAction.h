// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossPhaseAction.generated.h"

#pragma region 전방 선언
class UAnimMontage;
#pragma endregion 전방 선언

/**
 * @file    BTTask_BossPhaseAction.h
 * @class   UBTTask_BossPhaseAction
 * @brief   보스 페이즈 전환 시 전환 연출(몽타주 재생, 무적 태그 부여)을 처리하는 BT 태스크입니다.
 *
 * @details
 * [SRP 책임]
 * - 페이즈 전환 연출(무적 + 몽타주)만 담당합니다.
 * - 페이즈 전환 판단은 KNBossBase의 CheckPhaseTransition에 위임합니다.
 *
 * [동작 흐름]
 * 1. Invincible 태그 부여 (전환 중 피격 방지)
 * 2. 전환 몽타주 재생
 * 3. 몽타주 완료 후 Invincible 태그 제거 → Succeeded 반환
 */
UCLASS(meta = (DisplayName = "보스 페이즈 전환 연출"))
class KATANANEON_API UBTTask_BossPhaseAction : public UBTTaskNode
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    /**
     * @brief 태스크 기본값을 초기화합니다.
     */
    UBTTask_BossPhaseAction();
#pragma endregion 기본 생성자 및 초기화

#pragma region 태스크 오버라이드
protected:
    /**
     * @brief 태스크 실행 — 무적 태그 부여 및 전환 몽타주를 재생합니다.
     * @param OwnerComp 비헤이비어 트리 컴포넌트
     * @param NodeMemory 노드 메모리
     * @return 성공/실패
     */
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;
#pragma endregion 태스크 오버라이드

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 페이즈 전환 시 재생할 몽타주.
     * @details nullptr이면 연출 없이 즉시 완료됩니다.
     */
    UPROPERTY(EditAnywhere, Category = "KatanaNeon|Boss|Phase")
    TObjectPtr<UAnimMontage> PhaseTransitionMontage = nullptr;

    /**
     * @brief 전환 연출 중 무적 지속 시간 (초).
     * @details 몽타주 길이와 맞추는 것을 권장합니다.
     */
    UPROPERTY(EditAnywhere, Category = "KatanaNeon|Boss|Phase",
        meta = (ClampMin = 0.0f, ClampMax = 30.0f))
    float InvincibleDuration = 3.0f;
#pragma endregion 에디터 설정 데이터

};
