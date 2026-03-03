// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/KNInstantModifier.h"
#include "GAS/Attributes/KNAttributeSet.h"
#include "GAS/Tags/KNStatsTags.h"
#include "GAS/System/KNGASAttributeCache.h"

#pragma region Instant Gameplay Effect 구현
UKNInstantModifier::UKNInstantModifier()
{
    // 즉시 적용 후 소멸하며, 어트리뷰트의 Base Value를 영구적으로 변경합니다.
    DurationPolicy = EGameplayEffectDurationType::Instant;
    Modifiers.Empty();

    // UPROPERTY를 타지 않으므로 명시적인 float 타입으로 초기화
    Period = FScalableFloat(0.0f);

    // 실제 수치 증감 로직은 Execution Calculation에 완벽히 위임
    Executions.Add(FGameplayEffectExecutionDefinition(
        UKNInstantModifierExecution::StaticClass()));
}
#pragma endregion Instant Gameplay Effect 구현

// ────────────────────────────────────────────────────────────

#pragma region Execution Calculation 최적화 구현
UKNInstantModifierExecution::UKNInstantModifierExecution()
{
    // Snapshot = false : 피격 시점의 실시간 현재 값을 참조하여 데미지/회복을 적용합니다.
    for (TFieldIterator<FProperty> It(UKNAttributeSet::StaticClass()); It; ++It)
    {
        // 컴파일 에러를 방지하기 위해 const 키워드를 제외한 캐스팅을 수행합니다.
        if (FStructProperty* StructProp = CastField<FStructProperty>(*It))
        {
            if (StructProp->Struct->GetFName() == TEXT("GameplayAttributeData"))
            {
                FGameplayEffectAttributeCaptureDefinition Def(
                    FGameplayAttribute(StructProp),
                    EGameplayEffectAttributeCaptureSource::Target,
                    /*bSnapshot=*/false);
                RelevantAttributesToCapture.Add(Def);
            }
        }
    }
}

void UKNInstantModifierExecution::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{

    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
    const TMap<FGameplayTag, FGameplayAttribute>& AttributeMap = FKNGASAttributeCache::Get();

    // ── 최적화: TArray 대신 TSet으로 O(1) Contains 보장 ──
    static const TSet<FGameplayTag> MaxFirstTags = {
        KatanaNeon::Data::Stats::MaxHealth,
        KatanaNeon::Data::Stats::MaxStamina,
        KatanaNeon::Data::Stats::MaxChronos,
        KatanaNeon::Data::Stats::MaxOverclockPoint
    };

    // ── 1단계: Max 어트리뷰트 먼저 처리 ──
    for (const FGameplayTag& Tag : MaxFirstTags)
    {
        const float* MagnitudePtr = Spec.SetByCallerTagMagnitudes.Find(Tag);
        if (!MagnitudePtr || FMath::IsNearlyZero(*MagnitudePtr)) continue;

        const FGameplayAttribute* AttrPtr = AttributeMap.Find(Tag);
        if (!AttrPtr || !AttrPtr->IsValid()) continue;

        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(*AttrPtr, EGameplayModOp::Additive, *MagnitudePtr));
    }

    // ── 2단계: 나머지 어트리뷰트 처리 ──
    for (const auto& [Tag, Magnitude] : Spec.SetByCallerTagMagnitudes)
    {
        if (MaxFirstTags.Contains(Tag)) continue; // O(1)
        if (!Tag.IsValid() || FMath::IsNearlyZero(Magnitude)) continue;

        const FGameplayAttribute* AttrPtr = AttributeMap.Find(Tag);
        if (!AttrPtr || !AttrPtr->IsValid()) continue;

        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(*AttrPtr, EGameplayModOp::Additive, Magnitude));
    }
}
#pragma endregion Execution Calculation 최적화 구현