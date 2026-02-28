// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AIUnit/KNEnemyController.h"
#include "Characters/AIUnit/KNEnemyBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#pragma region 기본 생성자 및 초기화 구현
AKNEnemyController::AKNEnemyController()
{
    // Perception 컴포넌트 및 시각 설정 생성
    EnemyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyPerceptionComponent"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    if (SightConfig)
    {
        // 기본값 세팅 (추후 DataTable 수치로 오버라이드 가능)
        SightConfig->SightRadius = 1500.0f;
        SightConfig->LoseSightRadius = 1800.0f;
        SightConfig->PeripheralVisionAngleDegrees = 80.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

        EnemyPerceptionComponent->ConfigureSense(*SightConfig);
        EnemyPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    // 감지 이벤트 바인딩
    EnemyPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AKNEnemyController::OnTargetDetected);
}

void AKNEnemyController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 빙의한 육체(EnemyBase)로부터 구동할 비헤이비어 트리(BT)를 받아와 실행합니다.
    if (AKNEnemyBase* EnemyPawn = Cast<AKNEnemyBase>(InPawn))
    {
        //  빙의 시점에 DataTable 값으로 덮어씌웁니다!
        if (SightConfig && EnemyPerceptionComponent)
        {
            SightConfig->SightRadius = EnemyPawn->GetCachedStat().SightRadius;
            SightConfig->LoseSightRadius = EnemyPawn->GetCachedStat().SightRadius * 1.2f;

            EnemyPerceptionComponent->ConfigureSense(*SightConfig);
        }

        if (UBehaviorTree* BT = EnemyPawn->GetBehaviorTree())
        {
            UBlackboardComponent* RawBlackboard = Blackboard.Get();
            if (UseBlackboard(BT->BlackboardAsset, RawBlackboard))
            {
                Blackboard = RawBlackboard;
                RunBehaviorTree(BT);
            }
        }
    }
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region AI 감지 로직 구현
void AKNEnemyController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (Blackboard)
    {
        // FName 대신 완벽하게 안전한 SelectedKeyName을 사용합니다.
        if (Stimulus.WasSuccessfullySensed())
        {
            Blackboard->SetValueAsObject(TargetActorKey.SelectedKeyName, Actor);
        }
        else
        {
            Blackboard->SetValueAsObject(TargetActorKey.SelectedKeyName, nullptr);
        }
    }
}
#pragma endregion AI 감지 로직 구현