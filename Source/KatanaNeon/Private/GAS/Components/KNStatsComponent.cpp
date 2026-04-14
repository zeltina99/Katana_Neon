// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Components/KNStatsComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "GAS/Attributes/KNAttributeSet.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 기본 생성자 및 초기화 구현
UKNStatsComponent::UKNStatsComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 이벤트 주도형 아키텍처이므로 틱 비활성화
}

void UKNStatsComponent::BeginPlay()
{
    Super::BeginPlay();
    // 초기화는 오너 액터에서 InitializeStatComponent를 통해 명시적으로 통제합니다.
}

void UKNStatsComponent::InitializeStatComponent(UAbilitySystemComponent* InASC)
{
    if (ASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KNStatsComponent] 이미 초기화되었습니다."));
        return;
    }

    if (!ensure(InASC)) return;

    ASC = InASC;
    // ── 베테랑의 정석 반영: const_cast 삭제, 순수 const 포인터로 안전하게 캐싱 ──
    AttributeSet = ASC->GetSet<UKNAttributeSet>();

    if (!AttributeSet)
    {
        UE_LOG(LogTemp, Error, TEXT("[KNStatsComponent] UKNAttributeSet을 찾을 수 없습니다."));
        return;
    }

    // 데이터 드라이븐 데이터 테이블 로드
    const FKNBaseStatRow* BaseRow = BaseStatRowHandle.GetRow<FKNBaseStatRow>(TEXT("InitBaseStat"));
    const FKNActionCostRow* CostRow = ActionCostRowHandle.GetRow<FKNActionCostRow>(TEXT("InitActionCost"));
    const FKNOverclockSettingRow* OCRow = OverclockSettingRowHandle.GetRow<FKNOverclockSettingRow>(TEXT("InitOverclock"));

    if (CostRow) ActionCost = *CostRow;
    if (OCRow) OverclockSetting = *OCRow;

    // MaxOverclockPoint 초기화를 위해 OCRow도 함께 넘깁니다.
    if (BaseRow && OCRow)
    {
        ApplyBaseStats(BaseRow, OCRow);
    }

    // 핵심 스탯 UI 브로드캐스트 콜백 등록
    // AddLambda([this]) 대신 AddWeakLambda를 사용하여 컴포넌트 파괴 시 엔진이 자동으로 호출을 차단합니다.
    ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
        .AddWeakLambda(this, [this](const FOnAttributeChangeData& Data) {
        OnHealthChanged.Broadcast(Data.NewValue, AttributeSet->GetMaxHealth());
            });

    ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute())
        .AddWeakLambda(this, [this](const FOnAttributeChangeData& Data) {
        OnStaminaChanged.Broadcast(Data.NewValue, AttributeSet->GetMaxStamina());

        // 스태미나가 소모된 경우에만 딜레이 타이머를 초기화합니다.
        if (Data.NewValue < Data.OldValue)
        {
            RestartRegenDelay();
        }
            });

    ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetChronosAttribute())
        .AddWeakLambda(this, [this](const FOnAttributeChangeData& Data) {
        OnChronosChanged.Broadcast(Data.NewValue, AttributeSet->GetMaxChronos());
            });

    // 무기 상태 태그 변경 감지
    ASC->RegisterGameplayTagEvent(
        KatanaNeon::State::Combat::WeaponDrawn,
        EGameplayTagEventType::NewOrRemoved)
        .AddUObject(this, &UKNStatsComponent::OnWeaponDrawnTagChanged);

    /**
     * @brief 이동 속도 동기화 델리게이트
     * GAS의 이동 속도 스탯이 변경될 때마다(버프 획득/해제 등) 실제 캡슐 물리 엔진(MaxWalkSpeed)에 즉각 반영합니다.
     */
    ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMovementSpeedAttribute())
        .AddWeakLambda(this, [this](const FOnAttributeChangeData& Data) {

        if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
        {
            if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
            {
                MovementComp->MaxWalkSpeed = Data.NewValue;
            }
        }
            });

    // 오버클럭 자동 태그 시스템 콜백 등록
    ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetOverclockPointAttribute())
        .AddUObject(this, &UKNStatsComponent::OnOverclockPointChangedInternal);

    SyncOverclockLevelTags(AttributeSet->GetOverclockPoint());

    // ── 스태미나 자연 회복 타이머 시작 (ApplyBaseStats 완료 후 시점 보장) ──
    StartStaminaRegen();

    OnHealthChanged.Broadcast(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
    OnStaminaChanged.Broadcast(AttributeSet->GetStamina(), AttributeSet->GetMaxStamina());
    OnChronosChanged.Broadcast(AttributeSet->GetChronos(), AttributeSet->GetMaxChronos());
    OnOverclockPointChanged.Broadcast(AttributeSet->GetOverclockPoint(), OverclockSetting.MaxOverclockPoint);
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region 핵심 조작 구현
void UKNStatsComponent::GainOverclockPoint(float GainAmount)
{
    if (GainAmount <= 0.0f) return;

    ApplyInstantGEInternal(KatanaNeon::Data::Stats::OverclockPoint, GainAmount);
}

bool UKNStatsComponent::ConsumeOverclockLevel(int32 Level)
{
    if (!ASC || !AttributeSet) return false;

    float ConsumeAmount = 0.0f;

    switch (Level)
    {
    case 1: ConsumeAmount = OverclockSetting.Lv1Threshold; break;
    case 2: ConsumeAmount = OverclockSetting.Lv2Threshold; break;
    case 3: ConsumeAmount = OverclockSetting.Lv3Threshold; break;
    default: return false;
    }

    // 태그 대신 실제 포인트 값으로 직접 판단
    if (AttributeSet->GetOverclockPoint() < ConsumeAmount) return false;

    ApplyInstantGEInternal(KatanaNeon::Data::Stats::OverclockPoint, -ConsumeAmount);
    return true;
}
#pragma endregion 핵심 조작 구현

#pragma region 데이터 조회 구현
int32 UKNStatsComponent::GetCurrentOverclockLevel() const
{
    if (!ASC) return 0;

    // 동적 문자열 할당을 지우고 컴파일 타임 네이티브 상수로 완벽 교체
    if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Overclock::Lv3)) return 3;
    if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Overclock::Lv2)) return 2;
    if (ASC->HasMatchingGameplayTag(KatanaNeon::State::Overclock::Lv1)) return 1;

    return 0;
}
#pragma endregion 데이터 조회 구현

