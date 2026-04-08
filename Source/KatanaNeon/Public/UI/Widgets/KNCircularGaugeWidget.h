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
 * @brief   단일 원형 링 게이지 위젯입니다.
 * @details SRP 원칙에 따라 "0~MaxPoint 범위의 포인트를 받아 링을 채운다"는
 *          단 하나의 책임만 가집니다.
 *          색상과 머터리얼은 에디터에서 할당하며 C++은 Progress 파라미터만 제어합니다.
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
     * @brief 현재 포인트를 받아 Progress 비율로 변환하여 머터리얼에 적용합니다.
     * @param InPoint    현재 포인트 (0.0 ~ MaxPoint)
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|CircularGauge")
    void SetPoint(float InPoint);

    /**
     * @brief 현재 채움 비율을 반환합니다.
     * @return 0.0 ~ 1.0
     */
    UFUNCTION(BlueprintPure, Category = "KatanaNeon|UI|CircularGauge")
    float GetPercent() const { return CachedPercent; }
#pragma endregion 외부 제어 인터페이스

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 이 링에 적용할 머터리얼 인스턴스입니다.
     * @details 에디터에서 MI_UI_RadialGauge_Lv1/Lv2/Lv3 중 하나를 할당합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|CircularGauge|Config")
    TObjectPtr<UMaterialInterface> GaugeMaterial = nullptr;

    /**
     * @brief 머터리얼의 채움 비율 스칼라 파라미터 이름입니다.
     * @details 머터리얼 파라미터 이름과 반드시 일치해야 합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|CircularGauge|Config")
    FName FillPercentParamName = FName("Progress");

    /**
     * @brief 이 링이 담당하는 최대 포인트입니다.
     * @details KNOverclockGroupWidget에서 구간을 잘라서 전달하므로 기본값은 100입니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|CircularGauge|Config")
    float MaxPoint = 100.0f;
#pragma endregion 에디터 설정 데이터

#pragma region UMG 바인딩
protected:
    /**
     * @brief 원형 게이지 이미지 위젯입니다.
     * @details 블루프린트 위젯 이름이 반드시 Image_Gauge 이어야 합니다.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> Image_Gauge = nullptr;
#pragma endregion UMG 바인딩

#pragma region 런타임 상태
private:
    /** @brief 동적 머터리얼 인스턴스 */
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DynamicGaugeMaterial = nullptr;

    /** @brief 현재 채움 비율 캐시 */
    float CachedPercent = 0.0f;
#pragma endregion 런타임 상태

};
