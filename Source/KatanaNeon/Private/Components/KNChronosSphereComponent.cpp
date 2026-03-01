// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/KNChronosSphereComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "GAS/Tags/KNStatsTags.h" 

#pragma region 기본 생성자 및 초기화 구현
UKNChronosSphereComponent::UKNChronosSphereComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 최적화: 틱(Tick) 완전 차단

    SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetCollisionObjectType(ECC_WorldDynamic);
    SetCollisionResponseToAllChannels(ECR_Ignore);
    SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    SetGenerateOverlapEvents(false);

    ProjectileTag = KatanaNeon::Data::Actor::Projectile;
}

void UKNChronosSphereComponent::BeginPlay()
{
    Super::BeginPlay();

    OnComponentBeginOverlap.AddDynamic(this, &UKNChronosSphereComponent::OnSphereBeginOverlap);
    OnComponentEndOverlap.AddDynamic(this, &UKNChronosSphereComponent::OnSphereEndOverlap);
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region 외부 제어 인터페이스 구현
void UKNChronosSphereComponent::ActivateSphere(
    float InEnemySlowScale,
    float InProjectileSlowScale,
    float InRadius)
{
    if (bChronosActive) return;

    CachedEnemySlowScale = InEnemySlowScale;
    CachedProjectileSlowScale = InProjectileSlowScale;

    SetSphereRadius(InRadius);
    SetGenerateOverlapEvents(true);
    SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    bChronosActive = true;

    TArray<AActor*> AlreadyOverlapping;
    GetOverlappingActors(AlreadyOverlapping);

    for (AActor* Actor : AlreadyOverlapping)
    {
        if (!Actor || Actor == GetOwner()) continue;

        if (IsProjectileActor(Actor))
        {
            ApplyTimeDilationToActor(Actor, CachedProjectileSlowScale);
            SlowedActors.Add(Actor);
        }
        else if (IsEnemyActor(Actor))
        {
            ApplyTimeDilationToActor(Actor, CachedEnemySlowScale);
            SlowedActors.Add(Actor);
        }
    }
}

void UKNChronosSphereComponent::DeactivateSphere()
{
    if (!bChronosActive) return;

    bChronosActive = false;
    SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetGenerateOverlapEvents(false);

    PurgeInvalidActors();

    for (const TWeakObjectPtr<AActor>& WeakActor : SlowedActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            ApplyTimeDilationToActor(Actor, 1.0f);
        }
    }
    SlowedActors.Empty();

    CachedEnemySlowScale = 1.0f;
    CachedProjectileSlowScale = 1.0f;
}
#pragma endregion 외부 제어 인터페이스 구현

#pragma region 오버랩 콜백 구현
void UKNChronosSphereComponent::OnSphereBeginOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32                OtherBodyIndex,
    bool                 bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == GetOwner() || !bChronosActive) return;

    if (IsProjectileActor(OtherActor))
    {
        ApplyTimeDilationToActor(OtherActor, CachedProjectileSlowScale);
        SlowedActors.Add(OtherActor);
    }
    else if (IsEnemyActor(OtherActor))
    {
        ApplyTimeDilationToActor(OtherActor, CachedEnemySlowScale);
        SlowedActors.Add(OtherActor);
    }
}

void UKNChronosSphereComponent::OnSphereEndOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32                OtherBodyIndex)
{
    if (!OtherActor || !bChronosActive) return;

    const TWeakObjectPtr<AActor> WeakOther(OtherActor);
    if (SlowedActors.Contains(WeakOther))
    {
        ApplyTimeDilationToActor(OtherActor, 1.0f);
        SlowedActors.Remove(WeakOther);
    }
}
#pragma endregion 오버랩 콜백 구현

#pragma region 내부 헬퍼 함수 구현
bool UKNChronosSphereComponent::IsProjectileActor(const AActor* Actor) const
{
    if (!Actor || !ProjectileTag.IsValid()) return false;

    // 베테랑 최적화: IAbilitySystemInterface 캐스팅 (O(1)) 사용
    if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor))
    {
        if (const UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
        {
            return ASC->HasMatchingGameplayTag(ProjectileTag);
        }
    }

    // 베테랑 최적화: FName 파싱 비용 제거를 위한 static 캐싱
    static const FName NAME_KNProjectile(TEXT("KNProjectile"));
    return Actor->ActorHasTag(NAME_KNProjectile);
}

bool UKNChronosSphereComponent::IsEnemyActor(const AActor* Actor) const
{
    if (!Actor || Actor == GetOwner()) return false;

    // 베테랑 최적화: 컴포넌트 전체를 뒤지는 FindComponentByClass(O(N)) 제거.
    // ASC 인터페이스를 상속받은 캐릭터인지 즉시 캐스팅(O(1))하여 판별합니다.
    return Cast<IAbilitySystemInterface>(Actor) != nullptr;
}

/*static*/ void UKNChronosSphereComponent::ApplyTimeDilationToActor(AActor* Actor, float Scale)
{
    if (!Actor) return;
    Actor->CustomTimeDilation = Scale;
}

void UKNChronosSphereComponent::PurgeInvalidActors()
{
    // 베테랑 최적화: TSet은 RemoveAll을 지원하지 않으므로 Iterator로 순회 삭제합니다.
    for (auto It = SlowedActors.CreateIterator(); It; ++It)
    {
        if (!It->IsValid())
        {
            It.RemoveCurrent();
        }
    }
}
#pragma endregion 내부 헬퍼 함수 구현
