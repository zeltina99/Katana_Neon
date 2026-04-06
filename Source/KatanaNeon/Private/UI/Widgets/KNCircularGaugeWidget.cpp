// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/KNCircularGaugeWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

#pragma region 위젯 생명주기 오버라이드 구현
void UKNCircularGaugeWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기 단계(0) 머터리얼로 시작합니다.
    ApplyToMaterial(0.0f, 0);
}
#pragma endregion 위젯 생명주기 오버라이드 구현

#pragma region 외부 제어 인터페이스 구현
void UKNCircularGaugeWidget::SetOverclockPoint(float InPoint)
{
    int32 NewStageIndex = 0;
    float NewPercent = 0.0f;

    CalculateStageAndPercent(InPoint, NewStageIndex, NewPercent);

    if (NewStageIndex == CachedStageIndex &&
        FMath::IsNearlyEqual(CachedPercent, NewPercent))
    {
        return;
    }

    // 단계가 바뀌면 머터리얼 인스턴스를 교체합니다.
    if (NewStageIndex != CachedStageIndex)
    {
        ApplyToMaterial(NewPercent, NewStageIndex);
    }
    else
    {
        // 같은 단계면 Progress만 업데이트합니다.
        if (DynamicGaugeMaterial)
        {
            DynamicGaugeMaterial->SetScalarParameterValue(FillPercentParamName, NewPercent);
        }
    }

    CachedStageIndex = NewStageIndex;
    CachedPercent = NewPercent;
}
#pragma endregion 외부 제어 인터페이스 구현

#pragma region 내부 헬퍼 함수 구현
void UKNCircularGaugeWidget::CalculateStageAndPercent(
    float InPoint,
    int32& OutStageIndex,
    float& OutPercent) const
{
    if (StageThresholds.IsEmpty())
    {
        OutStageIndex = 0;
        OutPercent = 0.0f;
        return;
    }

    float PrevMax = 0.0f;

    for (int32 i = 0; i < StageThresholds.Num(); ++i)
    {
        const float StageMax = StageThresholds[i];
        const float StageRange = StageMax - PrevMax;

        if (InPoint <= StageMax || i == StageThresholds.Num() - 1)
        {
            OutStageIndex = i;
            OutPercent = (StageRange > 0.0f)
                ? FMath::Clamp((InPoint - PrevMax) / StageRange, 0.0f, 1.0f)
                : 0.0f;
            return;
        }

        PrevMax = StageMax;
    }

    OutStageIndex = StageThresholds.Num() - 1;
    OutPercent = 1.0f;
}

void UKNCircularGaugeWidget::ApplyToMaterial(float InPercent, int32 InStageIndex)
{
    if (!Image_Gauge) return;
    if (!StageMaterials.IsValidIndex(InStageIndex)) return;
    if (!StageMaterials[InStageIndex]) return;

    // 단계에 맞는 머터리얼로 새 동적 인스턴스를 생성합니다.
    DynamicGaugeMaterial = UMaterialInstanceDynamic::Create(
        StageMaterials[InStageIndex], this);

    Image_Gauge->SetBrushFromMaterial(DynamicGaugeMaterial);

    DynamicGaugeMaterial->SetScalarParameterValue(FillPercentParamName, InPercent);
}
#pragma endregion 내부 헬퍼 함수 구현
