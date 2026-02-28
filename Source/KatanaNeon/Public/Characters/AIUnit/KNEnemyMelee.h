// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AIUnit/KNEnemyBase.h"
#include "KNEnemyMelee.generated.h"

/**
 * @class  AKNEnemyMelee
 * @brief  근접 공격 전용 일반 적 클래스입니다.
 * @details 돌진 공격, 근접 히트박스를 DataTable 수치로 운용합니다.
 */
UCLASS()
class KATANANEON_API AKNEnemyMelee : public AKNEnemyBase
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    AKNEnemyMelee();

protected:
    virtual void BeginPlay() override;
#pragma endregion 기본 생성자 및 초기화 끝

#pragma region 근접 공격 인터페이스
public:
    /**
     * @brief BT 태스크에서 호출하는 근접 공격 실행 함수입니다.
     * @details 공격 예고 브로드캐스트 → 몽타주 재생 → 히트박스 활성화 순으로 진행됩니다.
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Enemy|Combat")
    void PerformMeleeAttack();

    /**
     * @brief 히트박스가 활성화된 순간 겹치는 플레이어에게 데미지를 적용합니다.
     * @details 애님 노티파이(AnimNotify)에서 호출됩니다.
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Enemy|Combat")
    void ActivateMeleeHitbox();
#pragma endregion 근접 공격 인터페이스

#pragma region 돌진 공격 인터페이스
public:
    /**
     * @brief 플레이어를 향해 직선으로 돌진하는 공격을 시작합니다.
     * @param TargetLocation 돌진 목표 지점
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Enemy|Combat")
    void PerformChargeAttack(const FVector& TargetLocation);
#pragma endregion 돌진 공격 인터페이스

#pragma region 런타임 전투 상태
private:
    /** @brief 현재 돌진 중 여부 — 히트박스 중복 발동 방지용 */
    bool bIsCharging = false;

    /** @brief 돌진 종료 타이머 핸들 (크래시 방지용 멤버 변수) */
    FTimerHandle ChargeEndHandle;

    /** @brief 돌진 상태를 해제하는 콜백 함수입니다. */
    void OnChargeEnd();
#pragma endregion 런타임 전투 상태
};
