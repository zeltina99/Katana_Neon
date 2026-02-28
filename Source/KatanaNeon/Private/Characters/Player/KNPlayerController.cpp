// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/KNPlayerController.h"
#include "Characters/Base/KNCharacterBase.h"
#include "Data/Assets/KNInputDataConfig.h"
#include "AbilitySystemComponent.h"
#include "GAS/Tags/KNStatsTags.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"

#pragma region 셋업 및 바인딩 구현
void AKNPlayerController::OnPossess(APawn* aPawn)
{
    Super::OnPossess(aPawn);

    // 로컬 플레이어 시스템에 매핑 컨텍스트(IMC) 등록
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (InputDataConfig && InputDataConfig->DefaultMappingContext)
        {
            Subsystem->AddMappingContext(InputDataConfig->DefaultMappingContext, 0);
        }
    }
}

void AKNPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EIC || !InputDataConfig) return;

    // ── 이동 및 시점 ──
    if (InputDataConfig->MoveAction) EIC->BindAction(InputDataConfig->MoveAction, ETriggerEvent::Triggered, this, &AKNPlayerController::Input_Move);
    if (InputDataConfig->LookAction) EIC->BindAction(InputDataConfig->LookAction, ETriggerEvent::Triggered, this, &AKNPlayerController::Input_Look);

    if (InputDataConfig->JumpAction)
    {
        EIC->BindAction(InputDataConfig->JumpAction, ETriggerEvent::Started, this, &AKNPlayerController::Input_JumpStart);
        EIC->BindAction(InputDataConfig->JumpAction, ETriggerEvent::Completed, this, &AKNPlayerController::Input_JumpStop);
    }

    // ── 전투 및 어빌리티 ──
    if (InputDataConfig->AttackAction) EIC->BindAction(InputDataConfig->AttackAction, ETriggerEvent::Started, this, &AKNPlayerController::Input_Attack);
    if (InputDataConfig->DashAction) EIC->BindAction(InputDataConfig->DashAction, ETriggerEvent::Started, this, &AKNPlayerController::Input_Dash);

    // (기타 액션들도 동일한 패턴으로 바인딩 생략...)
}
#pragma endregion 셋업 및 바인딩 구현 끝

#pragma region 입력 콜백 함수 구현
void AKNPlayerController::Input_Move(const FInputActionValue& Value)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    const FVector2D MovementVector = Value.Get<FVector2D>();
    const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);

    // 두뇌(Controller)가 육체(Pawn)에게 이동 명령을 내립니다.
    ControlledPawn->AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), MovementVector.Y);
    ControlledPawn->AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), MovementVector.X);
}

void AKNPlayerController::Input_Look(const FInputActionValue& Value)
{
    const FVector2D LookVector = Value.Get<FVector2D>();
    AddYawInput(LookVector.X);
    AddPitchInput(LookVector.Y);
}

void AKNPlayerController::Input_JumpStart(const FInputActionValue&)
{
    if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
    {
        ControlledCharacter->Jump();
    }
}

void AKNPlayerController::Input_JumpStop(const FInputActionValue&)
{
    if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
    {
        ControlledCharacter->StopJumping();
    }
}

// ── GAS 어빌리티 호출 로직 (SRP 완벽 준수) ──
void AKNPlayerController::Input_Attack(const FInputActionValue&)
{
    if (AKNCharacterBase* ControlledCharacter = Cast<AKNCharacterBase>(GetPawn()))
    {
        if (UAbilitySystemComponent* ASC = ControlledCharacter->GetAbilitySystemComponent())
        {
            ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(KatanaNeon::Ability::Combat::Attack));
        }
    }
}

void AKNPlayerController::Input_Dash(const FInputActionValue&)
{
    if (AKNCharacterBase* ControlledCharacter = Cast<AKNCharacterBase>(GetPawn()))
    {
        if (UAbilitySystemComponent* ASC = ControlledCharacter->GetAbilitySystemComponent())
        {
            ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(KatanaNeon::Ability::Combat::Dash));
        }
    }
}

// (나머지 Parry, Chronos, 빈 함수들도 동일한 방식으로 ASC를 꺼내어 호출)
void AKNPlayerController::Input_Parry(const FInputActionValue&) {}
void AKNPlayerController::Input_Chronos(const FInputActionValue&) {}
void AKNPlayerController::Input_OverclockLv1(const FInputActionValue&) {}
void AKNPlayerController::Input_OverclockLv2(const FInputActionValue&) {}
void AKNPlayerController::Input_OverclockLv3(const FInputActionValue&) {}
void AKNPlayerController::Input_SprintStart(const FInputActionValue&) {}
void AKNPlayerController::Input_SprintStop(const FInputActionValue&) {}
void AKNPlayerController::Input_LockOn(const FInputActionValue&) {}
void AKNPlayerController::Input_Interact(const FInputActionValue&) {}
void AKNPlayerController::Input_Potion(const FInputActionValue&) {}
void AKNPlayerController::Input_PauseMenu(const FInputActionValue&) {}
#pragma endregion 입력 콜백 함수 구현 끝