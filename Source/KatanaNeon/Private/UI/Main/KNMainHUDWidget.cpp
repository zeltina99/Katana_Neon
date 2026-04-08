// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Main/KNMainHUDWidget.h"
#include "UI/Widgets/KNProgressBarWidget.h"
#include "UI/Widgets/KNOverclockGroupWidget.h"
#include "UI/Widgets/KNDynamicIconWidget.h"
#include "UI/Widgets/KNWeaponStateWidget.h"
#include "GAS/Components/KNStatsComponent.h" 
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/KNAttributeSet.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 위젯 생명주기 오버라이드 구현
void UKNMainHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 보스 UI는 기본적으로 숨깁니다.
    SetBossHUDVisible(false);
}
#pragma endregion 위젯 생명주기 오버라이드 구현

#pragma region 외부 제어 인터페이스 구현
void UKNMainHUDWidget::InitHUD(UKNStatsComponent* InStatsComponent)
{
    if (!InStatsComponent) return;

    InStatsComponent->OnHealthChanged.AddDynamic(this, &UKNMainHUDWidget::OnHealthChangedCallback);
    InStatsComponent->OnStaminaChanged.AddDynamic(this, &UKNMainHUDWidget::OnStaminaChangedCallback);
    InStatsComponent->OnChronosChanged.AddDynamic(this, &UKNMainHUDWidget::OnChronosChangedCallback);
    InStatsComponent->OnOverclockPointChanged.AddDynamic(this, &UKNMainHUDWidget::OnOverclockPointChangedCallback);
    InStatsComponent->OnWeaponStateChanged.AddDynamic(this, &UKNMainHUDWidget::OnWeaponStateChangedCallback);
}

void UKNMainHUDWidget::UpdateHealth(float Current, float Max)
{
    if (HealthBar_Widget && Max > 0.0f)
    {
        HealthBar_Widget->SetPercent(Current / Max);
    }
}

void UKNMainHUDWidget::UpdateStamina(float Current, float Max)
{
    if (StaminaBar_Widget && Max > 0.0f)
    {
        StaminaBar_Widget->SetPercent(Current / Max);
    }
}

void UKNMainHUDWidget::UpdateChronos(float Current, float Max)
{
    if (ChronosBar_Widget && Max > 0.0f)
    {
        ChronosBar_Widget->SetPercent(Current / Max);
    }
}

void UKNMainHUDWidget::UpdateOverclockPoint(float Current, float Max)
{
    if (OverclockGroup_Widget)
    {
        OverclockGroup_Widget->SetOverclockPoint(Current);
    }
}

void UKNMainHUDWidget::UpdateBossHealth(float Current, float Max)
{
    if (BossHealthBar_Widget && Max > 0.0f)
    {
        BossHealthBar_Widget->SetPercent(Current / Max);
    }
}

void UKNMainHUDWidget::UpdateWeaponState(bool bIsDrawn)
{
    if (WeaponState_Widget)
    {
        WeaponState_Widget->SetWeaponDrawn(bIsDrawn);
    }
}

void UKNMainHUDWidget::SetBossHUDVisible(bool bVisible)
{
    if (BossHealthBar_Widget)
    {
        bVisible ? BossHealthBar_Widget->ShowWidget() : BossHealthBar_Widget->HideWidget();
    }
}

void UKNMainHUDWidget::SyncInitialValues(UKNStatsComponent* InStatsComponent)
{
    if (!InStatsComponent) return;

    // ASC와 AttributeSet이 초기화되지 않은 경우를 방어합니다.
    UAbilitySystemComponent* ASC = InStatsComponent->GetOwner()
        ->FindComponentByClass<UAbilitySystemComponent>();
    if (!ASC) return;

    const UKNAttributeSet* AttrSet = ASC->GetSet<UKNAttributeSet>();
    if (!AttrSet) return;

    // 무기 초기 상태 동기화
    if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::WeaponDrawn))
    {
        UpdateWeaponState(true);
    }
    else
    {
        UpdateWeaponState(false);
    }

    UpdateHealth(AttrSet->GetHealth(), AttrSet->GetMaxHealth());
    UpdateStamina(AttrSet->GetStamina(), AttrSet->GetMaxStamina());
    UpdateChronos(AttrSet->GetChronos(), AttrSet->GetMaxChronos());
    UpdateOverclockPoint(AttrSet->GetOverclockPoint(), AttrSet->GetMaxOverclockPoint());
}
#pragma endregion 외부 제어 인터페이스 구현

#pragma region 내부 콜백 함수 구현
void UKNMainHUDWidget::OnHealthChangedCallback(float Current, float Max)
{
    UpdateHealth(Current, Max);
}

void UKNMainHUDWidget::OnStaminaChangedCallback(float Current, float Max)
{
    UpdateStamina(Current, Max);
}

void UKNMainHUDWidget::OnChronosChangedCallback(float Current, float Max)
{
    UpdateChronos(Current, Max);
}

void UKNMainHUDWidget::OnOverclockPointChangedCallback(float Current, float Max)
{
    UpdateOverclockPoint(Current, Max);
}

void UKNMainHUDWidget::OnWeaponStateChangedCallback(bool bIsDrawn)
{
    UpdateWeaponState(bIsDrawn);
}
#pragma endregion 내부 콜백 함수 구현
