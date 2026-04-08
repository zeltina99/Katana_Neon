// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/KNOverclockGroupWidget.h"
#include "UI/Widgets/KNCircularGaugeWidget.h"
#include "UI/Widgets/KNDynamicIconWidget.h"

#pragma region 외부 제어 인터페이스 구현
void UKNOverclockGroupWidget::SetOverclockPoint(float InPoint)
{
    // 각 링에 해당 구간 포인트를 분배합니다.
    if (CircularGauge_Lv1_Widget)
    {
        CircularGauge_Lv1_Widget->SetPoint(GetPointForStage(InPoint, 0));

        // Lv2, Lv3는 이전 단계가 100 이상일 때만 표시합니다.
        const float Lv1Max = StageThresholds.IsValidIndex(0) ? StageThresholds[0] : 100.0f;
        if (CircularGauge_Lv2_Widget)
        {
            CircularGauge_Lv2_Widget->SetVisibility(
                InPoint >= Lv1Max
                ? ESlateVisibility::HitTestInvisible
                : ESlateVisibility::Collapsed);

            CircularGauge_Lv2_Widget->SetPoint(GetPointForStage(InPoint, 1));
        }

        const float Lv2Max = StageThresholds.IsValidIndex(1) ? StageThresholds[1] : 200.0f;
        if (CircularGauge_Lv3_Widget)
        {
            CircularGauge_Lv3_Widget->SetVisibility(
                InPoint >= Lv2Max
                ? ESlateVisibility::HitTestInvisible
                : ESlateVisibility::Collapsed);

            CircularGauge_Lv3_Widget->SetPoint(GetPointForStage(InPoint, 2));
        }
    }

    if (OverclockIcon_Widget)
    {
        OverclockIcon_Widget->SetOverclockPoint(InPoint);
    }
}
#pragma endregion 외부 제어 인터페이스 구현

#pragma region 내부 헬퍼 함수 구현
float UKNOverclockGroupWidget::GetPointForStage(float InPoint, int32 InStageIndex) const
{
    if (!StageThresholds.IsValidIndex(InStageIndex)) return 0.0f;

    const float StageMin = (InStageIndex == 0)
        ? 0.0f
        : StageThresholds[InStageIndex - 1];

    const float StageMax = StageThresholds[InStageIndex];

    return FMath::Clamp(InPoint - StageMin, 0.0f, StageMax - StageMin);
}
#pragma endregion 내부 헬퍼 함수 구현