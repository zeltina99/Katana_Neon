// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "KNEnemyController.generated.h"

#pragma region 전방 선언
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UBehaviorTree;
class UBlackboardComponent;
#pragma endregion 전방 선언

/**
 * @class  AKNEnemyController
 * @brief  적 AI의 두뇌 역할을 담당하는 컨트롤러입니다.
 * @details 시각 감지(Perception)와 비헤이비어 트리(BT) 구동을 전담합니다.
 */
UCLASS()
class KATANANEON_API AKNEnemyController : public AAIController
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    AKNEnemyController();

protected:
    /** @brief 컨트롤러가 적 캐릭터(Pawn)에 빙의할 때 호출되어 BT를 실행합니다. */
    virtual void OnPossess(APawn* InPawn) override;
#pragma endregion 기본 생성자 및 초기화

#pragma region AI 감지 컴포넌트 (Perception)
protected:
    /** @brief 시각, 청각 등 AI의 감각을 통제하는 컴포넌트입니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|AI", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAIPerceptionComponent> EnemyPerceptionComponent = nullptr;

    /** @brief 시각 감지 설정 (시야각, 감지 거리 등) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|AI", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAISenseConfig_Sight> SightConfig = nullptr;

    /** @brief 감각 업데이트 시 호출되는 콜백 함수입니다. */
    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);
#pragma endregion AI 감지 컴포넌트 (Perception)

#pragma region 블랙보드 연동 설정
protected:
    /**
     * @brief 타겟(플레이어)을 저장할 블랙보드 키입니다.
     * @details 에디터에서 드롭다운으로 선택하게 하여 오타로 인한 버그를 원천 차단합니다.
     */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|AI|Blackboard")
    FBlackboardKeySelector TargetActorKey;
#pragma endregion 블랙보드 연동 설정
};
