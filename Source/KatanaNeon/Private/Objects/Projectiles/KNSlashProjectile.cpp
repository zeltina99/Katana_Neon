// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Projectiles/KNSlashProjectile.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h" // 베테랑 최적화: O(1) ASC 캐스팅용
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "GAS/Tags/KNStatsTags.h" 

#pragma region 기본 생성자 및 초기화 구현
AKNSlashProjectile::AKNSlashProjectile()
{
    // ── 베테랑 최적화 1: Tick 완전 비활성화 (CPU 낭비 제로) ──
    PrimaryActorTick.bCanEverTick = false;

    // ── Box 충돌 컴포넌트 ──
    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    SetRootComponent(CollisionBox);
    CollisionBox->SetCollisionProfileName(TEXT("Projectile"));
    CollisionBox->SetGenerateOverlapEvents(false); // Overlap 대신 정확한 Hit 물리 이벤트 사용
    CollisionBox->SetNotifyRigidBodyCollision(true);

    // ── 이동 컴포넌트 ──
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f; // 참격파는 일직선으로 날아감

    // ── 비주얼 이펙트 ──
    SlashFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SlashFX"));
    SlashFX->SetupAttachment(RootComponent);
    SlashFX->bAutoActivate = true;
}

void AKNSlashProjectile::BeginPlay()
{
    Super::BeginPlay();

    // Hit 콜백 등록
    CollisionBox->OnComponentHit.AddDynamic(this, &AKNSlashProjectile::OnCollisionHit);
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region 발사 초기화 인터페이스 구현
void AKNSlashProjectile::InitProjectile(
    UAbilitySystemComponent* InInstigatorASC,
    TSubclassOf<UGameplayEffect>  InDamageGEClass,
    TSubclassOf<UGameplayEffect>  InGrogyGEClass,
    const FKNOverclockLv2Row& InRow)
{
    // TWeakObjectPtr로 안전하게 보관하여 플레이어가 도중 사망해도 크래시 안 남
    InstigatorASC = InInstigatorASC;
    DamageGEClass = InDamageGEClass;
    GrogyGEClass = InGrogyGEClass;
    CachedRow = InRow;

    // Box 크기 및 이동 속도를 기획자 데이터(DataTable)로 초기화
    CollisionBox->SetBoxExtent(InRow.SlashBoxHalfExtent);
    ProjectileMovement->InitialSpeed = InRow.SlashSpeed;
    ProjectileMovement->MaxSpeed = InRow.SlashSpeed;

    // ── 베테랑 최적화 2: Tick 거리 검사를 대체하는 엔진 네이티브 수명(LifeSpan) 시스템 ──
    // 시간 = 거리 / 속력
    if (InRow.SlashSpeed > 0.0f)
    {
        const float CalculatedLifeTime = InRow.SlashMaxDistance / InRow.SlashSpeed;
        SetLifeSpan(CalculatedLifeTime); // 지정된 시간 뒤에 엔진이 비용 0으로 자동 Destroy 처리
    }
}
#pragma endregion 발사 초기화 인터페이스 구현

#pragma region 내부 헬퍼 함수 구현
void AKNSlashProjectile::OnCollisionHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector              NormalImpulse,
    const FHitResult& Hit)
{
    // 본인(발사자)이거나 이미 맞춘 적이면 무시
    if (!OtherActor || OtherActor == GetInstigator() || bHitProcessed) return;

    ProcessHit(OtherActor);
}

void AKNSlashProjectile::ProcessHit(AActor* TargetActor)
{
    if (!TargetActor || !InstigatorASC.IsValid() || bHitProcessed) return;

    UAbilitySystemComponent* TargetASC = nullptr;

    // ── 베테랑 최적화 3: 컴포넌트 순회 탐색(O(N))을 버리고 인터페이스 캐스팅(O(1)) 사용 ──
    if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor))
    {
        TargetASC = ASI->GetAbilitySystemComponent();
    }

    // ASC가 없는 배경 프랍(벽, 바닥 등)에 맞은 경우: 이펙트만 터뜨리고 바로 소멸
    if (!TargetASC)
    {
        Destroy();
        return;
    }

    bHitProcessed = true; // 중복 피격 완벽 방지

    // ── 1. 데미지 GE 적용 ──
    if (DamageGEClass)
    {
        FGameplayEffectContextHandle Context = InstigatorASC->MakeEffectContext();
        Context.AddInstigator(GetInstigator(), this);

        FGameplayEffectSpecHandle DmgSpec = InstigatorASC->MakeOutgoingSpec(DamageGEClass, 1.0f, Context);
        if (FGameplayEffectSpec* Spec = DmgSpec.Data.Get())
        {
            Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Health, -CachedRow.SlashDamage);
            TargetASC->ApplyGameplayEffectSpecToSelf(*Spec);
        }
    }

    // ── 2. 그로기 Duration GE 적용 ──
    if (GrogyGEClass)
    {
        FGameplayEffectContextHandle Context = InstigatorASC->MakeEffectContext();
        FGameplayEffectSpecHandle GrogySpec = InstigatorASC->MakeOutgoingSpec(GrogyGEClass, 1.0f, Context);
        if (FGameplayEffectSpec* Spec = GrogySpec.Data.Get())
        {
            // 그로기 지속 시간: 엑셀 데이터로 GE의 Duration을 덮어씁니다.
            Spec->SetDuration(CachedRow.GrogyDuration, true);
            TargetASC->ApplyGameplayEffectSpecToSelf(*Spec);
        }
    }

    // 피격 후 발사체 소멸
    Destroy();
}
#pragma endregion 내부 헬퍼 함수 구현

