// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/KNWeaponStateWidget.h"
#include "Components/Image.h"

#pragma region 위젯 생명주기 오버라이드 구현
void UKNWeaponStateWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기 상태는 납도입니다.
    SetWeaponDrawn(false);
}
#pragma endregion 위젯 생명주기 오버라이드 구현

#pragma region 외부 제어 인터페이스 구현
void UKNWeaponStateWidget::SetWeaponDrawn(bool bIsDrawn)
{
    if (bCachedIsDrawn == bIsDrawn) return;
    bCachedIsDrawn = bIsDrawn;

    if (!Image_WeaponState) return;

    UMaterialInterface* TargetMaterial = bIsDrawn ? DrawnMaterial : SheathMaterial;
    if (TargetMaterial)
    {
        Image_WeaponState->SetBrushFromMaterial(TargetMaterial);
    }

    if (Anim_WeaponSwap)
    {
        PlayAnimation(Anim_WeaponSwap);
    }
}
#pragma endregion 외부 제어 인터페이스 구현