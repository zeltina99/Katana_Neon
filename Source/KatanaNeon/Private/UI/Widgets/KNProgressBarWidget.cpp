// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/KNProgressBarWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

#pragma region 위젯 생명주기 오버라이드 구현
void UKNProgressBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 에디터에서 할당한 머터리얼로 동적 인스턴스를 생성합니다.
    // 동적 인스턴스를 쓰지 않으면 SetVectorParameterValue 호출이 원본 에셋을 오염시킵니다.
    if (FillMaterial && Image_Fill)
    {
        DynamicFillMaterial = UMaterialInstanceDynamic::Create(FillMaterial, this);
        Image_Fill->SetBrushFromMaterial(DynamicFillMaterial);

        DynamicFillMaterial->SetScalarParameterValue(FillPercentParamName, 1.0f);
    }
}
#pragma endregion 위젯 생명주기 오버라이드 구현

#pragma region 외부 제어 인터페이스 구현
void UKNProgressBarWidget::SetPercent(float InPercent)
{
    const float ClampedPercent = FMath::Clamp(InPercent, 0.0f, 1.0f);
    if (FMath::IsNearlyEqual(CachedPercent, ClampedPercent)) return;

    CachedPercent = ClampedPercent;

    // 스케일 조절 대신 머터리얼 파라미터로 채움 비율을 전달합니다.
    // 머터리얼이 평행사변형 모양을 유지한 채로 왼쪽부터 채워집니다.
    if (DynamicFillMaterial)
    {
        DynamicFillMaterial->SetScalarParameterValue(FillPercentParamName, CachedPercent);
    }
}

void UKNProgressBarWidget::SetFillColor(const FLinearColor& InColor)
{
    if (DynamicFillMaterial)
    {
        DynamicFillMaterial->SetVectorParameterValue(FillColorParamName, InColor);
    }
}
#pragma endregion 외부 제어 인터페이스 구현