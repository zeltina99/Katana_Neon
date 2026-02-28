// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Components/KNStatsComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
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
    AttributeSet = const_cast<UKNAttributeSet*>(ASC->GetSet<UKNAttributeSet>());

    if (!AttributeSet)
    {
        UE_LOG(LogTemp, Error, TEXT("[KNStatsComponent] UKNAttributeSet을 찾을 수 없습니다."));
        return;
    }

    // 데이터 테이블 로드 및 런타임 캐시 세팅
    // TODO: 프로젝트의 실제 구조체 타입과 GetRow 방식에 맞춰 주석 해제하여 사용하세요.
    /*
    if (const FKNBaseStatRow* BaseRow = BaseStatRowHandle.GetRow<FKNBaseStatRow>(TEXT("InitBaseStat")))
    {
        ApplyBaseStats(BaseRow);
    }

    if (const FKNActionCostRow* CostRow = ActionCostRowHandle.GetRow<FKNActionCostRow>(TEXT("InitActionCost")))
    {
        ActionCost = *CostRow;
    }

    if (const FKNOverclockSettingRow* OCRow = OverclockSettingRowHandle.GetRow<FKNOverclockSettingRow>(TEXT("InitOverclock")))
    {
        OverclockSetting = *OCRow;
    }
    */

    // 핵심 스탯 UI 브로드캐스트 콜백 등록
    ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
        .AddLambda([this](const FOnAttributeChangeData& Data) {
        OnHealthChanged.Broadcast(Data.NewValue, AttributeSet->GetMaxHealth());
            });

    ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute())
        .AddLambda([this](const FOnAttributeChangeData& Data) {
        OnStaminaChanged.Broadcast(Data.NewValue, AttributeSet->GetMaxStamina());
            });

    ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetChronosAttribute())
        .AddLambda([this](const FOnAttributeChangeData& Data) {
        OnChronosChanged.Broadcast(Data.NewValue, AttributeSet->GetMaxChronos());
            });

    // 오버클럭 자동 태그 시스템 콜백 등록
    ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetOverclockPointAttribute())
        .AddUObject(this, &UKNStatsComponent::OnOverclockPointChangedInternal);
}
#pragma endregion 기본 생성자 및 초기화 구현 끝

#pragma region 핵심 조작 구현
void UKNStatsComponent::GainOverclockPoint(float GainAmount)
{
    if (GainAmount <= 0.0f) return;

    // 임시 태그: 실제 프로젝트의 태그 경로로 맞춰주세요. (예: KatanaNeon::Data::Stats::OverclockPoint)
    FGameplayTag OCTag = FGameplayTag::RequestGameplayTag(FName("Data.Stats.OverclockPoint"));
    ApplyInstantGEInternal(OCTag, GainAmount);
}

bool UKNStatsComponent::ConsumeOverclockLevel(int32 Level)
{
    if (!ASC) return false;

    FGameplayTag TargetTag;
    float ConsumeAmount = 0.0f;

    // TODO: OverclockSetting 캐시에서 임계값을 가져오도록 수정 필요
    switch (Level)
    {
    case 1:
        TargetTag = FGameplayTag::RequestGameplayTag(FName("State.Overclock.Lv1"));
        ConsumeAmount = 100.0f; // OverclockSetting.Lv1Threshold
        break;
    case 2:
        TargetTag = FGameplayTag::RequestGameplayTag(FName("State.Overclock.Lv2"));
        ConsumeAmount = 200.0f; // OverclockSetting.Lv2Threshold
        break;
    case 3:
        TargetTag = FGameplayTag::RequestGameplayTag(FName("State.Overclock.Lv3"));
        ConsumeAmount = 300.0f; // OverclockSetting.Lv3Threshold
        break;
    default:
        return false;
    }

    if (!ASC->HasMatchingGameplayTag(TargetTag)) return false;

    FGameplayTag OCTag = FGameplayTag::RequestGameplayTag(FName("Data.Stats.OverclockPoint"));
    ApplyInstantGEInternal(OCTag, -ConsumeAmount);

    return true;
}
#pragma endregion 핵심 조작 구현 끝

#pragma region 데이터 조회 구현
int32 UKNStatsComponent::GetCurrentOverclockLevel() const
{
    if (!ASC) return 0;

    // 하위 레벨보다 상위 레벨을 먼저 체크
    if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Overclock.Lv3")))) return 3;
    if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Overclock.Lv2")))) return 2;
    if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Overclock.Lv1")))) return 1;

    return 0;
}
#pragma endregion 데이터 조회 구현 끝

#pragma region 내부 콜백 및 헬퍼 구현
void UKNStatsComponent::OnOverclockPointChangedInternal(const FOnAttributeChangeData& Data)
{
    // Clamp 로직은 KNAttributeSet이 담당하므로, 여기서는 순수하게 동기화만 처리합니다.
    SyncOverclockLevelTags(Data.NewValue);

    // TODO: OverclockSetting.MaxOverclockPoint를 Max 인자로 사용하도록 연동 필요
    OnOverclockPointChanged.Broadcast(Data.NewValue, 300.0f);
}

void UKNStatsComponent::ApplyBaseStats(const FKNBaseStatRow* BaseStatRow)
{
    if (!ASC || !InstantGEClass || !BaseStatRow) return;

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(InstantGEClass, 1.0f, Context);
    FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

    if (!Spec) return;

    // TODO: BaseStatRow의 데이터를 Spec->SetSetByCallerMagnitude를 통해 적용
    // 이 작업이 끝나면 BaseStatRow 포인터는 더 이상 쥐고 있을 필요가 없으므로 메모리가 절약됩니다.

    ASC->ApplyGameplayEffectSpecToSelf(*Spec);
}

void UKNStatsComponent::SyncOverclockLevelTags(float CurrentPoint)
{
    // TODO: OverclockSetting의 임계값과 연동 필요
    const bool bLv1 = CurrentPoint >= 100.0f;
    const bool bLv2 = CurrentPoint >= 200.0f;
    const bool bLv3 = CurrentPoint >= 300.0f;

    SetGameplayTagActive(FGameplayTag::RequestGameplayTag(FName("State.Overclock.Lv1")), bLv1);
    SetGameplayTagActive(FGameplayTag::RequestGameplayTag(FName("State.Overclock.Lv2")), bLv2);
    SetGameplayTagActive(FGameplayTag::RequestGameplayTag(FName("State.Overclock.Lv3")), bLv3);

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
    FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

    if (!Spec) return false;

    Spec->SetSetByCallerMagnitude(StatTag, Delta);
    const FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);

    return Handle.IsValid();
}
#pragma endregion 내부 콜백 및 헬퍼 구현 끝

