// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AIUnit/KNEnemyBase.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameplayEffect.h"
#include "GAS/Tags/KNStatsTags.h"


#pragma region 기본 생성자 및 초기화 구현
AKNEnemyBase::AKNEnemyBase()
{
    // 육체는 스스로 감지하거나 생각하지 않으므로, Perception 관련 코드가 모두 제거되었습니다.
}

void AKNEnemyBase::BeginPlay()
{
    // 부모: InitAbilityActorInfo + GiveDefaultAbilities
    Super::BeginPlay();

    // DataTable 스탯 로드 및 GE로 어트리뷰트 초기화
    ApplyEnemyBaseStats();

    // BT 실행 로직은 AKNEnemyController::OnPossess 로 완전히 위임되었습니다.
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region 공격 예고 시스템 구현
void AKNEnemyBase::BroadcastAttackWarning()
{
    // 캐시된 스탯에서 판정 윈도우 시간을 꺼내 브로드캐스트합니다.
    OnAttackWarning.Broadcast(CachedEnemyStat.AttackWarningDuration);
}
#pragma endregion 공격 예고 시스템 구현

#pragma region 사망 처리 구현
void AKNEnemyBase::Die()
{
    // AI 컨트롤러 행동 정지
    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        AIC->StopMovement();
        AIC->ClearFocus(EAIFocusPriority::Gameplay);
    }

    // 물리 Ragdoll 전환
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // 부모 Die: 캡슐 콜리전 끄기 + OnCharacterDeath 브로드캐스트
    Super::Die();

    // 일정 시간 후 액터 제거
    SetLifeSpan(5.0f);
}
#pragma endregion 사망 처리 구현

#pragma region 스탯 초기화 구현
void AKNEnemyBase::ApplyEnemyBaseStats()
{
    const FKNEnemyBaseStatRow* StatRow = EnemyStatRowHandle.GetRow<FKNEnemyBaseStatRow>(TEXT("EnemyBaseStatInit"));

    // [최적화 & 안전망] 데이터가 없으면 즉시 에디터에 경고를 띄웁니다.
    if (!ensureAlwaysMsgf(StatRow, TEXT("[KNEnemyBase] %s : EnemyStatRowHandle 미할당 또는 행이 없습니다!"), *GetName()))
    {
        return;
    }

    // 런타임 캐시 저장
    CachedEnemyStat = *StatRow;

    if (!AbilitySystemComponent) return;

    // [최적화 & 안전망] 기획자가 블루프린트에서 GE 클래스를 안 넣었을 경우 방어합니다.
    if (!ensureAlwaysMsgf(InitStatGEClass, TEXT("[KNEnemyBase] %s : InitStatGEClass가 에디터에 할당되지 않았습니다!"), *GetName()))
    {
        return;
    }

    FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitStatGEClass, 1.0f, Context);

    if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
    {
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::MaxHealth, StatRow->MaxHealth);
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Health, StatRow->MaxHealth);
        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec);
    }
}
#pragma endregion 스탯 초기화 구현