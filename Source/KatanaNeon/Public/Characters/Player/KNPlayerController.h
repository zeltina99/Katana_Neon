// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "KNPlayerController.generated.h"

#pragma region 전방 선언
class UKNInputDataConfig;
class UKNMainHUDWidget;
struct FInputActionValue;
#pragma endregion 전방 선언

/**
 * @class  AKNPlayerController
 * @brief  플레이어의 하드웨어 입력(키보드, 마우스)을 수신하고 캐릭터(Pawn)에게 명령을 전달하는 두뇌 클래스입니다.
 * @details 단일 책임 원칙(SRP)에 따라 캐릭터(AKNPlayerCharacter)에서 입력 처리 로직을 완전히 분리했습니다.
 */
UCLASS()
class KATANANEON_API AKNPlayerController : public APlayerController
{
	GENERATED_BODY()
	
#pragma region 기본 셋업
protected:
    /** @brief 컨트롤러가 캐릭터(Pawn)에 빙의(Possess)될 때 매핑 컨텍스트(IMC)를 등록합니다. */
    virtual void OnPossess(APawn* aPawn) override;

    /** @brief 데이터 에셋(InputDataConfig)을 읽어 입력 액션을 바인딩합니다. */
    virtual void SetupInputComponent() override;
#pragma endregion 기본 셋업

#pragma region 데이터 주도형 입력 설정
protected:
    /** @brief 플레이어의 모든 입력 액션과 매핑 정보를 담고 있는 통합 데이터 에셋입니다. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input")
    TObjectPtr<UKNInputDataConfig> InputDataConfig = nullptr;
#pragma endregion 데이터 주도형 입력 설정

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief GAS 어빌리티 실행 코드를 단일화한 헬퍼 함수 (리뷰 반영).
     * @details 중복 캐스팅과 컨테이너 생성 비용을 한 곳으로 모아 최적화했습니다.
     * @param Tag 활성화하고자 하는 어빌리티의 네이티브 태그
     */
    void TryActivateAbilityByTag(const FGameplayTag& Tag);
    /**
     * @brief 게임플레이 태그를 이용해 실행 중인 특정 어빌리티를 강제 종료(Cancel)합니다.
     * @param Tag 종료할 어빌리티에 매핑된 식별 태그
     */
    void CancelAbilityByTag(const FGameplayTag& Tag);
#pragma endregion 내부 헬퍼 함수

#pragma region HUD 관리
public:
    /** @brief 현재 화면에 표시 중인 메인 HUD 위젯을 반환합니다. */
    UFUNCTION(BlueprintPure, Category = "KatanaNeon|UI|HUD")
    UKNMainHUDWidget* GetMainHUD() const { return MainHUDWidget; }

protected:
    /**
     * @brief 메인 HUD 위젯 클래스입니다.
     * @details 에디터에서 WBP_MainHUD 를 할당합니다.
     */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|UI|HUD")
    TSubclassOf<UKNMainHUDWidget> MainHUDWidgetClass = nullptr;

private:
    /** @brief 생성된 메인 HUD 위젯 인스턴스 */
    UPROPERTY(Transient)
    TObjectPtr<UKNMainHUDWidget> MainHUDWidget = nullptr;

    /**
     * @brief HUD 위젯을 생성하고 StatsComponent와 연결합니다.
     */
    void CreateAndInitHUD();
#pragma endregion HUD 관리

#pragma region 입력 콜백 함수
protected:
    // ── 이동 및 시점 ──
    void Input_Move(const FInputActionValue& Value);
    void Input_Look(const FInputActionValue& Value);
    void Input_JumpStart(const FInputActionValue& Value);
    void Input_JumpStop(const FInputActionValue& Value);
    void Input_SprintToggle(const FInputActionValue&);
    void Input_LockOn(const FInputActionValue& Value);

    // ── 전투 및 스킬 (GAS 어빌리티 연동) ──
    void Input_Attack(const FInputActionValue& Value);
    void Input_HeavyAttack(const FInputActionValue& Value);
    void Input_Parry(const FInputActionValue& Value);
    void Input_Dash(const FInputActionValue& Value);
    void Input_Chronos(const FInputActionValue& Value);
    void Input_ToggleStance(const FInputActionValue& Value);

    // ── 오버클럭 스킬 ──
    void Input_OverclockLv1(const FInputActionValue& Value);
    void Input_OverclockLv2(const FInputActionValue& Value);
    void Input_OverclockLv3(const FInputActionValue& Value);

    // ── 유틸리티 및 상호작용 ──
    void Input_Interact(const FInputActionValue& Value);
    void Input_Potion(const FInputActionValue& Value);
    void Input_PauseMenu(const FInputActionValue& Value);
#pragma endregion 입력 콜백 함수
};
