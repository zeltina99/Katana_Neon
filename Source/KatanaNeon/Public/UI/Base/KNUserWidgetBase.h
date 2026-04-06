// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KNUserWidgetBase.generated.h"

/**
 * @file    KNUserWidgetBase.h
 * @class   UKNUserWidgetBase
 * @brief   KatanaNeon 모든 UI 위젯의 최상위 부모 클래스입니다.
 * @details SRP 원칙에 따라 공통 초기화/해제 생명주기만 담당합니다.
 *          모든 하위 위젯은 반드시 이 클래스를 상속받아 제작합니다.
 */
UCLASS()
class KATANANEON_API UKNUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
#pragma region 위젯 생명주기 오버라이드
protected:
    /**
     * @brief 위젯이 뷰포트에 추가될 때 호출됩니다.
     * @details 하위 클래스에서 오버라이드하여 초기 데이터 바인딩을 수행합니다.
     *          반드시 Super::NativeConstruct()를 호출해야 합니다.
     */
    virtual void NativeConstruct() override;

    /**
     * @brief 위젯이 뷰포트에서 제거될 때 호출됩니다.
     * @details 하위 클래스에서 오버라이드하여 델리게이트 바인딩 해제 등 정리 작업을 수행합니다.
     *          반드시 Super::NativeDestruct()를 호출해야 합니다.
     */
    virtual void NativeDestruct() override;
#pragma endregion 위젯 생명주기 오버라이드

#pragma region 공통 표시 제어 인터페이스
public:
    /**
     * @brief 위젯을 부드럽게 표시합니다.
     * @details 블루프린트에서 페이드인 애니메이션을 오버라이드할 수 있습니다.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "KatanaNeon|UI|Visibility")
    void ShowWidget();

    /**
     * @brief 위젯을 부드럽게 숨깁니다.
     * @details 블루프린트에서 페이드아웃 애니메이션을 오버라이드할 수 있습니다.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "KatanaNeon|UI|Visibility")
    void HideWidget();
#pragma endregion 공통 표시 제어 인터페이스
};
