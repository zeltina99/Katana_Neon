// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Base/KNUserWidgetBase.h"
#include "KNMainHUDWidget.generated.h"

#pragma region 전방 선언
class UKNProgressBarWidget;
class UKNOverclockGroupWidget;
class UKNDynamicIconWidget;
class UKNWeaponStateWidget;
class UKNStatsComponent;
class UAbilitySystemComponent;
class UKNAttributeSet;
#pragma endregion 전방 선언
/**
 * @file    KNMainHUDWidget.h
 * @class   UKNMainHUDWidget
 * @brief   KatanaNeon 인게임 HUD 최상위 조립 위젯입니다.
 * @details MVC 원칙에 따라 모든 원자/그룹 위젯을 조립하고
 *          외부(KNStatsComponent)로부터 데이터를 받아 각 위젯에 분배하는
 *          단 하나의 책임만 가집니다.
 */
UCLASS()
class KATANANEON_API UKNMainHUDWidget : public UKNUserWidgetBase
{
	GENERATED_BODY()
	
#pragma region 위젯 생명주기 오버라이드
protected:
    virtual void NativeConstruct() override;
#pragma endregion 위젯 생명주기 오버라이드

#pragma region 외부 제어 인터페이스
public:
    /**
     * @brief StatsComponent의 델리게이트에 HUD를 구독시킵니다.
     * @details MVC 패턴에 따라 Controller(StatsComponent)와 View(HUD)를 연결합니다.
     *          KNPlayerController에서 위젯 생성 직후 반드시 호출해야 합니다.
     * @param InStatsComponent 플레이어 캐릭터의 KNStatsComponent
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|HUD")
    void InitHUD(UKNStatsComponent* InStatsComponent);

    /**
     * @brief 체력 바를 갱신합니다.
     * @param Current 현재 체력
     * @param Max     최대 체력
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|HUD")
    void UpdateHealth(float Current, float Max);

    /**
     * @brief 스태미나 바를 갱신합니다.
     * @param Current 현재 스태미나
     * @param Max     최대 스태미나
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|HUD")
    void UpdateStamina(float Current, float Max);

    /**
     * @brief 크로노스 바를 갱신합니다.
     * @param Current 현재 크로노스
     * @param Max     최대 크로노스
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|HUD")
    void UpdateChronos(float Current, float Max);

    /**
     * @brief 오버클럭 포인트를 갱신합니다.
     * @param Current 현재 오버클럭 포인트
     * @param Max     최대 오버클럭 포인트
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|HUD")
    void UpdateOverclockPoint(float Current, float Max);

    /**
     * @brief 보스 체력 바를 갱신합니다.
     * @param Current 현재 보스 체력
     * @param Max     최대 보스 체력
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|HUD")
    void UpdateBossHealth(float Current, float Max);

    /**
     * @brief 무기 상태를 갱신합니다.
     * @param bIsDrawn true = 발도, false = 납도
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|HUD")
    void UpdateWeaponState(bool bIsDrawn);

    /**
     * @brief 보스 UI를 표시/숨깁니다.
     * @param bVisible 보스 전투 진입 시 true
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|HUD")
    void SetBossHUDVisible(bool bVisible);

    /**
     * @brief HUD 생성 직후 현재 스탯 값으로 초기 화면을 채웁니다.
     * @details 델리게이트는 값 변경 시에만 발동되므로 초기값은 직접 조회해야 합니다.
     * @param InStatsComponent 플레이어 캐릭터의 KNStatsComponent
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|HUD")
    void SyncInitialValues(UKNStatsComponent* InStatsComponent);
#pragma endregion 외부 제어 인터페이스

#pragma region 내부 콜백 함수
private:
    /**
     * @brief OnHealthChanged 델리게이트 콜백입니다.
     * @param Current 현재 체력
     * @param Max     최대 체력
     */
    UFUNCTION()
    void OnHealthChangedCallback(float Current, float Max);

    /**
     * @brief OnStaminaChanged 델리게이트 콜백입니다.
     * @param Current 현재 스태미나
     * @param Max     최대 스태미나
     */
    UFUNCTION()
    void OnStaminaChangedCallback(float Current, float Max);

    /**
     * @brief OnChronosChanged 델리게이트 콜백입니다.
     * @param Current 현재 크로노스
     * @param Max     최대 크로노스
     */
    UFUNCTION()
    void OnChronosChangedCallback(float Current, float Max);

    /**
     * @brief OnOverclockPointChanged 델리게이트 콜백입니다.
     * @param Current 현재 오버클럭 포인트
     * @param Max     최대 오버클럭 포인트
     */
    UFUNCTION()
    void OnOverclockPointChangedCallback(float Current, float Max);

    /**
     * @brief OnWeaponStateChanged 델리게이트 콜백입니다.
     * @param bIsDrawn true = 발도, false = 납도
     */
    UFUNCTION()
    void OnWeaponStateChangedCallback(bool bIsDrawn);
#pragma endregion 내부 콜백 함수

#pragma region UMG 바인딩 스탯 바
protected:
    /** @brief 플레이어 체력 바 (WBP_HealthBar) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UKNProgressBarWidget> HealthBar_Widget = nullptr;

    /** @brief 플레이어 스태미나 바 (WBP_StaminaBar) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UKNProgressBarWidget> StaminaBar_Widget = nullptr;

    /** @brief 플레이어 크로노스 바 (WBP_ChronosBar) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UKNProgressBarWidget> ChronosBar_Widget = nullptr;
#pragma endregion UMG 바인딩 스탯 바

#pragma region UMG 바인딩 오버클럭
protected:
    /** @brief 오버클럭 링 게이지 + 레벨 아이콘 그룹 (WBP_OverclockGroup) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UKNOverclockGroupWidget> OverclockGroup_Widget = nullptr;
#pragma endregion UMG 바인딩 오버클럭

#pragma region UMG 바인딩 발도/납도
protected:
    /**
     * @brief 발도/납도 상태 아이콘 위젯입니다.
     * @details 블루프린트 위젯 이름이 반드시 WeaponState_Widget 이어야 합니다.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UKNWeaponStateWidget> WeaponState_Widget = nullptr;
#pragma endregion UMG 바인딩 발도/납도

#pragma region UMG 바인딩 스킬 덱
protected:
    /** @brief 오버클럭 스킬 1 아이콘 (WBP_SkillIcon_Lv1) — 미구현 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UKNDynamicIconWidget> SkillIcon_Lv1_Widget = nullptr;

    /** @brief 오버클럭 스킬 2 아이콘 (WBP_SkillIcon_Lv2) — 미구현 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UKNDynamicIconWidget> SkillIcon_Lv2_Widget = nullptr;

    /** @brief 오버클럭 스킬 3 아이콘 (WBP_SkillIcon_Lv3) — 미구현 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UKNDynamicIconWidget> SkillIcon_Lv3_Widget = nullptr;
#pragma endregion UMG 바인딩 스킬 덱

#pragma region UMG 바인딩 보스
protected:
    /** @brief 보스 체력 바 (WBP_BossHealthBar) — 미구현 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UKNProgressBarWidget> BossHealthBar_Widget = nullptr;
#pragma endregion UMG 바인딩 보스
};
