// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Boss/KNFinalBoss.h"

#pragma region 페이즈 전환 구현
void AKNFinalBoss::OnPhaseTransition(int32 NewPhaseIndex)
{
    Super::OnPhaseTransition(NewPhaseIndex);

    // [기획 확정] 2페이즈 구조
    // Phase 1 (NewPhaseIndex == 1): 카타나를 활용한 기본 근접 공격 패턴
    // Phase 2 (NewPhaseIndex == 2): 돌진 공격 및 광역기(AoE) 특수 패턴 해금

    // TODO: 2페이즈 진입 시 재생할 시네마틱 컷신 또는 맵 변화 연출 트리거

    UE_LOG(LogTemp, Log, TEXT("[KNFinalBoss] 최종 보스 페이즈 %d 돌입."), NewPhaseIndex);
}
#pragma endregion 페이즈 전환 구현