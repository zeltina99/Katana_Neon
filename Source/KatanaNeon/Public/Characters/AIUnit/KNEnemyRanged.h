// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AIUnit/KNEnemyBase.h"
#include "Engine/DataTable.h"
#include "Data/Structs/KNEnemyStatTable.h"
#include "KNEnemyRanged.generated.h"

#pragma region 전방 선언
class AActor;
#pragma endregion 전방 선언 끝

/**
 * @class  AKNEnemyRanged
 * @brief  원거리 발사체 공격을 수행하는 적 클래스입니다.
 * @details 최소 교전 거리 안으로 플레이어가 접근하면 후퇴하는 AI 패턴을 가집니다.
 */
UCLASS()
class KATANANEON_API AKNEnemyRanged : public AKNEnemyBase
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    AKNEnemyRanged();

protected:
    virtual void BeginPlay() override;
#pragma endregion 기본 생성자 및 초기화 끝

#pragma region 원거리 공격 인터페이스
public:
    /**
     * @brief BT 태스크에서 호출하는 발사체 발사 함수입니다.
     * @param TargetLocation 조준 목표 위치
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Enemy|Combat")
    void FireProjectile(const FVector& TargetLocation);
#pragma endregion 원거리 공격 인터페이스 끝

#pragma region 원거리 전용 데이터
protected:
    /** @brief 원거리 전용 추가 수치 행 핸들 (에디터 할당) */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Enemy|DataTable")
    FDataTableRowHandle RangedStatRowHandle;

    /** @brief 발사할 발사체 액터 클래스 (에디터 할당) */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Enemy|Combat")
    TSubclassOf<AActor> ProjectileClass = nullptr;

private:
    /** @brief 원거리 스탯 런타임 캐시 */
    FKNEnemyRangedStatRow CachedRangedStat;
#pragma endregion 원거리 전용 데이터 끝
};
