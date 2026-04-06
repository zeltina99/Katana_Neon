// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/KNPlayerController.h"
#include "Characters/Base/KNCharacterBase.h"
#include "Characters/Player/KNPlayerCharacter.h"
#include "Data/Assets/KNInputDataConfig.h"
#include "AbilitySystemComponent.h"
#include "GAS/Tags/KNStatsTags.h"
#include "GAS/Components/KNStatsComponent.h" 

#include "UI/Main/KNMainHUDWidget.h"  
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

    CreateAndInitHUD();
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

    if (InputDataConfig->SprintAction)
    {
        EIC->BindAction(InputDataConfig->SprintAction, ETriggerEvent::Started, this, &AKNPlayerController::Input_SprintToggle);
    }

    // ── 전투 및 어빌리티 ──
    if (InputDataConfig->AttackAction)      EIC->BindAction(InputDataConfig->AttackAction, ETriggerEvent::Started, this, &AKNPlayerController::Input_Attack);
    if (InputDataConfig->HeavyAttackAction) EIC->BindAction(InputDataConfig->HeavyAttackAction, ETriggerEvent::Started, this, &AKNPlayerController::Input_HeavyAttack);

    if (InputDataConfig->DashAction)        EIC->BindAction(InputDataConfig->DashAction, ETriggerEvent::Started, this, &AKNPlayerController::Input_Dash);
    if (InputDataConfig->ParryAction)       EIC->BindAction(InputDataConfig->ParryAction, ETriggerEvent::Started, this, &AKNPlayerController::Input_Parry);
    if (InputDataConfig->ChronosAction)     EIC->BindAction(InputDataConfig->ChronosAction, ETriggerEvent::Started, this, &AKNPlayerController::Input_Chronos);
    if (InputDataConfig->ToggleStanceAction)EIC->BindAction(InputDataConfig->ToggleStanceAction, ETriggerEvent::Started, this, &AKNPlayerController::Input_ToggleStance);

    // ── 오버클럭 ──
    if (InputDataConfig->OverclockLv1Action) EIC->BindAction(InputDataConfig->OverclockLv1Action, ETriggerEvent::Started, this, &AKNPlayerController::Input_OverclockLv1);
    if (InputDataConfig->OverclockLv2Action) EIC->BindAction(InputDataConfig->OverclockLv2Action, ETriggerEvent::Started, this, &AKNPlayerController::Input_OverclockLv2);
    if (InputDataConfig->OverclockLv3Action) EIC->BindAction(InputDataConfig->OverclockLv3Action, ETriggerEvent::Started, this, &AKNPlayerController::Input_OverclockLv3);

    // ── 유틸리티 (추후 구현 시 바인딩) ──
    // if (InputDataConfig->InteractAction) ...
    // if (InputDataConfig->PotionAction) ...
    if (InputDataConfig->PauseMenuAction)    EIC->BindAction(InputDataConfig->PauseMenuAction, ETriggerEvent::Started, this, &AKNPlayerController::Input_PauseMenu);
}
#pragma endregion 셋업 및 바인딩 구현

#pragma region 입력 콜백 헬퍼 함수 구현
void AKNPlayerController::TryActivateAbilityByTag(const FGameplayTag& Tag)
{
    // 최적화: 7곳에서 반복되던 캐스팅과 컨테이너 생성을 여기서 단 1번만 수행합니다. (리뷰 7번)
    if (AKNCharacterBase* ControlledCharacter = Cast<AKNCharacterBase>(GetPawn()))
    {
        if (UAbilitySystemComponent* ASC = ControlledCharacter->GetAbilitySystemComponent())
        {
            ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(Tag));
        }
    }
}
void AKNPlayerController::CancelAbilityByTag(const FGameplayTag& Tag)
{
    // 최적화: 종료할 때도 여기서 컨테이너를 단 1번만 생성하여 처리합니다.
    if (AKNCharacterBase* ControlledCharacter = Cast<AKNCharacterBase>(GetPawn()))
    {
        if (UAbilitySystemComponent* ASC = ControlledCharacter->GetAbilitySystemComponent())
        {
            TArray<FGameplayAbilitySpec*> MatchingSpecs;
            ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
                FGameplayTagContainer(Tag),
                MatchingSpecs,
                /*bOnlyAbilitiesThatSatisfyTagRequirements=*/false);

            for (FGameplayAbilitySpec* Spec : MatchingSpecs)
            {
                if (Spec && Spec->IsActive())
                {
                    ASC->CancelAbilityHandle(Spec->Handle);
                }
            }
        }
    }
}
#pragma endregion 입력 콜백 헬퍼 함수 구현

void AKNPlayerController::CreateAndInitHUD()
{
    if (!MainHUDWidgetClass) return;

    // 이미 생성된 HUD가 있으면 중복 생성하지 않습니다.
    if (MainHUDWidget) return;

    MainHUDWidget = CreateWidget<UKNMainHUDWidget>(this, MainHUDWidgetClass);
    if (!MainHUDWidget) return;

    MainHUDWidget->AddToViewport();

    // 빙의한 캐릭터에서 StatsComponent를 찾아 HUD에 연결합니다.
    if (AKNPlayerCharacter* PlayerCharacter = Cast<AKNPlayerCharacter>(GetPawn()))
    {
        if (UKNStatsComponent* StatsComp =
            PlayerCharacter->FindComponentByClass<UKNStatsComponent>())
        {
            // 델리게이트 구독
            MainHUDWidget->InitHUD(StatsComp);

            // 구독 직후 현재값으로 초기 화면을 채웁니다.
            MainHUDWidget->SyncInitialValues(StatsComp);
        }
    }
}

