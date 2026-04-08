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

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 각 링의 구간 최대값 배열입니다.
     * @details [0]=100, [1]=200, [2]=300 — 에디터에서 DataTable 임계값과 동일하게 맞춥니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|Overclock|Config")
    TArray<float> StageThresholds = { 100.0f, 200.0f, 300.0f };
#pragma endregion 에디터 설정 데이터

#pragma region UMG 바인딩
protected:
    /**
     * @brief Lv1 노란색 링 게이지 (0~100 담당).
     * @details 블루프린트 위젯 이름이 반드시 CircularGauge_Lv1_Widget 이어야 합니다.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UKNCircularGaugeWidget> CircularGauge_Lv1_Widget = nullptr;

    /**
     * @brief Lv2 파란색 링 게이지 (100~200 담당).
     * @details 블루프린트 위젯 이름이 반드시 CircularGauge_Lv2_Widget 이어야 합니다.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UKNCircularGaugeWidget> CircularGauge_Lv2_Widget = nullptr;

    /**
     * @brief Lv3 빨간색 링 게이지 (200~300 담당).
     * @details 블루프린트 위젯 이름이 반드시 CircularGauge_Lv3_Widget 이어야 합니다.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UKNCircularGaugeWidget> CircularGauge_Lv3_Widget = nullptr;

    /**
     * @brief 레벨 아이콘 위젯입니다.
     * @details 블루프린트 위젯 이름이 반드시 OverclockIcon_Widget 이어야 합니다.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UKNDynamicIconWidget> OverclockIcon_Widget = nullptr;
#pragma endregion UMG 바인딩

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief 특정 구간의 포인트를 0~구간최대값 범위로 잘라서 반환합니다.
     * @param InPoint      전체 포인트
     * @param InStageIndex 구간 인덱스 (0=Lv1, 1=Lv2, 2=Lv3)
     * @return 해당 구간 내 포인트 (0.0 ~ 구간 최대값)
     */
    float GetPointForStage(float InPoint, int32 InStageIndex) const;
#pragma endregion 내부 헬퍼 함수
};
