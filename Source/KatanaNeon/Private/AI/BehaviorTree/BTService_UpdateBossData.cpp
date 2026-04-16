// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/BTService_UpdateBossData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 기본 생성자 및 초기화 구현
UBTService_UpdateBossData::UBTService_UpdateBossData()
{
    NodeName = TEXT("보스 데이터 갱신");
    // 하드코딩 지양: 틱 간격은 에디터 디테일에서 조정 가능하도록 기본값만 제공합니다.
    Interval = 0.1f;
    RandomDeviation = 0.05f;

    // 타입 안전 필터 — 에디터 드롭다운에서 올바른 키 타입만 표시됩니다.
    TargetPlayerKey.AddObjectFilter(
        this,
        GET_MEMBER_NAME_CHECKED(UBTService_UpdateBossData, TargetPlayerKey),
        AActor::StaticClass());
    DistToPlayerKey.AddFloatFilter(
        this,
        GET_MEMBER_NAME_CHECKED(UBTService_UpdateBossData, DistToPlayerKey));
    IsStunnedKey.AddBoolFilter(
        this,
        GET_MEMBER_NAME_CHECKED(UBTService_UpdateBossData, IsStunnedKey));
}

void UBTService_UpdateBossData::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    // 블랙보드 에셋이 연결되면 셀렉터를 실제 키 인덱스로 해결합니다.
    if (UBlackboardData* BBAsset = GetBlackboardAsset())
    {
        TargetPlayerKey.ResolveSelectedKey(*BBAsset);
        DistToPlayerKey.ResolveSelectedKey(*BBAsset);
        IsStunnedKey.ResolveSelectedKey(*BBAsset);
    }
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region 서비스 오버라이드 구현
void UBTService_UpdateBossData::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    const AAIController* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return;

    const APawn* Boss = Controller->GetPawn();
    if (!Boss) return;

    // 플레이어 거리 갱신
    const AActor* Target = Cast<AActor>(
        BB->GetValue<UBlackboardKeyType_Object>(TargetPlayerKey.GetSelectedKeyID()));

    if (Target)
    {
        const float Dist = FVector::Dist(
            Boss->GetActorLocation(), Target->GetActorLocation());
        BB->SetValue<UBlackboardKeyType_Float>(DistToPlayerKey.GetSelectedKeyID(), Dist);
    }

    // 그로기 태그 확인 → bIsStunned 갱신
    if (const UAbilitySystemComponent* ASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
            const_cast<APawn*>(Boss)))
    {
        const bool bStunned =
            ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::Groggy);
        BB->SetValue<UBlackboardKeyType_Bool>(
            IsStunnedKey.GetSelectedKeyID(), bStunned);
    }
}
#pragma endregion 서비스 오버라이드 구현
