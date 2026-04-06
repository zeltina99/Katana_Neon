// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Base/KNUserWidgetBase.h"
#include "KNDynamicIconWidget.generated.h"

#pragma region 전방 선언
class UImage;
class UMaterialInterface;
class UMaterialInstanceDynamic;
#pragma endregion 전방 선언
/**
 * @file    KNDynamicIconWidget.h
 * @class   UKNDynamicIconWidget
 * @brief   오버클럭 포인트 구간에 따라 머터리얼 인스턴스를 교체하는 범용 아이콘 위젯입니다.
 * @details SRP 원칙에 따라 "포인트 구간을 받아 알맞은 머터리얼로 교체한다"는
 *          단 하나의 책임만 가집니다.
 *          0~99: LV0 (숫자 없음, 흰색 배경)
 *          100~199: LV1 (숫자 1, 노란색)
 *          200~299: LV2 (숫자 2, 파란색)
 *          300: LV3 (숫자 3, 빨간색)
 */
UCLASS()
class KATANANEON_API UKNDynamicIconWidget : public UKNUserWidgetBase
{
	GENERATED_BODY()
	
#pragma region 위젯 생명주기 오버라이드
protected:
    /**
     * @brief 위젯 초기화 시 기본 머터리얼(LV0)을 적용합니다.
     */
    virtual void NativeConstruct() override;
#pragma endregion 위젯 생명주기 오버라이드

#pragma region 외부 제어 인터페이스
public:
    /**
     * @brief 오버클럭 포인트를 받아 구간에 맞는 머터리얼 인스턴스로 교체합니다.
     * @param InPoint 현재 오버클럭 포인트 (0.0 ~ 300.0)
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|UI|DynamicIcon")
    void SetOverclockPoint(float InPoint);
#pragma endregion 외부 제어 인터페이스

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 0~99 구간 머터리얼 인스턴스 (숫자 없음, 흰색 배경).
     * @details 에디터에서 MI_UI_OverClock_LV0 을 할당합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|DynamicIcon|Config")
    TObjectPtr<UMaterialInterface> MaterialLv0 = nullptr;

    /**
     * @brief 100~199 구간 머터리얼 인스턴스 (숫자 1, 노란색).
     * @details 에디터에서 MI_UI_OverClock_LV1 을 할당합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|DynamicIcon|Config")
    TObjectPtr<UMaterialInterface> MaterialLv1 = nullptr;

    /**
     * @brief 200~299 구간 머터리얼 인스턴스 (숫자 2, 파란색).
     * @details 에디터에서 MI_UI_OverClock_LV2 을 할당합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|DynamicIcon|Config")
    TObjectPtr<UMaterialInterface> MaterialLv2 = nullptr;

    /**
     * @brief 300 구간 머터리얼 인스턴스 (숫자 3, 빨간색).
     * @details 에디터에서 MI_UI_OverClock_LV3 을 할당합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|DynamicIcon|Config")
    TObjectPtr<UMaterialInterface> MaterialLv3 = nullptr;

    /**
     * @brief 각 구간의 시작 포인트 임계값 배열입니다.
     * @details 에디터에서 수정하여 구간 경계를 자유롭게 조정합니다.
     *          기본값: [0] = 100.0, [1] = 200.0, [2] = 300.0
     *          하드코딩 없이 DataTable의 오버클럭 임계값과 동일하게 맞출 수 있습니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|DynamicIcon|Config")
    TArray<float> StageThresholds = { 100.0f, 200.0f, 300.0f };

    /**
     * @brief NumberIndex 전환 애니메이션 총 재생 시간 (초).
     * @details 값이 클수록 숫자가 천천히 전환됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|DynamicIcon|Config")
    float TransitionDuration = 0.4f;

    /**
     * @brief NumberIndex 전환 애니메이션 틱 간격 (초).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|DynamicIcon|Config")
    float TransitionTickInterval = 0.016f;

    /**
     * @brief 머터리얼의 NumberIndex 스칼라 파라미터 이름입니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|DynamicIcon|Config")
    FName NumberIndexParamName = FName("NumberIndex");

    /**
     * @brief 머터리얼의 Show texture 스칼라 파라미터 이름입니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|UI|DynamicIcon|Config")
    FName ShowTextureParamName = FName("Show texture");
#pragma endregion 에디터 설정 데이터

#pragma region UMG 바인딩
protected:
    /**
     * @brief 머터리얼을 표시할 이미지 위젯입니다.
     * @details 블루프린트 위젯 이름이 반드시 Image_Icon 이어야 합니다.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> Image_Icon = nullptr;
#pragma endregion UMG 바인딩

#pragma region 런타임 상태
private:
    /** @brief 현재 적용된 동적 머터리얼 인스턴스 */
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial = nullptr;

    /** @brief 현재 단계 인덱스 캐시 */
    int32 CachedStageIndex = -1;

    /** @brief 전환 애니메이션 시작 NumberIndex */
    float TransitionFromIndex = 0.0f;

    /** @brief 전환 애니메이션 목표 NumberIndex */
    float TransitionToIndex = 0.0f;

    /** @brief 전환 애니메이션 경과 시간 */
    float TransitionElapsed = 0.0f;

    /** @brief 전환 애니메이션 진행 여부 */
    bool bIsTransitioning = false;

    /** @brief 전환 애니메이션 타이머 핸들 */
    FTimerHandle TransitionTimerHandle;
#pragma endregion 런타임 상태

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief 포인트 값으로 단계 인덱스를 계산합니다.
     * @param InPoint 현재 오버클럭 포인트
     * @return 0 = LV0, 1 = LV1, 2 = LV2, 3 = LV3
     */
    int32 CalculateStageIndex(float InPoint) const;

    /**
     * @brief 단계 인덱스에 해당하는 머터리얼을 Image_Icon에 적용합니다.
     * @param InStageIndex 적용할 단계 인덱스
     */
    void ApplyMaterialForStage(int32 InStageIndex);

    /**
     * @brief 단계 인덱스에 해당하는 머터리얼을 반환합니다.
     * @param InStageIndex 조회할 단계 인덱스
     * @return 해당 머터리얼 인터페이스 포인터
     */
    UMaterialInterface* GetMaterialForStage(int32 InStageIndex) const;

    /**
     * @brief 단계 인덱스에 해당하는 NumberIndex 값을 반환합니다.
     * @details LV0 = -1(미사용), LV1 = 0.0, LV2 = 0.5, LV3 = 1.0
     */
    float GetNumberIndexForStage(int32 InStageIndex) const;

    /**
     * @brief NumberIndex 전환 애니메이션을 시작합니다.
     * @param FromIndex 시작 NumberIndex
     * @param ToIndex   목표 NumberIndex
     */
    void StartTransition(float FromIndex, float ToIndex);

    /** @brief 전환 애니메이션 틱 콜백 */
    UFUNCTION()
    void OnTransitionTick();
#pragma endregion 내부 헬퍼 함수
};
