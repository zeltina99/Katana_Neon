// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/KNBossController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Characters/Boss/KNBossBase.h"

#pragma region 블랙보드 키 이름 상수 정의
const FName AKNBossController::BBKey_TargetPlayer = TEXT("TargetPlayer");
const FName AKNBossController::BBKey_IsAttacking = TEXT("bIsAttacking");
const FName AKNBossController::BBKey_CurrentPhase = TEXT("CurrentPhase");
const FName AKNBossController::BBKey_IsStunned = TEXT("bIsStunned");
const FName AKNBossController::BBKey_DistToPlayer = TEXT("DistToPlayer");
#pragma endregion 블랙보드 키 이름 상수 정의

#pragma region 기본 생성자 및 초기화 구현
AKNBossController::AKNBossController()
{
    // Perception 컴포넌트 생성
    SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent")));

    // 시야 감지 설정
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;  // 감지 반경
    SightConfig->LoseSightRadius = 2500.0f;  // 감지 해제 반경
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    GetPerceptionComponent()->ConfigureSense(*SightConfig);
    GetPerceptionComponent()->SetDominantSense(SightConfig->GetSenseImplementation());
    GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(
        this, &AKNBossController::OnPerceptionUpdated);
}

void AKNBossController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AKNBossBase* Boss = Cast<AKNBossBase>(InPawn);
    if (!Boss) return;

    UBehaviorTree* BT = Boss->GetBehaviorTree();
    if (!BT) return;

    // 블랙보드 초기화 후 비헤이비어 트리 실행
    UBlackboardComponent* BB = nullptr;
    if (UseBlackboard(BT->BlackboardAsset, BB))
    {
        BB->SetValueAsInt(BBKey_CurrentPhase, 0);
        BB->SetValueAsBool(BBKey_IsAttacking, false);
        BB->SetValueAsBool(BBKey_IsStunned, false);
        BB->SetValueAsFloat(BBKey_DistToPlayer, 0.0f);
    }

    RunBehaviorTree(BT);

    // 보스 페이즈 전환 시 블랙보드 자동 갱신
    Boss->OnPhaseChanged.AddDynamic(this, &AKNBossController::SetCurrentPhase);
}

void AKNBossController::OnUnPossess()
{
    StopMovement();
    Super::OnUnPossess();
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region 블랙보드 갱신 인터페이스 구현
void AKNBossController::SetTargetPlayer(AActor* NewTarget)
{
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsObject(BBKey_TargetPlayer, NewTarget);
    }
}

void AKNBossController::SetCurrentPhase(int32 NewPhase)
{
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsInt(BBKey_CurrentPhase, NewPhase);
    }
}

void AKNBossController::SetIsStunned(bool bStunned)
{
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsBool(BBKey_IsStunned, bStunned);
    }
}
#pragma endregion 블랙보드 갱신 인터페이스 구현

#pragma region Perception 콜백 구현
void AKNBossController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        FActorPerceptionBlueprintInfo Info;
        GetPerceptionComponent()->GetActorsPerception(Actor, Info);

        // 가장 최근 시야 자극 확인
        for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
        {
            if (Stimulus.WasSuccessfullySensed())
            {
                // 플레이어를 발견했으면 타겟으로 설정
                SetTargetPlayer(Actor);
            }
            else
            {
                // 시야를 잃었으면 타겟 해제
                if (UBlackboardComponent* BB = GetBlackboardComponent())
                {
                    if (BB->GetValueAsObject(BBKey_TargetPlayer) == Actor)
                    {
                        SetTargetPlayer(nullptr);
                    }
                }
            }
        }
    }
}
#pragma endregion Perception 콜백 구현
