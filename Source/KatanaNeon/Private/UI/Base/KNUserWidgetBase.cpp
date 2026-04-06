// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Base/KNUserWidgetBase.h"

#pragma region 위젯 생명주기 오버라이드 구현
void UKNUserWidgetBase::NativeConstruct()
{
    Super::NativeConstruct();
    // 하위 클래스에서 Super::NativeConstruct() 호출 후 데이터 바인딩을 수행합니다.
}

void UKNUserWidgetBase::NativeDestruct()
{
    // 하위 클래스에서 Super::NativeDestruct() 호출 전 델리게이트 해제를 수행합니다.
    Super::NativeDestruct();
}
#pragma endregion 위젯 생명주기 오버라이드 구현

#pragma region 공통 표시 제어 인터페이스 구현
void UKNUserWidgetBase::ShowWidget_Implementation()
{
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UKNUserWidgetBase::HideWidget_Implementation()
{
    SetVisibility(ESlateVisibility::Collapsed);
}
#pragma endregion 공통 표시 제어 인터페이스 구현