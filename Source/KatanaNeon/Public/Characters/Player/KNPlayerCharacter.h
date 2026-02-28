// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Base/KNCharacterBase.h"
#include "KNPlayerCharacter.generated.h"

#pragma region 전방 선언
class USpringArmComponent;
class UCameraComponent;
class UKNInputDataConfig;
struct FInputActionValue;
#pragma endregion 전방 선언

/**
 * @class  AKNPlayerCharacter
 * @brief  유저가 직접 조종하는 사이버펑크 검객 플레이어 클래스입니다.
 * @details AKNCharacterBase의 GAS 시스템을 상속받으며, 카메라 제어 및
 * 향상된 입력 시스템(Enhanced Input) 바인딩을 추가로 담당합니다.
 */
UCLASS()
class KATANANEON_API AKNPlayerCharacter : public AKNCharacterBase
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    /** @brief 캐릭터 기본 생성자. 카메라와 스프링 암을 초기화합니다. */
    AKNPlayerCharacter();

protected:
    /** @brief 로컬 플레이어가 컨트롤러를 소유할 때 호출되어 매핑 컨텍스트(IMC)를 등록합니다. */
    virtual void PawnClientRestart() override;

    /** @brief 데이터 에셋(InputDataConfig)을 읽어 입력 액션을 바인딩합니다. */
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
#pragma endregion 기본 생성자 및 초기화

#pragma region 카메라 컴포넌트
private:
    /** @brief 캐릭터 뒤에서 카메라를 지지해주는 붐 암(Boom Arm)입니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraBoom = nullptr;

    /** @brief 플레이어의 3인칭 시점을 담당하는 추적 카메라입니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KatanaNeon|Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera = nullptr;
#pragma endregion 카메라 컴포넌트

#pragma region 데이터 주도형 입력 설정
protected:
    /**
     * @brief 플레이어의 모든 입력 액션과 매핑 정보를 담고 있는 통합 데이터 에셋입니다.
     * @details 기획자가 에디터에서 생성한 DataAsset을 할당하여 코딩 없이 키를 변경합니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input")
    TObjectPtr<UKNInputDataConfig> InputDataConfig = nullptr;
#pragma endregion 데이터 주도형 입력 설정

#pragma region 입력 콜백 함수
protected:
    // ── 기본 이동 및 시점 ──
    /** @brief WASD 및 좌스틱 이동 처리 */
    void Input_Move(const FInputActionValue& Value);
    /** @brief 마우스 및 우스틱 시점 회전 처리 */
    void Input_Look(const FInputActionValue& Value);
    /** @brief 점프 액션 시작 */
    void Input_JumpStart(const FInputActionValue& Value);
    /** @brief 점프 액션 종료 */
    void Input_JumpStop(const FInputActionValue& Value);

    // ── 전투 및 스킬 (추후 GAS 어빌리티와 연동될 뼈대) ──
    void Input_Attack(const FInputActionValue& Value);
    void Input_Parry(const FInputActionValue& Value);
    void Input_Dash(const FInputActionValue& Value);
    void Input_Chronos(const FInputActionValue& Value);

    // ── 오버클럭 스킬 ──
    void Input_OverclockLv1(const FInputActionValue& Value);
    void Input_OverclockLv2(const FInputActionValue& Value);
    void Input_OverclockLv3(const FInputActionValue& Value);
#pragma endregion 입력 콜백 함수
};
