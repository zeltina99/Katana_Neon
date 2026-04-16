// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateBossData.generated.h"

/**
 * @file    BTService_UpdateBossData.h
 * @class   UBTService_UpdateBossData
 * @brief   매 틱마다 보스 관련 블랙보드 데이터를 갱신하는 BT 서비스입니다.
 *
 * @details
 * [SRP 책임]
 * - 블랙보드 데이터 갱신만 담당합니다.
 * - FBlackboardKeySelector를 사용하여 에디터에서 키를 유연하게 지정할 수 있습니다.
 * - 루트 노드에 붙여 항상 실행되도록 설정합니다.
 *
 * [갱신 항목]
 * - DistToPlayer : 보스와 플레이어 사이의 거리 (cm)
 * - bIsStunned   : GAS 태그 State.Combat.Groggy 보유 여부
 */
UCLASS(meta = (DisplayName = "보스 데이터 갱신 서비스"))
class KATANANEON_API UBTService_UpdateBossData : public UBTService
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    /**
     * @brief 서비스 기본값 및 블랙보드 키 필터를 초기화합니다.
     * @details 타입 안전성을 위해 각 키에 올바른 타입 필터를 설정합니다.
     */
    UBTService_UpdateBossData();

protected:
    /**
     * @brief 블랙보드 에셋 연결 시 키를 실제 인덱스로 해결합니다.
     * @param Asset 연결된 비헤이비어 트리 에셋
     */
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
#pragma endregion 기본 생성자 및 초기화

#pragma region 서비스 오버라이드
protected:
    /**
     * @brief 서비스 틱 — 플레이어 거리와 그로기 상태를 블랙보드에 갱신합니다.
     * @param OwnerComp 비헤이비어 트리 컴포넌트
     * @param NodeMemory 노드 메모리
     * @param DeltaSeconds 델타 시간
     */
    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds) override;
#pragma endregion 서비스 오버라이드

#pragma region 에디터 노출 블랙보드 키
protected:
    /** @brief 블랙보드의 TargetPlayer 키와 연결할 셀렉터 */
    UPROPERTY(EditAnywhere, Category = "KatanaNeon|Blackboard")
    FBlackboardKeySelector TargetPlayerKey;

    /** @brief 블랙보드의 DistToPlayer 키와 연결할 셀렉터 */
    UPROPERTY(EditAnywhere, Category = "KatanaNeon|Blackboard")
    FBlackboardKeySelector DistToPlayerKey;

    /** @brief 블랙보드의 bIsStunned 키와 연결할 셀렉터 */
    UPROPERTY(EditAnywhere, Category = "KatanaNeon|Blackboard")
    FBlackboardKeySelector IsStunnedKey;
#pragma endregion 에디터 노출 블랙보드 키

};
