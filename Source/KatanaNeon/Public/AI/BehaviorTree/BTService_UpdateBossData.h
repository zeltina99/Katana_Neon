// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateBossData.generated.h"

/**
 * @file    BTService_UpdateBossData.h
 * @class   UBTService_UpdateBossData
 * @brief   매 틱마다 블랙보드의 플레이어 거리를 갱신하는 BT 서비스입니다.
 *
 * @details
 * [SRP 책임]
 * - 블랙보드 데이터 갱신만 담당합니다.
 * - 루트 노드 아래에 붙여 항상 실행되도록 설정합니다.
 *
 * [갱신 항목]
 * - DistToPlayer : 보스와 플레이어 사이의 거리 (cm)
 * - bIsStunned   : GAS 태그 State.Combat.Groggy 보유 여부
 */
UCLASS(meta = (DisplayName = "보스 데이터 갱신 서비스"))
class KATANANEON_API UBTService_UpdateBossData : public UBTService
{
	GENERATED_BODY()
	
public:
    /** @brief 서비스 기본값 초기화 */
    UBTService_UpdateBossData();

protected:
    /**
     * @brief 서비스 틱 — 블랙보드 데이터를 갱신합니다.
     * @param OwnerComp 비헤이비어 트리 컴포넌트
     * @param NodeMemory 노드 메모리
     * @param DeltaSeconds 델타 시간
     */
    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds) override;
};
