// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "KNSlashProjectile.generated.h"

#pragma region 전방 선언
class UBoxComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UAbilitySystemComponent;
class UGameplayEffect;
#pragma endregion 전방 선언

/**
 * @file    KNSlashProjectile.h
 * @class   AKNSlashProjectile
 * @brief   오버클럭 2단계 어빌리티가 발사하는 참격파(Slash Wave) 발사체 액터입니다.
 *
 * @details
 * [SRP 책임]
 * - 이 클래스는 오직 "발사체 이동 · 충돌 · 피격 처리"만 담당합니다.
 * - 데미지 GE 적용, 그로기 태그 부여도 이 클래스 내에서 직접 처리합니다.
 *
 * [베테랑의 최적화 적용]
 * 1. Tick 함수 완전 제거: 매 프레임 거리를 계산하는 대신, 초기화 시 (최대 거리 / 속력)으로
 * Lifespan을 계산하여 엔진이 비용 0으로 자동 소멸시키도록 설계했습니다.
 * 2. O(1) 피격 판정: FindComponentByClass를 배제하고 IAbilitySystemInterface를 통해
 * 적의 ASC를 즉시 가져옵니다.
 */
UCLASS()
class KATANANEON_API AKNSlashProjectile : public AActor
{
    GENERATED_BODY()

#pragma region 기본 생성자 및 초기화
public:
    /**
     * @brief 발사체 기본 컴포넌트 구성.
     * @details Tick을 비활성화하고 BoxComponent, ProjectileMovement, Niagara를 세팅합니다.
     */
    AKNSlashProjectile();

protected:
    virtual void BeginPlay() override;
#pragma endregion 기본 생성자 및 초기화

#pragma region 발사 초기화 인터페이스
public:
    /**
     * @brief 발사체를 초기화합니다. 지연 스폰(Deferred Spawn) 직후 수동으로 호출됩니다.
     * @param InInstigatorASC  발사한 플레이어의 AbilitySystemComponent
     * @param InDamageGEClass  피격 시 적용할 데미지 Instant GE 클래스
     * @param InGrogyGEClass   피격 시 적용할 그로기 Duration GE 클래스
     * @param InRow            DT_OverclockLv2Setting 에서 로드한 수치 행
     */
    void InitProjectile(
        UAbilitySystemComponent* InInstigatorASC,
        TSubclassOf<UGameplayEffect>  InDamageGEClass,
        TSubclassOf<UGameplayEffect>  InGrogyGEClass,
        const FKNOverclockLv2Row& InRow);
#pragma endregion 발사 초기화 인터페이스

#pragma region 에디터 설정 데이터
protected:
    /** @brief 발사체 Box Collision — 충돌 판정 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|Slash|Components")
    TObjectPtr<UBoxComponent> CollisionBox = nullptr;

    /** @brief 자동 이동 처리 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|Slash|Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

    /** @brief 참격파 시각 이펙트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|Slash|Components")
    TObjectPtr<UNiagaraComponent> SlashFX = nullptr;
#pragma endregion 에디터 설정 데이터

#pragma region 런타임 상태
private:
    /** @brief 발사한 플레이어의 ASC — 데미지 GE 적용 출처로 사용 */
    TWeakObjectPtr<UAbilitySystemComponent> InstigatorASC = nullptr;

    /** @brief 피격 즉시 적용할 데미지 Instant GE 클래스 */
    TSubclassOf<UGameplayEffect> DamageGEClass = nullptr;

    /** @brief 피격 대상에 그로기 Duration GE 클래스 */
    TSubclassOf<UGameplayEffect> GrogyGEClass = nullptr;

    /** @brief 현재 발사체에 적용된 수치 행 캐시 */
    FKNOverclockLv2Row CachedRow;

    /** @brief 이미 피격 처리를 수행했는지 여부 (다단 히트 방지) */
    bool bHitProcessed = false;
#pragma endregion 런타임 상태

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief Box Collision OnHit 이벤트 콜백.
     */
    UFUNCTION()
    void OnCollisionHit(
        UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector              NormalImpulse,
        const FHitResult& Hit);

    /**
     * @brief 피격 처리 공통 로직 — 데미지 GE + 그로기 태그를 대상에게 적용합니다.
     * @param TargetActor 피격 대상 Actor
     */
    void ProcessHit(AActor* TargetActor);
#pragma endregion 내부 헬퍼 함수
};
