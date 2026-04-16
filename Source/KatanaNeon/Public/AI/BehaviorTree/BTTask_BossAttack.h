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
	
#pragma region 기본 생성자 및 초기화
public:
    /**
     * @brief 태스크 기본값 및 블랙보드 키 필터를 초기화합니다.
     */
    UBTTask_BossAttack();

protected:
    /**
     * @brief 블랙보드 에셋 연결 시 키를 실제 인덱스로 해결합니다.
     * @param Asset 연결된 비헤이비어 트리 에셋
     */
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
#pragma endregion 기본 생성자 및 초기화

#pragma region 태스크 오버라이드
protected:
    /**
     * @brief 태스크 실행 — 플레이어 방향 회전 후 공격 어빌리티를 활성화합니다.
     * @param OwnerComp 비헤이비어 트리 컴포넌트
     * @param NodeMemory 노드 메모리
     * @return 성공/실패
     */
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;
#pragma endregion 태스크 오버라이드

#pragma region 에디터 노출 블랙보드 키
protected:
    /**
     * @brief 감지된 플레이어 액터 키.
     * @details Object 타입 키만 선택 가능합니다.
     */
    UPROPERTY(EditAnywhere, Category = "KatanaNeon|Blackboard")
    FBlackboardKeySelector TargetPlayerKey;

    /**
     * @brief 공격 중 여부 키.
     * @details Bool 타입 키만 선택 가능합니다.
     */
    UPROPERTY(EditAnywhere, Category = "KatanaNeon|Blackboard")
    FBlackboardKeySelector IsAttackingKey;
#pragma endregion 에디터 노출 블랙보드 키

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 활성화할 공격 어빌리티의 GameplayTag.
     * @details 에디터에서 페이즈별로 다른 태그를 설정하여 다양한 공격 패턴을 구성합니다.
     */
    UPROPERTY(EditAnywhere, Category = "KatanaNeon|Boss|Attack")
    FGameplayTag AttackAbilityTag;

    /**
     * @brief 공격 후 대기 시간 (초). 공격 쿨타임 역할을 합니다.
     * @details 몽타주 길이에 맞춰 설정하는 것을 권장합니다.
     */
    UPROPERTY(EditAnywhere, Category = "KatanaNeon|Boss|Attack",
        meta = (ClampMin = 0.0f, ClampMax = 10.0f))
    float AttackCooldown = 1.5f;
#pragma endregion 에디터 설정 데이터
};
