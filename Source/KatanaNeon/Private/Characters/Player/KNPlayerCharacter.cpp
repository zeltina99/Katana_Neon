// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/KNPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/Components/KNStatsComponent.h"
#include "Components/StaticMeshComponent.h" 
#include "GAS/Tags/KNStatsTags.h"
#include "Components/KNChronosSphereComponent.h"

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

    // ── 무기 컴포넌트 생성 및 초기 부착 (게임 시작 시 납도 상태) ──
    KatanaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KatanaMesh"));
    KatanaMesh->SetupAttachment(GetMesh(), TEXT("SheathSocket_L")); // 왼손 허리에 납도
    KatanaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 무기 자체 충돌판정 끔(최적화)

    SheathMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SheathMesh"));
    SheathMesh->SetupAttachment(GetMesh(), TEXT("SheathSocket_L"));
    SheathMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ChronosSphereComponent = CreateDefaultSubobject<UKNChronosSphereComponent>(TEXT("ChronosSphereComponent"));
    ChronosSphereComponent->SetupAttachment(GetRootComponent());
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

#pragma region 무기 스왑 제어 구현
void AKNPlayerCharacter::EquipWeapon()
{
    if (KatanaMesh)
    {
        // 1. 오른손 소켓으로 물리적 부착 (스케일은 유지, 위치/회전은 소켓에 스냅)
        KatanaMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket_R"));
        bIsWeaponDrawn = true;

        // 2. GAS에 '무기 들었음' 태그 부여
        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
        {
            ASC->AddLooseGameplayTag(KatanaNeon::State::Combat::WeaponDrawn);
        }
    }
}

void AKNPlayerCharacter::UnequipWeapon()
{
    if (KatanaMesh)
    {
        // 1. 왼쪽 칼집 소켓으로 물리적 부착 귀환
        KatanaMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("SheathSocket_L"));
        bIsWeaponDrawn = false;

        // 2. GAS에서 '무기 들었음' 태그 제거
        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
        {
            ASC->RemoveLooseGameplayTag(KatanaNeon::State::Combat::WeaponDrawn);
        }
    }
}
#pragma endregion 무기 스왑 제어 구현

#pragma region 락온 시스템 구현
void AKNPlayerCharacter::SetLockOnState(bool bNewLockOn)
{
    bIsLockOn = bNewLockOn;

    // 포인터 유효성 검사를 통한 방어적 프로그래밍
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        if (bIsLockOn)
        {
            // [락온 활성화] 타겟을 바라보며 8방향 게걸음 스텝 활성화
            MoveComp->bOrientRotationToMovement = false;
            bUseControllerRotationYaw = true;
        }
        else
        {
            // [락온 해제] 입력한 방향으로 몸을 돌리며 1D 전진 활성화
            bUseControllerRotationYaw = false;
            MoveComp->bOrientRotationToMovement = true;
        }
    }
}
#pragma endregion 락온 시스템 구현