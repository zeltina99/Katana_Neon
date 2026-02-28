// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Boss/KNMidBoss.h"

#pragma region 페이즈 전환 구현
void AKNMidBoss::OnPhaseTransition(int32 NewPhaseIndex)
{
    // 공통 페이즈 처리 (체력 임계값 갱신, 델리게이트 브로드캐스트)
    Super::OnPhaseTransition(NewPhaseIndex);

    // TODO: 페이즈별 전용 패턴 활성화
    // Phase 1 (NewPhaseIndex == 1): 돌진 공격 추가
    // Phase 2 (NewPhaseIndex == 2): 광역 폭발 패턴 추가
    UE_LOG(LogTemp, Log,
        TEXT("[KNMidBoss] 중간 보스 페이즈 %d 패턴 활성화."), NewPhaseIndex);
}
#pragma endregion 페이즈 전환 구현 끝