// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Boss/KNBossBase.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/KNAttributeSet.h"
#include "TimerManager.h"

#pragma region 기본 생성자 및 초기화 구현
AKNBossBase::AKNBossBase()
{
}

void AKNBossBase::BeginPlay()
{
    Super::BeginPlay();

    //페이즈 DataTable 로드 및 안전망 적용
    const FKNBossPhaseRow* PhaseRow = BossPhaseRowHandle.GetRow<FKNBossPhaseRow>(TEXT("BossPhaseInit"));
    if (ensureAlwaysMsgf(PhaseRow, TEXT("[KNBossBase] %s : BossPhaseRowHandle 미할당 또는 데이터가 없습니다!"), *GetName()))
    {
        CachedPhaseData = *PhaseRow;
    }

    // 체력 변경 시 페이즈 전환 자동 체크 등록
    if (AbilitySystemComponent && AttributeSet)
    {
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSet->GetHealthAttribute())
            .AddLambda([this](const FOnAttributeChangeData&)
                {
                    CheckPhaseTransition();
                });
    }
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region 페이즈 시스템 구현
void AKNBossBase::CheckPhaseTransition()
{
    if (bIsTransitioning || !AttributeSet) return;
    if (CachedPhaseData.PhaseHealthThresholds.IsEmpty()) return;

    const float MaxHP = AttributeSet->GetMaxHealth();
    const float CurrentHP = AttributeSet->GetHealth();
    if (MaxHP <= 0.0f) return;

    const float HealthRatio = CurrentHP / MaxHP;

    // 다음 페이즈 임계값과 비교 (내림차순 배열 순회)
    const int32 NextPhase = CurrentPhaseIndex + 1;
    if (!CachedPhaseData.PhaseHealthThresholds.IsValidIndex(CurrentPhaseIndex)) return;

    if (HealthRatio <= CachedPhaseData.PhaseHealthThresholds[CurrentPhaseIndex])
    {
        OnPhaseTransition(NextPhase);
    }
}

void AKNBossBase::Die()
{
    // 보스가 죽을 때 잊지 않고 타이머를 강제 해제합니다.
    GetWorldTimerManager().ClearTimer(TransitionHandle);
    bIsTransitioning = false;

    Super::Die();
}

void AKNBossBase::OnPhaseTransition(int32 NewPhaseIndex)
{
    bIsTransitioning = true;
    CurrentPhaseIndex = NewPhaseIndex;

    OnPhaseChanged.Broadcast(CurrentPhaseIndex);

    UE_LOG(LogTemp, Log, TEXT("[KNBossBase] %s : Phase %d 전환됨."), *GetName(), CurrentPhaseIndex);

    // 로컬 변수 대신 멤버 핸들을 사용하여 추적 가능하게 만듭니다.
    GetWorldTimerManager().SetTimer(
        TransitionHandle,
        this,
        &AKNBossBase::UnlockPhaseTransition,
        CachedPhaseData.TransitionDuration,
        false
    );
}

void AKNBossBase::UnlockPhaseTransition()
{
    bIsTransitioning = false;
}
#pragma endregion 페이즈 시스템 구현
