// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/KNAbilityOverclockLv1.h"
#include "AbilitySystemComponent.h"
#include "Characters/Base/KNCharacterBase.h"
#include "Engine/DataTable.h"
#include "GAS/Components/KNStatsComponent.h"
#include "GAS/Tags/KNStatsTags.h" 

#pragma region 기본 생성자 및 초기화 구현
UKNAbilityOverclockLv1::UKNAbilityOverclockLv1()
{
    // ── UE 5.5 최신 규약 적용 (경고 제거) ──
    FGameplayTagContainer TempTags;
    TempTags.AddTag(KatanaNeon::Ability::Overclock::Lv1);
    SetAssetTags(TempTags);

    // 오버클럭 Lv1 태그 없으면 자동 차단
    ActivationRequiredTags.AddTag(KatanaNeon::State::Overclock::Lv1);

    // 전술 강화 중복 발동 불가
    ActivationBlockedTags.AddTag(KatanaNeon::State::Combat::OverclockTactical);

    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 핵심 오버라이드 구현
void UKNAbilityOverclockLv1::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // ── 1. DataTable 수치 로드 (DDD) ──
    if (!LoadLv1Setting())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── 2. 오버클럭 포인트 소모 ──
    if (!ConsumeOverclockLevel())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── 3. 상태 태그 부여 ──
    GrantStateTags(ASC);

    // ── 4. 스탯 버프 GE 적용 ──
    ApplyStatBuffGEs(ASC);

    // ── 5. 발동 몽타주 재생 ──
    if (AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo()))
    {
        if (TacticalMontage)
        {
            Owner->PlayAnimMontage(TacticalMontage);
        }
    }

    // ── 6. 지속 시간 타이머 설정 ──
    GetWorld()->GetTimerManager().SetTimer(
        DurationTimerHandle,
        this,
        &UKNAbilityOverclockLv1::OnDurationExpired,
        CachedSetting.Duration,
        false);
}

void UKNAbilityOverclockLv1::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DurationTimerHandle);
    }

    // Loose 태그 안전 제거
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::OverclockTactical))
        {
            ASC->RemoveLooseGameplayTag(KatanaNeon::State::Combat::OverclockTactical);
        }

        if (CachedSetting.bStaminaImmune &&
            ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::StaminaImmune))
        {
            ASC->RemoveLooseGameplayTag(KatanaNeon::State::Combat::StaminaImmune);
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion GAS 핵심 오버라이드 구현

#pragma region 내부 헬퍼 함수 구현
bool UKNAbilityOverclockLv1::LoadLv1Setting()
{
    if (!Lv1SettingRowHandle.DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[KNAbilityOverclockLv1] Lv1SettingRowHandle이 에디터에 할당되지 않았습니다!"));
        return false;
    }

    const FKNOverclockLv1Row* Row = Lv1SettingRowHandle.GetRow<FKNOverclockLv1Row>(TEXT("LoadLv1Setting"));

    if (!ensureAlwaysMsgf(Row, TEXT("[KNAbilityOverclockLv1] 지정된 오버클럭 Lv1 설정 행을 찾을 수 없습니다.")))
    {
        return false;
    }

    CachedSetting = *Row;
    return true;
}

bool UKNAbilityOverclockLv1::ConsumeOverclockLevel()
{
    // 최적화: 범용 ACharacter가 아닌 우리 고유의 베이스 캐릭터로 캐스팅
    AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo());
    if (!Owner) return false;

    // 활성화 시 1회만 호출되므로 FindComponentByClass 사용이 허용됩니다.
    UKNStatsComponent* Stats = Owner->FindComponentByClass<UKNStatsComponent>();
    if (!Stats)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KNAbilityOverclockLv1] KNStatsComponent를 찾을 수 없습니다!"));
        return false;
    }

    return Stats->ConsumeOverclockLevel(1);
}

void UKNAbilityOverclockLv1::ApplyStatBuffGEs(UAbilitySystemComponent* ASC)
{
    if (!ASC) return;

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();

    // ── 공격 속도 버프 GE 적용 ──
    if (AttackSpeedBuffGEClass)
    {
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(AttackSpeedBuffGEClass, 1.0f, Context);
        if (FGameplayEffectSpec* SpecPtr = Spec.Data.Get())
        {
            SpecPtr->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::AttackSpeed, CachedSetting.AttackSpeedAdditive);
            SpecPtr->SetDuration(CachedSetting.Duration, true);
            ASC->ApplyGameplayEffectSpecToSelf(*SpecPtr);
        }
    }

    // ── 이동 속도 버프 GE 적용 ──
    if (MovementSpeedBuffGEClass)
    {
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(MovementSpeedBuffGEClass, 1.0f, Context);
        if (FGameplayEffectSpec* SpecPtr = Spec.Data.Get())
        {
            SpecPtr->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::MovementSpeed, CachedSetting.MovementSpeedAdditive);
            SpecPtr->SetDuration(CachedSetting.Duration, true);
            ASC->ApplyGameplayEffectSpecToSelf(*SpecPtr);
        }
    }
}

void UKNAbilityOverclockLv1::GrantStateTags(UAbilitySystemComponent* ASC)
{
    if (!ASC) return;

    ASC->AddLooseGameplayTag(KatanaNeon::State::Combat::OverclockTactical);

    if (CachedSetting.bStaminaImmune)
    {
        ASC->AddLooseGameplayTag(KatanaNeon::State::Combat::StaminaImmune);
    }
}

void UKNAbilityOverclockLv1::OnDurationExpired()
{
    EndAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo(),
        true, false);
}
#pragma endregion 내부 헬퍼 함수 구현