#pragma region 내부 콜백 및 헬퍼 구현
void UKNStatsComponent::OnOverclockPointChangedInternal(const FOnAttributeChangeData& Data)
{
    // Clamp 로직은 KNAttributeSet이 담당하므로, 여기서는 순수하게 동기화만 처리합니다.
    SyncOverclockLevelTags(Data.NewValue);
    // 하드코딩된 300.0f 맥스값을 런타임 캐시 변수로 대체
    OnOverclockPointChanged.Broadcast(Data.NewValue, OverclockSetting.MaxOverclockPoint);
}

void UKNStatsComponent::ApplyBaseStats(const FKNBaseStatRow* BaseStatRow, const FKNOverclockSettingRow* OCRow)
{
    if (!ASC || !InstantGEClass || !BaseStatRow || !OCRow) return;

    UE_LOG(LogTemp, Error, TEXT("[ApplyBaseStats] 호출됨 - %s"), *GetOwner()->GetName());

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(InstantGEClass, 1.0f, Context);
    if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
    {
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::MaxHealth, BaseStatRow->MaxHealth);
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Health, BaseStatRow->MaxHealth);
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::MaxStamina, BaseStatRow->MaxStamina);
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Stamina, BaseStatRow->MaxStamina);
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::StaminaRegenRate, BaseStatRow->StaminaRegenRate);
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::MovementSpeed, BaseStatRow->MovementSpeed);
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::MaxChronos, BaseStatRow->MaxChronos);
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::Chronos, BaseStatRow->MaxChronos);
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::AttackSpeed, BaseStatRow->AttackSpeed);
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::MaxOverclockPoint, OCRow->MaxOverclockPoint);
        Spec->SetSetByCallerMagnitude(KatanaNeon::Data::Stats::OverclockPoint, OCRow->InitialOverclockPoint);

        ASC->ApplyGameplayEffectSpecToSelf(*Spec);
    }
}

void UKNStatsComponent::SyncOverclockLevelTags(float CurrentPoint)
{
    // 데이터 드라이븐 연동 완료
    const bool bLv1 = CurrentPoint >= OverclockSetting.Lv1Threshold;
    const bool bLv2 = CurrentPoint >= OverclockSetting.Lv2Threshold;
    const bool bLv3 = CurrentPoint >= OverclockSetting.Lv3Threshold;

    SetGameplayTagActive(KatanaNeon::State::Overclock::Lv1, bLv1);
    SetGameplayTagActive(KatanaNeon::State::Overclock::Lv2, bLv2);
    SetGameplayTagActive(KatanaNeon::State::Overclock::Lv3, bLv3);

    const int32 NewLevel = bLv3 ? 3 : bLv2 ? 2 : bLv1 ? 1 : 0;
    OnOverclockLevelChanged.Broadcast(NewLevel);
}