#pragma region 입력 콜백 함수 구현
void AKNPlayerController::Input_Move(const FInputActionValue& Value)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    const FVector2D MovementVector = Value.Get<FVector2D>();
    const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);

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
    // 점프도 원래는 엔진 Jump()를 쓰지만, GAS로 짰기 때문에 태그로 호출하는 것이 맞습니다.
    TryActivateAbilityByTag(KatanaNeon::Ability::Movement::Jump);
}

void AKNPlayerController::Input_JumpStop(const FInputActionValue&)
{
    if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
    {
        ControlledCharacter->StopJumping();
    }
}

void AKNPlayerController::Input_SprintToggle(const FInputActionValue&)
{
    /** @brief Shift 토글 시 Sprint 활성/비활성을 전환합니다. */
    if (AKNCharacterBase* ControlledCharacter = Cast<AKNCharacterBase>(GetPawn()))
    {
        if (UAbilitySystemComponent* ASC = ControlledCharacter->GetAbilitySystemComponent())
        {
            TArray<FGameplayAbilitySpec*> MatchingSpecs;
            ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
                FGameplayTagContainer(KatanaNeon::Ability::Movement::Sprint),
                MatchingSpecs,
                /*bOnlyAbilitiesThatSatisfyTagRequirements=*/false);

            bool bIsActive = false;
            for (FGameplayAbilitySpec* Spec : MatchingSpecs)
            {
                if (Spec && Spec->IsActive())
                {
                    bIsActive = true;
                    break;
                }
            }

            bIsActive
                ? CancelAbilityByTag(KatanaNeon::Ability::Movement::Sprint)
                : TryActivateAbilityByTag(KatanaNeon::Ability::Movement::Sprint);
        }
    }
}

// ── GAS 어빌리티 호출 로직 (공통 헬퍼 사용으로 획기적 최적화) ──
void AKNPlayerController::Input_Attack(const FInputActionValue&)
{
    TryActivateAbilityByTag(KatanaNeon::Ability::Combat::Attack);
}

void AKNPlayerController::Input_HeavyAttack(const FInputActionValue&)
{
    // TODO: 강공격은 현재 콤보 공격 파생 로직으로 처리해야 하므로,
    // 나중에 KNAbilityComboAttack::RequestHeavyAttack()을 호출하는 로직이 필요합니다.
    // 지금은 예비용으로 비워둡니다.
}

void AKNPlayerController::Input_Dash(const FInputActionValue&)
{
    TryActivateAbilityByTag(KatanaNeon::Ability::Combat::Dash);
}

void AKNPlayerController::Input_Parry(const FInputActionValue&)
{
    TryActivateAbilityByTag(KatanaNeon::Ability::Combat::Parry);
}

void AKNPlayerController::Input_Chronos(const FInputActionValue&)
{
    TryActivateAbilityByTag(KatanaNeon::Ability::Skill::Chronos);
}

void AKNPlayerController::Input_ToggleStance(const FInputActionValue& Value)
{
    /** @brief Tab 키를 누르면 무기 토글 어빌리티(발도/납도 몽타주 재생)를 호출합니다. */
    TryActivateAbilityByTag(KatanaNeon::Ability::Combat::ToggleWeapon);
}

void AKNPlayerController::Input_OverclockLv1(const FInputActionValue&)
{
    UE_LOG(LogTemp, Warning, TEXT("[Controller] 1번 키 눌림! ASC에 Lv1 태그 전달 시도 중..."));
   
    /*if (AKNCharacterBase* ControlledCharacter = Cast<AKNCharacterBase>(GetPawn()))
    {
        if (UAbilitySystemComponent* ASC = ControlledCharacter->GetAbilitySystemComponent())
        {
            for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
            {
                UE_LOG(LogTemp, Warning, TEXT("[Controller] 등록된 어빌리티: %s"),
                    *Spec.Ability->GetClass()->GetName());
            }

            bool bResult = ASC->TryActivateAbilitiesByTag(
                FGameplayTagContainer(FGameplayTag::RequestGameplayTag("KatanaNeon.Ability.Overclock.Lv1")));

            UE_LOG(LogTemp, Error, TEXT("[Controller] TryActivate 결과: %d"), bResult);
        }
    }*/

    TryActivateAbilityByTag(KatanaNeon::Ability::Overclock::Lv1);
}

void AKNPlayerController::Input_OverclockLv2(const FInputActionValue&)
{
    TryActivateAbilityByTag(KatanaNeon::Ability::Overclock::Lv2);
}

void AKNPlayerController::Input_OverclockLv3(const FInputActionValue&)
{
    TryActivateAbilityByTag(KatanaNeon::Ability::Overclock::Lv3);
}

// 기타 유틸리티 액션
void AKNPlayerController::Input_LockOn(const FInputActionValue&)
{
    /** @brief 락온 키를 누르면 LockOn 태그를 가진 어빌리티를 토글 방식으로 활성화합니다. */
    TryActivateAbilityByTag(KatanaNeon::Ability::Combat::LockOn);
}
void AKNPlayerController::Input_Interact(const FInputActionValue&) {}
void AKNPlayerController::Input_Potion(const FInputActionValue&) {}
void AKNPlayerController::Input_PauseMenu(const FInputActionValue&)
{
    // 일시정지 함수 호출
    SetPause(true);
}
#pragma endregion 입력 콜백 함수 구현