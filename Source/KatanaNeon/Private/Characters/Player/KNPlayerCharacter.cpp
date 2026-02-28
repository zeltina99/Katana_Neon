// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/KNPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Data/Assets/KNInputDataConfig.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#pragma region 기본 생성자 및 초기화 구현
AKNPlayerCharacter::AKNPlayerCharacter()
{
    // 액션 게임의 부드러운 회전을 위해 컨트롤러 회전 사용을 끕니다.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // 캐릭터가 이동하는 방향을 자연스럽게 바라보도록 설정합니다.
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    }

    // 카메라 붐 생성 및 설정
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f; // 카메라 기본 거리
    CameraBoom->bUsePawnControlRotation = true; // 마우스 회전에 따라 붐 암이 회전하도록 설정

    // 팔로우 카메라 생성 및 설정
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false; // 카메라는 붐 암의 회전을 그대로 따라감
}

void AKNPlayerCharacter::PawnClientRestart()
{
    Super::PawnClientRestart();

    // 로컬 플레이어의 Enhanced Input 서브시스템에 매핑 컨텍스트(IMC)를 안전하게 등록합니다.
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            if (InputDataConfig && InputDataConfig->DefaultMappingContext)
            {
                // 최우선 순위(0)로 매핑 컨텍스트 적용
                Subsystem->AddMappingContext(InputDataConfig->DefaultMappingContext, 0);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[KNPlayerCharacter] InputDataConfig 또는 DefaultMappingContext가 설정되지 않았습니다."));
            }
        }
    }
}
#pragma endregion 기본 생성자 및 초기화 구현 끝

#pragma region 데이터 주도형 입력 바인딩 구현
void AKNPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 구형 입력 시스템이 아닌 Enhanced Input Component인지 안전하게 캐스팅합니다.
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // 데이터 에셋 누락 방어
        if (!InputDataConfig) return;

        // 1. 이동 및 시점 바인딩
        if (InputDataConfig->MoveAction)
        {
            EnhancedInputComponent->BindAction(InputDataConfig->MoveAction, ETriggerEvent::Triggered, this, &AKNPlayerCharacter::Input_Move);
        }
        if (InputDataConfig->LookAction)
        {
            EnhancedInputComponent->BindAction(InputDataConfig->LookAction, ETriggerEvent::Triggered, this, &AKNPlayerCharacter::Input_Look);
        }
        if (InputDataConfig->JumpAction)
        {
            EnhancedInputComponent->BindAction(InputDataConfig->JumpAction, ETriggerEvent::Started, this, &AKNPlayerCharacter::Input_JumpStart);
            EnhancedInputComponent->BindAction(InputDataConfig->JumpAction, ETriggerEvent::Completed, this, &AKNPlayerCharacter::Input_JumpStop);
        }

        // 2. 전투 액션 바인딩
        if (InputDataConfig->AttackAction) { EnhancedInputComponent->BindAction(InputDataConfig->AttackAction, ETriggerEvent::Started, this, &AKNPlayerCharacter::Input_Attack); }
        if (InputDataConfig->ParryAction) { EnhancedInputComponent->BindAction(InputDataConfig->ParryAction, ETriggerEvent::Started, this, &AKNPlayerCharacter::Input_Parry); }
        if (InputDataConfig->DashAction) { EnhancedInputComponent->BindAction(InputDataConfig->DashAction, ETriggerEvent::Started, this, &AKNPlayerCharacter::Input_Dash); }

        // 3. 특수 스킬 바인딩
        if (InputDataConfig->ChronosAction) { EnhancedInputComponent->BindAction(InputDataConfig->ChronosAction, ETriggerEvent::Started, this, &AKNPlayerCharacter::Input_Chronos); }
        if (InputDataConfig->OverclockLv1Action) { EnhancedInputComponent->BindAction(InputDataConfig->OverclockLv1Action, ETriggerEvent::Started, this, &AKNPlayerCharacter::Input_OverclockLv1); }
        if (InputDataConfig->OverclockLv2Action) { EnhancedInputComponent->BindAction(InputDataConfig->OverclockLv2Action, ETriggerEvent::Started, this, &AKNPlayerCharacter::Input_OverclockLv2); }
        if (InputDataConfig->OverclockLv3Action) { EnhancedInputComponent->BindAction(InputDataConfig->OverclockLv3Action, ETriggerEvent::Started, this, &AKNPlayerCharacter::Input_OverclockLv3); }
    }
}
#pragma endregion 데이터 주도형 입력 바인딩 구현 끝

#pragma region 입력 콜백 함수 구현
void AKNPlayerCharacter::Input_Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        // 카메라 시점 기준으로 앞/뒤, 좌/우 이동 방향을 계산합니다.
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AKNPlayerCharacter::Input_Look(const FInputActionValue& Value)
{
    const FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void AKNPlayerCharacter::Input_JumpStart(const FInputActionValue& Value) { Jump(); }
void AKNPlayerCharacter::Input_JumpStop(const FInputActionValue& Value) { StopJumping(); }

// ── 추후 GAS 어빌리티(SendGameplayEventToActor) 연동될 위치 ──
void AKNPlayerCharacter::Input_Attack(const FInputActionValue& Value) { /* TODO: GAS 공격 어빌리티 호출 */ }
void AKNPlayerCharacter::Input_Parry(const FInputActionValue& Value) { /* TODO: GAS 패링 어빌리티 호출 */ }
void AKNPlayerCharacter::Input_Dash(const FInputActionValue& Value) { /* TODO: GAS 대시 어빌리티 호출 */ }
void AKNPlayerCharacter::Input_Chronos(const FInputActionValue& Value) { /* TODO: GAS 크로노스 토글 호출 */ }
void AKNPlayerCharacter::Input_OverclockLv1(const FInputActionValue& Value) { /* TODO: 오버클럭 1단계 어빌리티 호출 */ }
void AKNPlayerCharacter::Input_OverclockLv2(const FInputActionValue& Value) { /* TODO: 오버클럭 2단계 어빌리티 호출 */ }
void AKNPlayerCharacter::Input_OverclockLv3(const FInputActionValue& Value) { /* TODO: 오버클럭 3단계 어빌리티 호출 */ }
#pragma endregion 입력 콜백 함수 구현 끝