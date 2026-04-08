// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/KNAbilityChronos.h"
#include "AbilitySystemComponent.h"
#include "Characters/Base/KNCharacterBase.h" // ACharacter 교체
#include "Engine/DataTable.h"
#include "Components/KNChronosSphereComponent.h"
#include "GAS/Attributes/KNAttributeSet.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 기본 생성자 및 초기화 구현
UKNAbilityChronos::UKNAbilityChronos()
{
    // ── UE 5.5 최신 규약 적용 (경고 제거) ──
    FGameplayTagContainer TempTags;
    TempTags.AddTag(KatanaNeon::Ability::Combat::Chronos);
    SetAssetTags(TempTags);

    // 인스턴스 단위로 토글 상태(bIsChronosActive)를 관리하므로 InstancedPerActor 필수
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 핵심 오버라이드 구현
bool UKNAbilityChronos::CanActivateAbility(
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

    // 이미 활성 중이면 토글 OFF를 위해 무조건 허용
    if (bIsChronosActive) return true;

    // 최초 활성화: 크로노스 게이지 잔량 확인
    if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
    {
        if (const UKNAttributeSet* AttrSet = ASC->GetSet<UKNAttributeSet>())
        {
            return AttrSet->GetChronos() > 0.0f;
        }
    }
    return false;
}

void UKNAbilityChronos::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // ── 토글 ON 로직 ──
    if (!LoadChronosSetting())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!ResolveSphereComponent())
    {
        UE_LOG(LogTemp, Error, TEXT("[KNAbilityChronos] KNChronosSphereComponent를 플레이어 캐릭터에서 찾을 수 없습니다!"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 상태 태그 부여
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        ASC->AddLooseGameplayTag(KatanaNeon::State::Combat::ChronosActive);
    }

    // 구체 콜리전 활성화 (범위 감속 시작)
    if (UKNChronosSphereComponent* Sphere = WeakSphereComp.Get())
    {
        Sphere->ActivateSphere(
            CachedSetting.EnemySlowScale,
            CachedSetting.ProjectileSlowScale,
            CachedSetting.SphereRadius);
    }

    // 게이지 소모 반복 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(
        DrainTimerHandle,
        this,
        &UKNAbilityChronos::OnDrainTick,
        CachedSetting.DrainTickInterval,
        true);

    bIsChronosActive = true;
}

void UKNAbilityChronos::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // 소모 타이머 해제
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DrainTimerHandle);
    }

    // 구체 비활성화 (내부에서 모든 감속 복구 수행)
    if (UKNChronosSphereComponent* Sphere = WeakSphereComp.Get())
    {
        Sphere->DeactivateSphere();
    }

    // 상태 태그 안전 제거
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::ChronosActive))
        {
            ASC->RemoveLooseGameplayTag(KatanaNeon::State::Combat::ChronosActive);
        }
    }

    bIsChronosActive = false;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion GAS 핵심 오버라이드 구현

#pragma region 내부 헬퍼 함수 구현
bool UKNAbilityChronos::LoadChronosSetting()
{
    // 하드코딩 제거 및 안전 검사
    if (!ChronosSettingRowHandle.DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[KNAbilityChronos] ChronosSettingRowHandle이 에디터에 할당되지 않았습니다!"));
        return false;
    }

    const FKNChronosSettingRow* Row = ChronosSettingRowHandle.GetRow<FKNChronosSettingRow>(TEXT("LoadChronosSetting"));

    if (!ensureAlwaysMsgf(Row, TEXT("[KNAbilityChronos] 지정된 ChronosSetting 행을 찾을 수 없습니다.")))
    {
        return false;
    }

    CachedSetting = *Row;
    return true;
}

bool UKNAbilityChronos::ResolveSphereComponent()
{
    if (WeakSphereComp.IsValid()) return true;

    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!Avatar) return false;

    UKNChronosSphereComponent* Sphere = Avatar->FindComponentByClass<UKNChronosSphereComponent>();
    if (!Sphere) return false;

    WeakSphereComp = Sphere;
    return true;
}

void UKNAbilityChronos::OnDrainTick()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC || !ChronosDrainGEClass) return;

    // 잔량 확인 → 0 이하면 어빌리티 자동 종료
    if (const UKNAttributeSet* AttrSet = ASC->GetSet<UKNAttributeSet>())
    {
        if (AttrSet->GetChronos() <= 0.0f)
        {
            EndAbility(
                GetCurrentAbilitySpecHandle(),
                GetCurrentActorInfo(),
                GetCurrentActivationInfo(),
                true, false);
            return;
        }
    }

    // DrainTickInterval 동안의 소모량 계산
    const float DrainAmount = CachedSetting.DrainRatePerSecond * CachedSetting.DrainTickInterval;

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle    SpecHandle = ASC->MakeOutgoingSpec(ChronosDrainGEClass, 1.0f, Context);

    if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
    {
        // 엑셀에서 기획자가 지정한 수치만큼 마이너스 처리
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Chronos, -DrainAmount);
        ASC->ApplyGameplayEffectSpecToSelf(*Spec);
    }
}
#pragma endregion 내부 헬퍼 함수 구현
