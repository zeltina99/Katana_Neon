// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AIUnit/KNEnemyMelee.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Data/Structs/KNEnemyStatTable.h"

#pragma region 기본 생성자 및 초기화 구현
AKNEnemyMelee::AKNEnemyMelee()
{
    // 근접 적은 빠른 회전으로 플레이어를 즉시 바라봅니다.
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
    }
}

void AKNEnemyMelee::BeginPlay()
{
    Super::BeginPlay();

    // 이동 속도를 DataTable 캐시 값으로 설정합니다.
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = CachedEnemyStat.MoveSpeed;
    }
}
#pragma endregion 기본 생성자 및 초기화 구현 끝

#pragma region 근접 공격 구현
void AKNEnemyMelee::PerformMeleeAttack()
{
    // 1. 저스트 회피 판정 윈도우를 플레이어에게 알림
    BroadcastAttackWarning();

    // 2. 공격 몽타주 재생 (블루프린트에서 오버라이드하여 실제 몽타주를 지정하세요)
    // PlayAnimMontage(MeleeAttackMontage);
}

void AKNEnemyMelee::ActivateMeleeHitbox()
{
    if (!AbilitySystemComponent) return;

    // 공격 사거리 내의 플레이어를 구체 오버랩으로 감지합니다.
    TArray<FOverlapResult> Overlaps;
    const FCollisionShape AttackSphere =
        FCollisionShape::MakeSphere(CachedEnemyStat.AttackRange);

    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation() + GetActorForwardVector() * (CachedEnemyStat.AttackRange * 0.5f),
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        AttackSphere);

    for (const FOverlapResult& Overlap : Overlaps)
    {
        if (Overlap.GetActor() == this) continue;

        if (UAbilitySystemComponent* TargetASC =
            Overlap.GetActor()->FindComponentByClass<UAbilitySystemComponent>())
        {
            // TODO: 데미지 GE 클래스를 에디터에서 할당 후 SetByCaller로 전달
            // FGameplayEffectSpecHandle DmgSpec = ...
            // TargetASC->ApplyGameplayEffectSpecToTarget(...);
        }
    }
}
#pragma endregion 근접 공격 구현 끝

#pragma region 돌진 공격 구현
void AKNEnemyMelee::PerformChargeAttack(const FVector& TargetLocation)
{
    if (bIsCharging) return;

    bIsCharging = true;
    BroadcastAttackWarning(); // 돌진도 저스트 회피 판정 대상

    const FVector ChargeDirection =
        (TargetLocation - GetActorLocation()).GetSafeNormal();

    // 돌진 속도 = 기본 이동속도 * 2.5배
    LaunchCharacter(ChargeDirection * CachedEnemyStat.MoveSpeed * 2.5f,
        /*bXYOverride=*/true, /*bZOverride=*/false);

    // 일정 시간 후 돌진 상태 해제
    FTimerHandle ChargeEndHandle;
    GetWorldTimerManager().SetTimer(ChargeEndHandle, [this]()
        {
            bIsCharging = false;
        }, 0.5f, false);
}
#pragma endregion 돌진 공격 구현 끝