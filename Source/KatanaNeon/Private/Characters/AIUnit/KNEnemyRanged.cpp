// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AIUnit/KNEnemyRanged.h"


#pragma region 기본 생성자 및 초기화 구현
AKNEnemyRanged::AKNEnemyRanged()
{
}

void AKNEnemyRanged::BeginPlay()
{
    Super::BeginPlay();

    // 원거리 전용 추가 스탯 로드
    const FKNEnemyRangedStatRow* RangedRow = RangedStatRowHandle.GetRow<FKNEnemyRangedStatRow>(TEXT("EnemyRangedStatInit"));

    // [안전망 추가] 데이터 테이블 행이 할당되지 않았을 때 명확히 경고합니다.
    if (ensureAlwaysMsgf(RangedRow, TEXT("[KNEnemyRanged] %s : RangedStatRowHandle 미할당 또는 행 이름 오류!"), *GetName()))
    {
        CachedRangedStat = *RangedRow;
    }
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region 원거리 공격 구현
void AKNEnemyRanged::FireProjectile(const FVector& TargetLocation)
{
    if (!ProjectileClass)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[KNEnemyRanged] %s : ProjectileClass 미할당."), *GetName());
        return;
    }

    BroadcastAttackWarning(); // 발사 직전 저스트 회피 판정 알림

    // 총구 위치에서 목표 방향으로 발사체 스폰
    const FVector MuzzleLocation = GetMesh()->GetSocketLocation(FName("MuzzleSocket"));
    const FRotator FireRotation =
        (TargetLocation - MuzzleLocation).GetSafeNormal().Rotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    GetWorld()->SpawnActor<AActor>(
        ProjectileClass, MuzzleLocation, FireRotation, SpawnParams);
}
#pragma endregion 원거리 공격 구현