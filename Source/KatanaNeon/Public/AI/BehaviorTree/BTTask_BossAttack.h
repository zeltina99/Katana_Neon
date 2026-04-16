// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossAttack.generated.h"

/**
 * @file    BTTask_BossAttack.h
 * @class   UBTTask_BossAttack
 * @brief   보스가 플레이어를 향해 공격 어빌리티를 발동하는 BT 태스크입니다.
 *
 * @details
 * [SRP 책임]
 * - ASC를 통해 지정된 어빌리티 태그로 공격만 실행합니다.
 * - 공격 애니메이션 완료 여부는 GAS 어빌리티 내부에서 처리합니다.
 *
 * [완료 조건]
 * - TryActivateAbilitiesByTag 성공 → 즉시 Succeeded 반환
 * - 실패 → Failed 반환
 */
UCLASS(meta = (DisplayName = "보스 공격 실행"))
class KATANANEON_API UBTTask_BossAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_BossAttack();

protected:
    /**
     * @brief 태스크 실행 — 공격 어빌리티를 활성화합니다.
     * @param OwnerComp 비헤이비어 트리 컴포넌트
     * @param NodeMemory 노드 메모리
     * @return 성공/실패/진행 중
     */
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 활성화할 공격 어빌리티의 GameplayTag.
     * @details 에디터에서 페이즈별로 다른 태그를 설정하여 다양한 공격 패턴을 구성합니다.
     */
    UPROPERTY(EditAnywhere, Category = "KatanaNeon|Boss|Attack")
    FGameplayTag AttackAbilityTag;

    /**
     * @brief 공격 후 대기 시간 (초).
     * @details 공격 쿨타임 역할을 합니다.
     */
    UPROPERTY(EditAnywhere, Category = "KatanaNeon|Boss|Attack",
        meta = (ClampMin = 0.0f, ClampMax = 10.0f))
    float AttackCooldown = 1.5f;
#pragma endregion 에디터 설정 데이터
};
