// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/KNAbilityJump.h"
#include "AbilitySystemComponent.h"
#include "Characters/Base/KNCharacterBase.h" // ACharacter 대신 고유 베이스 캐릭터
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DataTable.h"
#include "GAS/Attributes/KNAttributeSet.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 기본 생성자 및 초기화 구현
UKNAbilityJump::UKNAbilityJump()
{
    // 언리얼 5.5 권장 규약 (경고 제거)
    FGameplayTagContainer TempTags;
    TempTags.AddTag(KatanaNeon::Ability::Movement::Jump);
    SetAssetTags(TempTags);

    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 핵심 오버라이드 구현
bool UKNAbilityJump::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    if (!JumpSettingRowHandle.DataTable) return false;

    // 기획자가 에디터에 할당한 점프 수치 로드
    const FKNJumpSettingRow* CostRow = JumpSettingRowHandle.GetRow<FKNJumpSettingRow>(TEXT("CanActivateJump"));
    if (!CostRow) return false;

    AKNCharacterBase* Character = Cast<AKNCharacterBase>(ActorInfo->AvatarActor.Get());
    if (!Character) return false;

    UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
    if (!MoveComp) return false;

    // 현재 공중인지 지상인지 판별하여 요구 스태미나 결정
    const bool bIsFalling = MoveComp->IsFalling();
    const float RequiredStamina = bIsFalling ? CostRow->DoubleJumpStaminaCost : CostRow->JumpStaminaCost;

    if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
    {
        if (const UKNAttributeSet* AttrSet = ASC->GetSet<UKNAttributeSet>())
        {
            UE_LOG(LogTemp, Error, TEXT("[Jump CanActivate] IsFalling: %d / 현재스태미나: %.1f / 필요스태미나: %.1f"),
                bIsFalling, AttrSet->GetStamina(), RequiredStamina);
        }
    }

    // 공중일 경우: 더블 점프가 활성화되어 있는지, 그리고 이미 더블 점프를 소모했는지 검사
    if (bIsFalling)
    {
        if (!CostRow->bDoubleJumpEnabled) return false;

        UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
        if (ASC && ASC->HasMatchingGameplayTag(KatanaNeon::State::Movement::DoubleJumped))
        {
            return false; // 이미 공중에서 점프를 소모함
        }
    }

    // 스태미나 잔량 검사
    if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
    {
        if (const UKNAttributeSet* AttrSet = ASC->GetSet<UKNAttributeSet>())
        {
            return AttrSet->GetStamina() >= RequiredStamina;
        }
    }

    return false;
}

void UKNAbilityJump::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!LoadJumpSetting())
    {
        UE_LOG(LogTemp, Error, TEXT("[Jump] LoadJumpSetting 실패"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AKNCharacterBase* Character = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo());
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (!Character || !ASC)
    {
        UE_LOG(LogTemp, Error, TEXT("[Jump] Character 또는 ASC 없음"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
    if (!MoveComp)
    {
        UE_LOG(LogTemp, Error, TEXT("[Jump] MoveComp 없음"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const bool bIsFalling = MoveComp->IsFalling();
    const float CostToConsume = bIsFalling ? CachedJumpSetting.DoubleJumpStaminaCost : CachedJumpSetting.JumpStaminaCost;

    // 스태미나 소모 처리
    if (CostToConsume > 0.0f)
    {
        if (!ConsumeStamina(CostToConsume))
        {
            UE_LOG(LogTemp, Error, TEXT("[Jump] 스태미나 부족"));
            EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
            return;
        }
    }

    // 지상 vs 공중(더블점프) 분기 실행
    if (bIsFalling)
    {
        UE_LOG(LogTemp, Error, TEXT("[Jump] PerformDoubleJump 진입"));
        PerformDoubleJump(Character, MoveComp, ASC);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Jump] PerformGroundJump 진입"));
        PerformGroundJump(Character, MoveComp);
    }

    // 이동 명령 하달 후 어빌리티 즉시 종료 (점프 체공 시간은 엔진이 물리로 계산)
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UKNAbilityJump::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion GAS 핵심 오버라이드 구현

#pragma region 내부 헬퍼 함수 구현
bool UKNAbilityJump::LoadJumpSetting()
{
    if (!JumpSettingRowHandle.DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[KNAbility_Jump] JumpSettingRowHandle이 에디터에 할당되지 않았습니다!"));
        return false;
    }

    const FKNJumpSettingRow* Row = JumpSettingRowHandle.GetRow<FKNJumpSettingRow>(TEXT("LoadJumpSetting"));
    if (!ensureAlwaysMsgf(Row, TEXT("[KNAbility_Jump] 지정된 JumpSetting 행을 찾을 수 없습니다!")))
    {
        return false;
    }

    CachedJumpSetting = *Row;
    bSettingLoaded = true;
    return true;
}

bool UKNAbilityJump::ConsumeStamina(float CostAmount)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    UE_LOG(LogTemp, Error, TEXT("[Jump ConsumeStamina] ASC: %d / StaminaCostGEClass: %d / CostAmount: %.1f"),
        ASC != nullptr, StaminaCostGEClass != nullptr, CostAmount);

    if (!ASC || !StaminaCostGEClass) return false;

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle    SpecHandle = ASC->MakeOutgoingSpec(StaminaCostGEClass, 1.0f, Context);

    UE_LOG(LogTemp, Error, TEXT("[Jump ConsumeStamina] SpecHandle 유효: %d"), SpecHandle.Data.IsValid());


    if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
    {
        // 캐싱된 비용(엑셀 데이터)만큼 차감
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Stamina, -CostAmount);
        ASC->ApplyGameplayEffectSpecToSelf(*Spec);
        return true;
    }

    UE_LOG(LogTemp, Error, TEXT("[Jump ConsumeStamina] Spec 생성 실패"));
    return false;
}

void UKNAbilityJump::PerformGroundJump(AKNCharacterBase* Character, UCharacterMovementComponent* Movement)
{
    // 물리적 점프 높이 동기화 후 네이티브 점프 실행
    Movement->JumpZVelocity = CachedJumpSetting.JumpZVelocity;
    Character->Jump();
}

void UKNAbilityJump::PerformDoubleJump(AKNCharacterBase* Character, UCharacterMovementComponent* Movement, UAbilitySystemComponent* ASC)
{
    /// 더블 점프 소모 태그 부여 (착지 시 베이스 캐릭터에서 제거해야 함)
    ASC->AddLooseGameplayTag(KatanaNeon::State::Movement::DoubleJumped);

    // 에러 회피: FKNJumpSettingRow에 DoubleJumpVelocityMultiplier가 누락되어 일단 기본 점프력을 그대로 사용합니다.
    const float LaunchVelocity = CachedJumpSetting.JumpZVelocity;

    // Z축 속도를 덮어씌워 공중에서 다시 튀어오르게 만듦
    Character->LaunchCharacter(FVector(0.0f, 0.0f, LaunchVelocity), false, true);

     // ── 디버그 로그 추가 ──
    UE_LOG(LogTemp, Error, TEXT("[DoubleJump] DoubleJumpMontage 유효: %d"),
        DoubleJumpMontage != nullptr);

    // 기획자가 할당한 몽타주가 있다면 재생
    if (DoubleJumpMontage)
    {
        Character->PlayAnimMontage(DoubleJumpMontage);
    }
}
#pragma endregion 내부 헬퍼 함수 구현
