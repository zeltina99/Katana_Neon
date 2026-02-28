// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Boss/KNFinalBoss.h"

#pragma region 페이즈 전환 구현
void AKNFinalBoss::OnPhaseTransition(int32 NewPhaseIndex)
{
    Super::OnPhaseTransition(NewPhaseIndex);

    // TODO: 페이즈별 컷신/시퀀서 연동
    // Phase 1 (NewPhaseIndex == 1): 폼 변경 연출
    // Phase 2 (NewPhaseIndex == 2): 맵 변화 + 오버클럭 게이지 강제 충전
    // Phase 3 (NewPhaseIndex == 3): 절망적 광역 공격 (사망 직전 발악)

    UE_LOG(LogTemp, Log, TEXT("[KNFinalBoss] 최종 보스 페이즈 %d 돌입."), NewPhaseIndex);
}
#pragma endregion 페이즈 전환 구현 끝