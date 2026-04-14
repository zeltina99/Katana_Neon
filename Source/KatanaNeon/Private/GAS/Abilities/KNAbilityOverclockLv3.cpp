// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/KNAbilityOverclockLv3.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Characters/Base/KNCharacterBase.h" // ACharacter 교체
#include "GameFramework/WorldSettings.h"
#include "Engine/DataTable.h"
#include "GAS/Components/KNStatsComponent.h"
#include "GAS/Tags/KNStatsTags.h" 

#pragma region 기본 생성자 및 초기화 구현
UKNAbilityOverclockLv3::UKNAbilityOverclockLv3()
{
    // ── UE 5.5 최신 규약 적용 (경고 제거) ──
    FGameplayTagContainer TempTags;
    TempTags.AddTag(KatanaNeon::Ability::Overclock::Lv3);
    SetAssetTags(TempTags);

    // 오버클럭 Lv3 태그 없으면 자동 차단
    ActivationRequiredTags.AddTag(KatanaNeon::State::Overclock::Lv3);

    // 시간 정지 중 재발동 방지
    ActivationBlockedTags.AddTag(KatanaNeon::State::Combat::WorldTimeFrozen);

    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 핵심 오버라이드 구현
void UKNAbilityOverclockLv3::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // ── 1. DataTable 수치 로드 (DDD) ──
    if (!LoadLv3Setting())
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

    AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo());

    // ── 3. 발동 몽타주 재생 ──
    if (Owner && TimeFreezeUltimateMontage)
    {
        Owner->PlayAnimMontage(TimeFreezeUltimateMontage);
    }

    // ── 4. Niagara 이펙트 스폰 ──
    if (Owner && TimeStopNiagara)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), TimeStopNiagara, Owner->GetActorLocation());
    }

    // ── 5. 시간 정지 활성화 ──
    ActivateTimeStop();
}

void UKNAbilityOverclockLv3::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // 타이머 해제
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TimeStopTimerHandle);
    }

    // 시간 정지 중이면 강제로 1.0(정상) 복구
    if (bIsTimeStopped)
    {
        ApplyTimeDilation(1.0f);
        bIsTimeStopped = false;
    }

    // 상태 태그 안전 제거
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::WorldTimeFrozen))
        {
            ASC->RemoveLooseGameplayTag(KatanaNeon::State::Combat::WorldTimeFrozen);
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion GAS 핵심 오버라이드 구현

#pragma region 외부 조회 인터페이스 구현
float UKNAbilityOverclockLv3::GetFrozenDamageMultiplier() const
{
    return bIsTimeStopped ? CachedSetting.FrozenDamageMultiplier : 1.0f;
}
#pragma endregion 외부 조회 인터페이스 구현

#pragma region 내부 헬퍼 함수 구현
bool UKNAbilityOverclockLv3::LoadLv3Setting()
{
    // 하드코딩 배제 및 안전 검사
    if (!Lv3SettingRowHandle.DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[KNAbilityOverclockLv3] Lv3SettingRowHandle이 에디터에 할당되지 않았습니다!"));
        return false;
    }

    const FKNOverclockLv3Row* Row = Lv3SettingRowHandle.GetRow<FKNOverclockLv3Row>(TEXT("LoadLv3Setting"));

    if (!ensureAlwaysMsgf(Row, TEXT("[KNAbilityOverclockLv3] 지정된 오버클럭 Lv3 설정 행을 찾을 수 없습니다.")))
    {
        return false;
    }

    CachedSetting = *Row;
    return true;
}

bool UKNAbilityOverclockLv3::ConsumeOverclockLevel()
{
    // 최적화: 범용 ACharacter가 아닌 우리 고유의 베이스 캐릭터로 캐스팅
    AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo());
    if (!Owner) return false;

    // 활성화 시 1회만 호출되므로 FindComponentByClass 허용
    UKNStatsComponent* Stats = Owner->FindComponentByClass<UKNStatsComponent>();
    if (!Stats)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KNAbilityOverclockLv3] KNStatsComponent를 찾을 수 없습니다!"));
        return false;
    }

    return Stats->ConsumeOverclockLevel(CachedSetting.OverclockCost);
}

void UKNAbilityOverclockLv3::ApplyTimeDilation(float WorldScale)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // ── GlobalTimeDilation 설정 ──
    if (AWorldSettings* WS = World->GetWorldSettings())
    {
        WS->SetTimeDilation(WorldScale);
    }

    // ── 플레이어 CustomTimeDilation 보정 ──
    // CustomTimeDilation = 1.0 / GlobalTimeDilation → 플레이어의 체감 속도는 1.0 (정상 유지)
    if (AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo()))
    {
        // 0으로 나누는(Divide by Zero) 크래시 방지를 위한 안전장치
        Owner->CustomTimeDilation = (WorldScale > KINDA_SMALL_NUMBER) ? (1.0f / WorldScale) : 1.0f;
    }
}

void UKNAbilityOverclockLv3::ActivateTimeStop()
{
    // ── 시간 배율 적용 ──
    ApplyTimeDilation(CachedSetting.WorldTimeDilationScale);
    bIsTimeStopped = true;

    // ── 상태 태그 부여 ──
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        ASC->AddLooseGameplayTag(KatanaNeon::State::Combat::WorldTimeFrozen);
    }

    // ── 자동 해제 타이머 설정 ──
    // 💡 베테랑의 팁: 엔진의 TimeDilation이 극단적으로 낮아지면(예: 0.0001) 일반 타이머도 
    // 그에 맞춰 느리게 흐릅니다. (5초 타이머가 실제 현실 시간으로 수만 초가 걸릴 수 있음)
    // 따라서 타이머 세팅 시, 지속 시간에 WorldTimeDilationScale을 곱해주어 
    // 엔진 틱 기준에 맞추는 방식으로 세팅하는 것이 실무에서 안전한 TimeStop 테크닉입니다.

    float ActualTimerDelay = CachedSetting.TimeStopDuration * CachedSetting.WorldTimeDilationScale;
    if (ActualTimerDelay < KINDA_SMALL_NUMBER)
    {
        ActualTimerDelay = CachedSetting.TimeStopDuration; // 예외 처리 방어
    }

    GetWorld()->GetTimerManager().SetTimer(
        TimeStopTimerHandle,
        this,
        &UKNAbilityOverclockLv3::OnTimeStopExpired,
        ActualTimerDelay,
        false);

    UE_LOG(LogTemp, Log,
        TEXT("[KNAbilityOverclockLv3] 시간 정지 활성화. GlobalTimeDilation=%.4f, PlayerCustom=%.1f, Duration=%.1f초"),
        CachedSetting.WorldTimeDilationScale,
        (CachedSetting.WorldTimeDilationScale > 0) ? (1.0f / CachedSetting.WorldTimeDilationScale) : 1.0f,
        CachedSetting.TimeStopDuration);
}

void UKNAbilityOverclockLv3::OnTimeStopExpired()
{
    UE_LOG(LogTemp, Log, TEXT("[KNAbilityOverclockLv3] 시간 정지 만료 → 해제 진행"));

    EndAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo(),
        true, false);
}
#pragma endregion 내부 헬퍼 함수 구현
