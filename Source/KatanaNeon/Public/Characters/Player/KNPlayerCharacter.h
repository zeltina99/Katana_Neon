// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Base/KNCharacterBase.h"
#include "KNPlayerCharacter.generated.h"

#pragma region 전방 선언
class USpringArmComponent;
class UCameraComponent;
class UKNStatsComponent;
#pragma endregion 전방 선언

/**
 * @class  AKNPlayerCharacter
 * @brief  유저가 직접 조종하는 사이버펑크 검객 플레이어 클래스입니다.
 * @details AKNCharacterBase의 GAS 시스템을 상속받으며, 카메라 제어,
 * 플레이어 전용 스탯 매니저(StatsComponent), 그리고 Enhanced Input 바인딩을 담당합니다.
 */
UCLASS()
class KATANANEON_API AKNPlayerCharacter : public AKNCharacterBase
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    /** @brief 캐릭터 기본 생성자. 카메라와 스프링 암, 플레이어 전용 컴포넌트를 초기화합니다. */
    AKNPlayerCharacter();

protected:
    /** @brief 게임 시작 시 플레이어 전용 스탯 컴포넌트를 초기화합니다. */
    virtual void BeginPlay() override;
#pragma endregion 기본 생성자 및 초기화

#pragma region 카메라 세팅
protected:
    /**
     * @brief 스프링 암의 소켓 오프셋입니다.
     * @details Z값을 올리면 카메라가 캐릭터 머리 위로 올라가 내려다보는 앵글이 됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Camera|Transform")
    FVector CameraSocketOffset = FVector(0.0f, 0.0f, 150.0f);

    /**
     * @brief 카메라 상하 회전(Pitch) 최소 제한 각도 (바닥을 내려다보는 한계)
     * @details -89가 언리얼 엔진의 수직 하단입니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Camera|Limit", meta = (ClampMin = "-89.0", ClampMax = "0.0"))
    float MinCameraPitch = -60.0f;

    /**
     * @brief 카메라 상하 회전(Pitch) 최대 제한 각도 (하늘을 올려다보는 한계)
     * @details 액션 게임에서는 하늘을 너무 많이 보지 못하게 제한하는 것이 좋습니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|Camera|Limit", meta = (ClampMin = "0.0", ClampMax = "89.0"))
    float MaxCameraPitch = 20.0f;
#pragma endregion 카메라 세팅

#pragma region 컴포넌트
private:
    /** @brief 캐릭터 뒤에서 카메라를 지지해주는 붐 암(Boom Arm)입니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraBoom = nullptr;

    /** @brief 플레이어의 3인칭 시점을 담당하는 추적 카메라입니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera = nullptr;

    /** @brief 플레이어 전용 기획 데이터(DataTable) 연동 스탯 매니저 컴포넌트입니다. (적은 사용하지 않음) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|GAS", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UKNStatsComponent> StatsComponent = nullptr;
#pragma endregion 컴포넌트
};
