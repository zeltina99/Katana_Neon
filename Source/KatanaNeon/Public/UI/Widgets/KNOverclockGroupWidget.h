// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Base/KNUserWidgetBase.h"
#include "KNOverclockGroupWidget.generated.h"

#pragma region 전방 선언
class UKNCircularGaugeWidget;
class UKNDynamicIconWidget;
#pragma endregion 전방 선언
/**
 * @file    KNOverclockGroupWidget.h
 * @class   UKNOverclockGroupWidget
 * @brief   원형 게이지(링)와 레벨 아이콘을 하나로 묶는 오버클럭 UI 조립 위젯입니다.
 * @details SRP 원칙에 따라 두 원자 위젯을 조합하고
 *          외부로부터 오버클럭 포인트를 받아 두 위젯에 동시에 전달하는
 *          단 하나의 책임만 가집니다.
 */
UCLASS()
class KATANANEON_API UKNOverclockGroupWidget : public UKNUserWidgetBase
{
	GENERATED_BODY()
	
#pragma region 외부 제어 인터페이스
public:
    /**
     * @brief 오버클럭 포인트를 받아 링 게이지와 레벨 아이콘에 동시에 전달합니다.
     * @param InPoint 현재 오버클럭 포인트 (0.0 ~ 300.0)
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|Overclock")
    void SetOverclockPoint(float InPoint);
#pragma endregion 외부 제어 인터페이스

#pragma region UMG 바인딩
protected:
    /**
     * @brief 원형 링 게이지 위젯입니다.
     * @details 블루프린트 위젯 이름이 반드시 CircularGauge_Widget 이어야 합니다.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UKNCircularGaugeWidget> CircularGauge_Widget = nullptr;

    /**
     * @brief 레벨 아이콘 위젯입니다.
     * @details 블루프린트 위젯 이름이 반드시 OverclockIcon_Widget 이어야 합니다.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UKNDynamicIconWidget> OverclockIcon_Widget = nullptr;
#pragma endregion UMG 바인딩
};
