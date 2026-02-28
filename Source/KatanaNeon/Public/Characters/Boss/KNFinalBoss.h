// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Boss/KNBossBase.h"
#include "KNFinalBoss.generated.h"

/**
 * @class  AKNFinalBoss
 * @brief  최종 보스 클래스. 3페이즈 이상의 복합 패턴과 연출을 담습니다.
 */
UCLASS()
class KATANANEON_API AKNFinalBoss : public AKNBossBase
{
	GENERATED_BODY()
	
#pragma region 페이즈 전환 오버라이드
protected:
    /**
     * @brief 최종 보스 전용 페이즈 전환 — 시네마틱 컷신 트리거 포함.
     * @param NewPhaseIndex 전환될 페이즈 인덱스
     */
    virtual void OnPhaseTransition(int32 NewPhaseIndex) override;
#pragma endregion 페이즈 전환 오버라이드 끝

#pragma region 최종 보스 전용 기능
public:
    /**
     * @brief 오버클럭 3단계 궁극기 피격 시 발동되는 특수 연출 트리거입니다.
     * @details 블루프린트에서 레벨 시퀀서를 재생하도록 구현합니다.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "KatanaNeon|Boss|FinalBoss")
    void OnOverclockUltimateHit();
#pragma endregion 최종 보스 전용 기능 끝
};
