// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/KNPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/Components/KNStatsComponent.h"

#pragma region 기본 생성자 및 초기화 구현
AKNPlayerCharacter::AKNPlayerCharacter()
{
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    }

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    CameraBoom->SocketOffset = CameraSocketOffset;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    StatsComponent = CreateDefaultSubobject<UKNStatsComponent>(TEXT("StatsComponent"));
}

void AKNPlayerCharacter::BeginPlay()
{
    // 부모 클래스의 GAS 셋업(InitAbilityActorInfo) 호출
    Super::BeginPlay();

    if (StatsComponent && AbilitySystemComponent)
    {
        StatsComponent->InitializeStatComponent(AbilitySystemComponent);
    }

    // 카메라 상하(Pitch) 회전 제한 적용
    // bUsePawnControlRotation이 true일 때, 카메라 회전은 PlayerCameraManager가 통제합니다.
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->ViewPitchMin = MinCameraPitch;
            PC->PlayerCameraManager->ViewPitchMax = MaxCameraPitch;
        }
    }
}
#pragma endregion 기본 생성자 및 초기화 구현