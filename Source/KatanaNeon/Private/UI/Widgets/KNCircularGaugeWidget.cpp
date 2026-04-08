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
        DynamicGaugeMaterial->SetScalarParameterValue(FillPercentParamName, 0.0f);
    }
}
#pragma endregion 위젯 생명주기 오버라이드 구현

#pragma region 외부 제어 인터페이스 구현
void UKNCircularGaugeWidget::SetPoint(float InPoint)
{
    if (!DynamicGaugeMaterial) return;
    if (MaxPoint <= 0.0f) return;

    const float NewPercent = FMath::Clamp(InPoint / MaxPoint, 0.0f, 1.0f);
    if (FMath::IsNearlyEqual(CachedPercent, NewPercent)) return;

    CachedPercent = NewPercent;
    DynamicGaugeMaterial->SetScalarParameterValue(FillPercentParamName, CachedPercent);
}
#pragma endregion 외부 제어 인터페이스 구현

//#pragma region 내부 헬퍼 함수 구현
//void UKNCircularGaugeWidget::CalculateStageAndPercent(
//    float InPoint,
//    int32& OutStageIndex,
//    float& OutPercent) const
//{
//    if (StageThresholds.IsEmpty())
//    {
//        OutStageIndex = 0;
//        OutPercent = 0.0f;
//        return;
//    }
//
//    float PrevMax = 0.0f;
//
//    for (int32 i = 0; i < StageThresholds.Num(); ++i)
//    {
//        const float StageMax = StageThresholds[i];
//        const float StageRange = StageMax - PrevMax;
//
//        if (InPoint <= StageMax || i == StageThresholds.Num() - 1)
//        {
//            OutStageIndex = i;
//            OutPercent = (StageRange > 0.0f)
//                ? FMath::Clamp((InPoint - PrevMax) / StageRange, 0.0f, 1.0f)
//                : 0.0f;
//            return;
//        }
//
//        PrevMax = StageMax;
//    }
//
//    OutStageIndex = StageThresholds.Num() - 1;
//    OutPercent = 1.0f;
//}
//
//void UKNCircularGaugeWidget::ApplyToMaterial(float InPercent, int32 InStageIndex)
//{
//    if (!Image_Gauge) return;
//    if (!StageMaterials.IsValidIndex(InStageIndex)) return;
//    if (!StageMaterials[InStageIndex]) return;
//
//    // 단계에 맞는 머터리얼로 새 동적 인스턴스를 생성합니다.
//    DynamicGaugeMaterial = UMaterialInstanceDynamic::Create(
//        StageMaterials[InStageIndex], this);
//
//    Image_Gauge->SetBrushFromMaterial(DynamicGaugeMaterial);
//
//    DynamicGaugeMaterial->SetScalarParameterValue(FillPercentParamName, InPercent);
//}
//#pragma endregion 내부 헬퍼 함수 구현
