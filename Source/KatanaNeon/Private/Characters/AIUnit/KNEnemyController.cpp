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
        if (UBehaviorTree* BT = EnemyPawn->GetBehaviorTree())
        {
            // 언리얼 5의 TObjectPtr 호환성을 위해 로컬 원시 포인터를 거쳐서 넘겨줍니다.
            UBlackboardComponent* RawBlackboard = Blackboard.Get();

            // UseBlackboard가 성공적으로 초기화되면 RawBlackboard에 새 컴포넌트를 할당해 줍니다.
            if (UseBlackboard(BT->BlackboardAsset, RawBlackboard))
            {
                // 초기화된 원시 포인터를 다시 컨트롤러의 TObjectPtr 멤버 변수에 안전하게 담아줍니다.
                Blackboard = RawBlackboard;

                // 트리를 실행합니다.
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
        // 🔥 하드코딩이 아닌, 에디터에서 세팅한 TargetKeyName 변수를 사용합니다.
        if (Stimulus.WasSuccessfullySensed())
        {
            Blackboard->SetValueAsObject(TargetKeyName, Actor);
        }
        else
        {
            Blackboard->SetValueAsObject(TargetKeyName, nullptr);
        }
    }
}
#pragma endregion AI 감지 로직 구현