// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Base/KNUserWidgetBase.h"
#include "KNWeaponStateWidget.generated.h"

#pragma region 전방 선언
class UImage;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UWidgetAnimation;
#pragma endregion 전방 선언

/**
 * @file    KNWeaponStateWidget.h
 * @class   UKNWeaponStateWidget
 * @brief   발도/납도 상태에 따라 이미지를 교체하는 무기 상태 위젯입니다.
 * @details SRP 원칙에 따라 "발도/납도 상태를 받아 이미지를 교체한다"는
 *          단 하나의 책임만 가집니다.
 */
UCLASS()
class KATANANEON_API UKNWeaponStateWidget : public UKNUserWidgetBase
{
	GENERATED_BODY()
	
#pragma region 위젯 생명주기 오버라이드
protected:
    /** @brief 초기 상태(납도)로 시작합니다. */
    virtual void NativeConstruct() override;
#pragma endregion 위젯 생명주기 오버라이드

#pragma region 외부 제어 인터페이스
public:
    /**
     * @brief 무기 상태를 설정합니다.
     * @param bIsDrawn true = 발도, false = 납도
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|WeaponState")
    void SetWeaponDrawn(bool bIsDrawn);
#pragma endregion 외부 제어 인터페이스

#pragma region UMG 애니메이션 바인딩
protected:
    /**
     * @brief 발도/납도 전환 시 재생되는 글리치 애니메이션입니다.
     * @details 블루프린트 애니메이션 이름이 반드시 Anim_WeaponSwap 이어야 합니다.
     */
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> Anim_WeaponSwap = nullptr;
#pragma endregion UMG 애니메이션 바인딩

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 발도 상태 머터리얼 인스턴스입니다.
     * @details 에디터에서 발도 상태 이미지 머터리얼을 할당합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|WeaponState|Config")
    TObjectPtr<UMaterialInterface> DrawnMaterial = nullptr;

    /**
     * @brief 납도 상태 머터리얼 인스턴스입니다.
     * @details 에디터에서 납도 상태 이미지 머터리얼을 할당합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|WeaponState|Config")
    TObjectPtr<UMaterialInterface> SheathMaterial = nullptr;
#pragma endregion 에디터 설정 데이터

#pragma region UMG 바인딩
protected:
    /**
     * @brief 무기 상태 이미지 위젯입니다.
     * @details 블루프린트 위젯 이름이 반드시 Image_WeaponState 이어야 합니다.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> Image_WeaponState = nullptr;
#pragma endregion UMG 바인딩

#pragma region 런타임 상태
private:
    /** @brief 현재 발도 상태 캐시 */
    bool bCachedIsDrawn = false;
#pragma endregion 런타임 상태
};
