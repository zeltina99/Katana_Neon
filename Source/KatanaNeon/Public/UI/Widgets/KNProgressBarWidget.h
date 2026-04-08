// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Base/KNUserWidgetBase.h"
#include "KNProgressBarWidget.generated.h"

#pragma region 전방 선언
class UImage;
class UMaterialInstanceDynamic;
#pragma endregion 전방 선언

/**
 * @file    KNProgressBarWidget.h
 * @class   UKNProgressBarWidget
 * @brief   체력/스태미나/크로노스/보스체력에 공용으로 재사용되는 범용 이미지 기반 프로그레스 바 위젯입니다.
 * @details UProgressBar 대신 UImage 두 장(배경/채우기)으로 구성하여
 *          평행사변형 등 머터리얼 모양을 클리핑 없이 그대로 표현합니다.
 *          SetPercent 호출 시 채우기 이미지의 스케일 X를 조절하여 채움 비율을 표현합니다.
 */
UCLASS()
class KATANANEON_API UKNProgressBarWidget : public UKNUserWidgetBase
{
	GENERATED_BODY()
	
#pragma region 위젯 생명주기 오버라이드
protected:
    /** @brief 위젯 초기화 시 에디터에서 설정한 기본 색상을 바에 적용합니다. */
    virtual void NativeConstruct() override;
#pragma endregion 위젯 생명주기 오버라이드

#pragma region 외부 제어 인터페이스
public:
    /**
     * @brief 바의 채움 비율을 설정합니다.
     * @details 채우기 이미지의 렌더 트랜스폼 스케일 X를 조절하여 비율을 표현합니다.
     * @param InPercent 0.0(빈 상태) ~ 1.0(가득 찬 상태) 사이의 정규화된 값
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|ProgressBar")
    void SetPercent(float InPercent);

    /**
     * @brief 머터리얼의 Filled pip color 파라미터를 동적으로 변경합니다.
     * @param InColor 적용할 선형 색상
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|ProgressBar")
    void SetFillColor(const FLinearColor& InColor);

    /**
     * @brief 현재 바의 채움 비율을 반환합니다.
     * @return 0.0 ~ 1.0 사이의 현재 퍼센트 값
     */
    UFUNCTION(BlueprintPure, Category = "KatanaNeon|UI|ProgressBar")
    float GetPercent() const { return CachedPercent; }
#pragma endregion 외부 제어 인터페이스

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 채우기 이미지에 적용할 머터리얼 인스턴스입니다.
     * @details WBP마다 각자 색상이 다른 MI를 에디터에서 할당합니다.
     *          (MI_UI_PB_HP, MI_UI_PB_STM, MI_UI_PB_CHR, MI_UI_PB_Boss)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|ProgressBar|Config")
    TObjectPtr<UMaterialInterface> FillMaterial = nullptr;

    /**
     * @brief 머터리얼의 Filled pip color 파라미터 이름입니다.
     * @details 머터리얼에서 정의한 파라미터 이름과 반드시 일치해야 합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|ProgressBar|Config")
    FName FillColorParamName = FName("Filled pip color");

    /**
     * @brief 머터리얼의 채움 비율 스칼라 파라미터 이름입니다.
     * @details 머터리얼에서 정의한 파라미터 이름과 반드시 일치해야 합니다.
     *          에디터 클래스 디폴트에서 WBP마다 직접 입력합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|ProgressBar|Config")
    FName FillPercentParamName = FName("Progress");
#pragma endregion 에디터 설정 데이터

#pragma region UMG 바인딩
protected:
    /**
     * @brief 배경과 채우기를 모두 처리하는 단일 이미지 위젯입니다.
     * @details 블루프린트 위젯 이름이 반드시 Image_Fill 이어야 합니다.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> Image_Fill = nullptr;
#pragma endregion UMG 바인딩

#pragma region 런타임 상태
private:
    /** @brief 채우기 이미지에 적용된 동적 머터리얼 인스턴스 */
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DynamicFillMaterial = nullptr;

    /** @brief 현재 적용된 퍼센트 캐시 — 불필요한 렌더 트랜스폼 호출을 방지합니다. */
    float CachedPercent = -1.0f;
#pragma endregion 런타임 상태
};
