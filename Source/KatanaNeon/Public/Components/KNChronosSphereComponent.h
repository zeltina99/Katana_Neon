// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameplayTagContainer.h"
#include "KNChronosSphereComponent.generated.h"

#pragma region 전방 선언
class UAbilitySystemComponent;
class IAbilitySystemInterface;
#pragma endregion 전방 선언

/**
 * @file    KNChronosSphereComponent.h
 * @class   UKNChronosSphereComponent
 * @brief   플레이어에 부착되어 구체 범위 내 적과 적 발사체의 시간을 감속시키는 컴포넌트입니다.
 *
 * @details
 * [SRP 책임]
 * - "누가 구체 안에 있는가" 감지 및 CustomTimeDilation 적용/복구만 담당합니다.
 * - 크로노스 게이지 소모(GE), 어빌리티 생애 주기는 KNAbility_Chronos에서 관리합니다.
 *
 * [최적화 적용]
 * - FindComponentByClass(O(N)) 연산을 배제하고 IAbilitySystemInterface(O(1))로 대상을 식별합니다.
 * - 오버랩 이벤트는 어빌리티가 활성화(ActivateSphere)될 때만 켜져 평시 틱 비용을 0으로 만듭니다.
 */
UCLASS(ClassGroup = (KatanaNeon), meta = (BlueprintSpawnableComponent))
class KATANANEON_API UKNChronosSphereComponent : public USphereComponent
{
    GENERATED_BODY()

#pragma region 기본 생성자 및 초기화
public:
    /**
     * @brief 컴포넌트 기본값 초기화.
     * @details 기본적으로 비활성(콜리전 Disabled) 상태로 생성됩니다.
     */
    UKNChronosSphereComponent();

protected:
    virtual void BeginPlay() override;

    /**
     * @brief 크로노스 활성화 중에만 틱이 켜집니다.
     *        디버그 드로우 전용이므로 Shipping 빌드에서는 완전 제거됩니다.
     */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;
#pragma endregion 기본 생성자 및 초기화

#pragma region 외부 제어 인터페이스
public:
    /**
     * @brief 크로노스 구체를 활성화하고 현재 구체 내부 액터를 즉시 감속합니다.
     * @param InEnemySlowScale       적 캐릭터에 적용할 CustomTimeDilation 배율
     * @param InProjectileSlowScale  적 발사체에 적용할 CustomTimeDilation 배율
     * @param InRadius               구체 반경 (cm)
     */
    void ActivateSphere(float InEnemySlowScale, float InProjectileSlowScale, float InRadius);

    /**
     * @brief 크로노스 구체를 비활성화하고 감속 중인 모든 액터를 즉시 복구합니다.
     */
    void DeactivateSphere();

    /**
     * @brief 현재 구체가 활성화 상태인지 반환합니다.
     * @return 활성화 중이면 true
     */
    FORCEINLINE bool IsChronosActive() const { return bChronosActive; }
#pragma endregion 외부 제어 인터페이스

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 발사체 Actor를 식별하는 GameplayTag.
     * @details KatanaNeon.Data.Actor.Projectile 태그를 기본값으로 사용합니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Chronos|Config")
    FGameplayTag ProjectileTag;
#pragma endregion 에디터 설정 데이터

#pragma region 런타임 상태
private:
    bool bChronosActive = false;
    float CachedEnemySlowScale = 1.0f;
    float CachedProjectileSlowScale = 1.0f;

    /**
     * @brief 현재 구체 내에서 감속 중인 Actor 집합.
     * @details TWeakObjectPtr 사용 — 파괴된 액터 참조 시 발생하는 크래시(Dangling Pointer) 방지.
     */
    TSet<TWeakObjectPtr<AActor>> SlowedActors;
#pragma endregion 런타임 상태

#pragma region 오버랩 콜백
private:
    UFUNCTION()
    void OnSphereBeginOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32                OtherBodyIndex,
        bool                 bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32                OtherBodyIndex);
#pragma endregion 오버랩 콜백

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief 대상 Actor가 적 발사체인지 확인합니다. (최적화 적용)
     */
    bool IsProjectileActor(const AActor* Actor) const;

    /**
     * @brief 대상 Actor가 적 캐릭터인지 확인합니다. (최적화 적용)
     */
    bool IsEnemyActor(const AActor* Actor) const;

    /**
     * @brief 대상 Actor의 CustomTimeDilation을 설정합니다.
     */
    static void ApplyTimeDilationToActor(AActor* Actor, float Scale);

    /**
     * @brief SlowedActors에서 이미 파괴된 약참조를 정리합니다.
     */
    void PurgeInvalidActors();
#pragma endregion 내부 헬퍼 함수
};
