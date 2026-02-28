// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Boss/KNBossBase.h"
#include "KNMidBoss.generated.h"

/**
 * @class  AKNMidBoss
 * @brief  중간 보스 클래스. KNBossBase의 2페이즈 전환 시스템을 기반으로 합니다.
 * @details 1주차 프로토타입에서는 페이즈 전환 시 공격 패턴 변화를 블루프린트로 구현합니다.
 */
UCLASS()
class KATANANEON_API AKNMidBoss : public AKNBossBase
{
	GENERATED_BODY()
	
#pragma region 페이즈 전환 오버라이드
protected:
    /**
     * @brief 중간 보스 전용 페이즈 전환 연출 및 패턴 변화를 처리합니다.
     * @param NewPhaseIndex 전환될 페이즈 인덱스
     */
    virtual void OnPhaseTransition(int32 NewPhaseIndex) override;
#pragma endregion 페이즈 전환 오버라이드 끝
};