void UKNStatsComponent::SetGameplayTagActive(const FGameplayTag& Tag, bool bGrant)
{
    if (!ASC || !Tag.IsValid()) return;

    const bool bHas = ASC->HasMatchingGameplayTag(Tag);
    if (bGrant && !bHas)
    {
        ASC->AddLooseGameplayTag(Tag);
    }
    else if (!bGrant && bHas)
    {
        ASC->RemoveLooseGameplayTag(Tag);
    }
}

bool UKNStatsComponent::ApplyInstantGEInternal(const FGameplayTag& StatTag, float Delta)
{
    if (!ASC || !InstantGEClass || !StatTag.IsValid()) return false;

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(InstantGEClass, 1.0f, Context);
    if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
    {
        Spec->SetSetByCallerMagnitude(StatTag, Delta);
        const FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);
        return Handle.IsValid();
    }
    return false;
}

void UKNStatsComponent::StartStaminaRegen()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 중복 실행 방지
    if (World->GetTimerManager().IsTimerActive(StaminaRegenTimerHandle)) return;

    World->GetTimerManager().SetTimer(
        StaminaRegenTimerHandle,
        this,
        &UKNStatsComponent::OnStaminaRegenTick,
        StaminaRegenTickInterval,
        /*bLoop=*/true);
}

void UKNStatsComponent::OnStaminaRegenTick()
{
    if (!ASC || !AttributeSet) return;

    // 이미 최대치면 GE 호출 비용 자체를 차단합니다.
    if (AttributeSet->GetStamina() >= AttributeSet->GetMaxStamina()) return;

    // 초당 회복량 × 틱 간격 = 이번 틱 회복량 (매직넘버 없이 어트리뷰트에서 직접 조회)
    const float RegenThisTick = AttributeSet->GetStaminaRegenRate() * StaminaRegenTickInterval;
    if (RegenThisTick <= 0.0f) return;

    ApplyInstantGEInternal(KatanaNeon::Data::Stats::Stamina, RegenThisTick);
}

void UKNStatsComponent::RestartRegenDelay()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 진행 중인 리젠 틱을 중단하고 딜레이를 처음부터 다시 셉니다.
    World->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);
    World->GetTimerManager().SetTimer(
        StaminaRegenDelayHandle,
        this,
        &UKNStatsComponent::StartStaminaRegen,
        StaminaRegenDelaySeconds,
        /*bLoop=*/false);
}

void UKNStatsComponent::OnWeaponDrawnTagChanged(const FGameplayTag Tag, int32 Count)
{
    OnWeaponStateChanged.Broadcast(Count > 0);
}
#pragma endregion 내부 콜백 및 헬퍼 구현

#pragma region 영구 및 시간제 버프 API 구현
void UKNStatsComponent::ApplyInfiniteBuff(const FGameplayTag& StatTag, float Delta, const FGameplayTag& HandleKey)
{
    if (!ASC || !InfiniteGEClass || !StatTag.IsValid() || !HandleKey.IsValid()) return;

    RemoveInfiniteBuff(HandleKey); // 동일 키 버프 덮어쓰기 로직 방어

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(InfiniteGEClass, 1.0f, Context);
    if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
    {
        Spec->SetSetByCallerMagnitude(StatTag, Delta);
        FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);

        if (Handle.IsValid())
        {
            InfiniteBuffHandles.Add(HandleKey, Handle);
        }
    }
}

void UKNStatsComponent::RemoveInfiniteBuff(const FGameplayTag& HandleKey)
{
    if (!ASC) return;

    if (FActiveGameplayEffectHandle* FoundHandle = InfiniteBuffHandles.Find(HandleKey))
    {
        ASC->RemoveActiveGameplayEffect(*FoundHandle);
        InfiniteBuffHandles.Remove(HandleKey);
    }
}

void UKNStatsComponent::ApplyDurationBuff(const FGameplayTag& StatTag, float Delta, float Duration)
{
    if (!ASC || !DurationGEClass || !StatTag.IsValid() || Duration <= 0.0f) return;


    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DurationGEClass, 1.0f, Context);
    if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
    {
        Spec->SetDuration(Duration, true);
        Spec->SetSetByCallerMagnitude(StatTag, Delta);
        ASC->ApplyGameplayEffectSpecToSelf(*Spec);
    }
}
#pragma endregion 영구 및 시간제 버프 API 구현