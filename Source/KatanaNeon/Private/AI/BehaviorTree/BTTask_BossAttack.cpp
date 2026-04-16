// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/BTTask_BossAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

#pragma region 기본 생성자 및 초기화 구현
UBTTask_BossAttack::UBTTask_BossAttack()
{
    NodeName = TEXT("보스 공격 실행");
    bNotifyTick = false;

    // 타입 안전 필터 — 에디터 드롭다운에서 올바른 키 타입만 표시됩니다.
    TargetPlayerKey.AddObjectFilter(
        this,
        GET_MEMBER_NAME_CHECKED(UBTTask_BossAttack, TargetPlayerKey),
        AActor::StaticClass());
    IsAttackingKey.AddBoolFilter(
        this,
        GET_MEMBER_NAME_CHECKED(UBTTask_BossAttack, IsAttackingKey));
}

void UBTTask_BossAttack::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    if (UBlackboardData* BBAsset = GetBlackboardAsset())
    {
        TargetPlayerKey.ResolveSelectedKey(*BBAsset);
        IsAttackingKey.ResolveSelectedKey(*BBAsset);
    }
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region 태스크 오버라이드 구현
EBTNodeResult::Type UBTTask_BossAttack::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    AAIController* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    APawn* Boss = Controller->GetPawn();
    if (!Boss) return EBTNodeResult::Failed;

    // 타겟 확인
    AActor* Target = Cast<AActor>(
        BB->GetValue<UBlackboardKeyType_Object>(TargetPlayerKey.GetSelectedKeyID()));
    if (!Target) return EBTNodeResult::Failed;

    // 공격 전 플레이어 방향으로 회전
    const FVector Direction =
        (Target->GetActorLocation() - Boss->GetActorLocation()).GetSafeNormal2D();
    Boss->SetActorRotation(Direction.Rotation());

    // ASC를 통해 공격 어빌리티 활성화
    UAbilitySystemComponent* ASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Boss);
    if (!ASC || !AttackAbilityTag.IsValid()) return EBTNodeResult::Failed;

    const bool bSuccess =
        ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AttackAbilityTag));

    if (!bSuccess) return EBTNodeResult::Failed;

    // 공격 중 플래그 설정
    BB->SetValue<UBlackboardKeyType_Bool>(IsAttackingKey.GetSelectedKeyID(), true);

    // uint8 대신 FBlackboard::FKey 타입을 사용하여 최신 엔진 문법 준수
    const FBlackboard::FKey KeyID = IsAttackingKey.GetSelectedKeyID();

    FTimerHandle CooldownHandle;

    // [최적화] 보스가 파괴되었을 때 안전하게 콜백을 무시하도록 WeakLambda 사용
    Boss->GetWorldTimerManager().SetTimer(
        CooldownHandle,
        FTimerDelegate::CreateWeakLambda(Boss, [BB, KeyID]()
            {
                // 최적화: 캡처된 블랙보드가 여전히 유효한지 검사
                if (BB != nullptr)
                {
                    BB->SetValue<UBlackboardKeyType_Bool>(KeyID, false);
                }
            }), // 람다 블록이 끝나는 여기에 반드시 콤마(,)가 있어야 합니다!
        AttackCooldown,
        false
    );

    return EBTNodeResult::Succeeded;
}
#pragma endregion 태스크 오버라이드 구현
