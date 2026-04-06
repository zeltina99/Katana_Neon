// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/KNDynamicIconWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

#pragma region 위젯 생명주기 오버라이드 구현
void UKNDynamicIconWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기 상태는 LV0 (포인트 0) 으로 시작합니다.
    ApplyMaterialForStage(0);
    CachedStageIndex = 0;
}
#pragma endregion 위젯 생명주기 오버라이드 구현

#pragma region 외부 제어 인터페이스 구현
void UKNDynamicIconWidget::SetOverclockPoint(float InPoint)
{
    const int32 NewStageIndex = CalculateStageIndex(InPoint);
    if (NewStageIndex == CachedStageIndex) return;

    const int32 PrevStageIndex = CachedStageIndex;
    CachedStageIndex = NewStageIndex;

    // LV0 진입/이탈 시 NumberIndex 애니메이션 없이 머터리얼만 교체합니다.
    if (NewStageIndex == 0 || PrevStageIndex == 0)
    {
        ApplyMaterialForStage(NewStageIndex);
        return;
    }

    // LV1~LV3 사이 전환 시 NumberIndex 애니메이션을 재생합니다.
    const float FromIndex = GetNumberIndexForStage(PrevStageIndex);
    const float ToIndex = GetNumberIndexForStage(NewStageIndex);

    // 전환 시작 시점에 목표 단계의 머터리얼(색상)을 즉시 교체합니다.
    ApplyMaterialForStage(NewStageIndex);

    StartTransition(FromIndex, ToIndex);
}
#pragma endregion 외부 제어 인터페이스 구현

#pragma region 내부 헬퍼 함수 구현
int32 UKNDynamicIconWidget::CalculateStageIndex(float InPoint) const
{
    if (StageThresholds.IsEmpty()) return 0;

    // 임계값 배열을 역순으로 순회하여 현재 구간을 O(N) 으로 판별합니다.
    // StageThresholds = [100, 200, 300] 기준:
    // InPoint >= 300 → 인덱스 3 (LV3)
    // InPoint >= 200 → 인덱스 2 (LV2)
    // InPoint >= 100 → 인덱스 1 (LV1)
    // InPoint <  100 → 인덱스 0 (LV0)
    for (int32 i = StageThresholds.Num() - 1; i >= 0; --i)
    {
        if (InPoint >= StageThresholds[i])
        {
            return i + 1;
        }
    }

    return 0;
}

void UKNDynamicIconWidget::ApplyMaterialForStage(int32 InStageIndex)
{
    if (!Image_Icon) return;

    UMaterialInterface* TargetMaterial = GetMaterialForStage(InStageIndex);
    if (!TargetMaterial) return;

    // 단계가 바뀔 때마다 새 동적 인스턴스를 생성합니다.
    DynamicMaterial = UMaterialInstanceDynamic::Create(TargetMaterial, this);
    Image_Icon->SetBrushFromMaterial(DynamicMaterial);

    // LV0은 숫자를 숨깁니다.
    if (DynamicMaterial)
    {
        const float ShowTexture = (InStageIndex == 0) ? 0.0f : 1.0f;
        DynamicMaterial->SetScalarParameterValue(ShowTextureParamName, ShowTexture);

        // 초기 NumberIndex를 현재 단계 값으로 세팅합니다.
        if (InStageIndex > 0)
        {
            DynamicMaterial->SetScalarParameterValue(
                NumberIndexParamName,
                GetNumberIndexForStage(InStageIndex));
        }
    }
}

UMaterialInterface* UKNDynamicIconWidget::GetMaterialForStage(int32 InStageIndex) const
{
    switch (InStageIndex)
    {
    case 0:  return MaterialLv0;
    case 1:  return MaterialLv1;
    case 2:  return MaterialLv2;
    case 3:  return MaterialLv3;
    default: return MaterialLv0;
    }
}

float UKNDynamicIconWidget::GetNumberIndexForStage(int32 InStageIndex) const
{
    // LV1 = 0.0, LV2 = 0.5, LV3 = 1.0
    switch (InStageIndex)
    {
    case 1:  return 0.0f;
    case 2:  return 0.5f;
    case 3:  return 1.0f;
    default: return 0.0f;
    }
}

void UKNDynamicIconWidget::StartTransition(float FromIndex, float ToIndex)
{
    // 진행 중인 애니메이션이 있으면 중단하고 새로 시작합니다.
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TransitionTimerHandle);
    }

    TransitionFromIndex = FromIndex;
    TransitionToIndex = ToIndex;
    TransitionElapsed = 0.0f;
    bIsTransitioning = true;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TransitionTimerHandle,
            this,
            &UKNDynamicIconWidget::OnTransitionTick,
            TransitionTickInterval,
            /*bLoop=*/true);
    }
}

void UKNDynamicIconWidget::OnTransitionTick()
{
    if (!bIsTransitioning || !DynamicMaterial) return;

    TransitionElapsed += TransitionTickInterval;

    const float Alpha = FMath::Clamp(TransitionElapsed / TransitionDuration, 0.0f, 1.0f);

    // EaseInOut 보간으로 숫자 전환 연출을 부드럽게 만듭니다.
    const float EasedAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
    const float CurrentIndex = FMath::Lerp(TransitionFromIndex, TransitionToIndex, EasedAlpha);

    DynamicMaterial->SetScalarParameterValue(NumberIndexParamName, CurrentIndex);

    // 애니메이션 완료 시 타이머를 해제합니다.
    if (Alpha >= 1.0f)
    {
        bIsTransitioning = false;

        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(TransitionTimerHandle);
        }
    }
}
#pragma endregion 내부 헬퍼 함수 구현