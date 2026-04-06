// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Base/KNUserWidgetBase.h"
#include "KNCircularGaugeWidget.generated.h"

#pragma region 전방 선언
class UImage;
class UMaterialInstanceDynamic;
class UMaterialInterface;
#pragma endregion 전방 선언

#pragma region 단계 설정 구조체
/**
 * @struct FKNGaugeStageData
 * @brief  오버클럭 단계별 임계값과 색상을 정의하는 데이터 구조체입니다.
 * @details 에디터에서 단계별 색상과 최대값을 직접 설정합니다.
 *          하드코딩 없이 단계 수를 자유롭게 추가/변경할 수 있습니다.
 */
USTRUCT(BlueprintType)
struct FKNGaugeStageData
{
    GENERATED_BODY()

    /** @brief 이 단계의 최대 포인트 (예: 1단계=100, 2단계=200, 3단계=300) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|CircularGauge")
    float MaxPoint = 100.0f;

    /** @brief 이 단계에서 표시할 게이지 색상 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|CircularGauge")
    FLinearColor StageColor = FLinearColor::White;
};
#pragma endregion 단계 설정 구조체

/**
 * @file    KNCircularGaugeWidget.h
 * @class   UKNCircularGaugeWidget
 * @brief   오버클럭 포인트(0~300)를 받아 단계별 색상과 회전 비율을 표현하는 원형 게이지 위젯입니다.
 * @details 0~100 노란색 한 바퀴, 100~200 파란색 한 바퀴, 200~300 빨간색 한 바퀴.
 *          단계 정의는 에디터의 Stages 배열에서 데이터 드리븐으로 관리합니다.
 */
UCLASS()
class KATANANEON_API UKNCircularGaugeWidget : public UKNUserWidgetBase
{
	GENERATED_BODY()
	
#pragma region 위젯 생명주기 오버라이드
protected:
    /**
     * @brief 위젯 초기화 시 머터리얼 다이나믹 인스턴스를 생성합니다.
     */
    virtual void NativeConstruct() override;
#pragma endregion 위젯 생명주기 오버라이드

#pragma region 외부 제어 인터페이스
public:
    /**
     * @brief 오버클럭 포인트 전체 값을 받아 단계와 퍼센트를 자동으로 계산합니다.
     * @param InPoint 현재 오버클럭 포인트 (0.0 ~ MaxPoint 합산)
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|CircularGauge")
    void SetOverclockPoint(float InPoint);

    /**
     * @brief 현재 게이지의 채움 비율을 반환합니다.
     * @return 0.0 ~ 1.0 사이의 현재 단계 내 퍼센트 값
     */
    UFUNCTION(BlueprintPure, Category = "KatanaNeon|UI|CircularGauge")
    float GetPercent() const { return CachedPercent; }
#pragma endregion 외부 제어 인터페이스

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 단계별 머터리얼 인스턴스 배열입니다.
     * @details [0]=MI_UI_RadialGauge_Lv1, [1]=MI_UI_RadialGauge_Lv2, [2]=MI_UI_RadialGauge_Lv3
     *          단계가 바뀔 때 해당 인스턴스로 교체됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|CircularGauge|Config")
    TArray<TObjectPtr<UMaterialInterface>> StageMaterials;

    /** @brief 머터리얼의 채움 비율 스칼라 파라미터 이름입니다. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|CircularGauge|Config")
    FName FillPercentParamName = FName("Progress");

    /** @brief 단계별 임계값 배열입니다. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|CircularGauge|Config")
    TArray<float> StageThresholds = { 100.0f, 200.0f, 300.0f };
#pragma endregion 에디터 설정 데이터

#pragma region UMG 바인딩
protected:
    /**
     * @brief 원형 게이지를 표현하는 단일 이미지 위젯입니다.
     * @details 블루프린트 위젯 이름이 반드시 Image_Gauge 이어야 합니다.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> Image_Gauge = nullptr;
#pragma endregion UMG 바인딩

#pragma region 런타임 상태
private:
    /** @brief 게이지 이미지에 적용된 동적 머터리얼 인스턴스 */
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DynamicGaugeMaterial = nullptr;

    /** @brief 현재 단계 내 퍼센트 캐시 */
    float CachedPercent = 0.0f;

    /** @brief 현재 활성 단계 인덱스 캐시 */
    int32 CachedStageIndex = 0;
#pragma endregion 런타임 상태

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief 총 포인트값으로 현재 단계 인덱스와 단계 내 퍼센트를 계산합니다.
     * @param InPoint   총 오버클럭 포인트
     * @param OutStageIndex 계산된 단계 인덱스
     * @param OutPercent    계산된 단계 내 퍼센트 (0.0~1.0)
     */
    void CalculateStageAndPercent(float InPoint, int32& OutStageIndex, float& OutPercent) const;

    /**
     * @brief 머터리얼에 퍼센트와 색상을 적용합니다.
     * @param InPercent    적용할 퍼센트
     * @param InStageIndex 적용할 단계 인덱스
     */
    void ApplyToMaterial(float InPercent, int32 InStageIndex);
#pragma endregion 내부 헬퍼 함수
};
