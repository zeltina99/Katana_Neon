// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/KNOverclockGroupWidget.h"
#include "UI/Widgets/KNCircularGaugeWidget.h"
#include "UI/Widgets/KNDynamicIconWidget.h"

#pragma region 외부 제어 인터페이스 구현
void UKNOverclockGroupWidget::SetOverclockPoint(float InPoint)
{
    // 두 원자 위젯에 동시에 포인트를 전달합니다.
    if (CircularGauge_Widget)
    {
        CircularGauge_Widget->SetOverclockPoint(InPoint);
    }

    if (OverclockIcon_Widget)
    {
        OverclockIcon_Widget->SetOverclockPoint(InPoint);
    }
}
#pragma endregion 외부 제어 인터페이스 구현