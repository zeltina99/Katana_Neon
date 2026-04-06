// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/KNCircularGaugeWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

#pragma region 위젯 생명주기 오버라이드 구현
void UKNCircularGaugeWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (GaugeMaterial && Image_Gauge)
    {
        DynamicGaugeMaterial = UMaterialInstanceDynamic::Create(GaugeMaterial, this);
        Image_Gauge->SetBrushFromMaterial(DynamicGaugeMaterial);
    }
}
#pragma endregion 위젯 생명주기 오버라이드 구현

#pragma region 외부 제어 인터페이스 구현
void UKNCircularGaugeWidget::SetOverclockPoint(float InPoint)
{
    int32 NewStageIndex = 0;
    float NewPercent = 0.0f;

    CalculateStageAndPercent(InPoint, NewStageIndex, NewPercent);

    // 단계와 퍼센트가 모두 동일하면 머터리얼 호출을 차단합니다.
    if (NewStageIndex == CachedStageIndex &&
        FMath::IsNearlyEqual(CachedPercent, NewPercent))
    {
        return;
    }

    CachedStageIndex = NewStageIndex;
    CachedPercent = NewPercent;

    ApplyToMaterial(CachedPercent, CachedStageIndex);
}
#pragma endregion 외부 제어 인터페이스 구현

#pragma region 내부 헬퍼 함수 구현
void UKNCircularGaugeWidget::CalculateStageAndPercent(
    float InPoint,
    int32& OutStageIndex,
    float& OutPercent) const
{
    if (Stages.IsEmpty())
    {
        OutStageIndex = 0;
        OutPercent = 0.0f;
        return;
    }

    float PrevMax = 0.0f;

    for (int32 i = 0; i < Stages.Num(); ++i)
    {
        const float StageMax = Stages[i].MaxPoint;
        const float StageRange = StageMax - PrevMax;

        if (InPoint <= StageMax || i == Stages.Num() - 1)
        {
            OutStageIndex = i;
            // 단계 내 포인트 비율을 0.0~1.0으로 정규화합니다.
            OutPercent = (StageRange > 0.0f)
                ? FMath::Clamp((InPoint - PrevMax) / StageRange, 0.0f, 1.0f)
                : 0.0f;
            return;
        }

        PrevMax = StageMax;
    }

    OutStageIndex = Stages.Num() - 1;
    OutPercent = 1.0f;
}

void UKNCircularGaugeWidget::ApplyToMaterial(float InPercent, int32 InStageIndex)
{
    if (!DynamicGaugeMaterial) return;
    if (!Stages.IsValidIndex(InStageIndex)) return;

    DynamicGaugeMaterial->SetScalarParameterValue(FillPercentParamName, InPercent);
    DynamicGaugeMaterial->SetVectorParameterValue(FillColorParamName, Stages[InStageIndex].StageColor);
}
#pragma endregion 내부 헬퍼 함수 구현
