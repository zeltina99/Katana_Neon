// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/KNInfiniteModifier.h"
#include "GAS/Attributes/KNAttributeSet.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region Infinite Gameplay Effect 구현
UKNInfiniteModifier::UKNInfiniteModifier()
{
    // 명시적인 해제 명령(RemoveActiveGameplayEffect)이 있을 때까지 영구 지속됩니다.
    DurationPolicy = EGameplayEffectDurationType::Infinite;
    Modifiers.Empty(); // 기본 하드코딩 모디파이어는 비웁니다.

    Executions.Add(FGameplayEffectExecutionDefinition(
        UKNInfiniteModifierExecution::StaticClass()));
}
#pragma endregion Infinite Gameplay Effect 구현

// ────────────────────────────────────────────────────────────

#pragma region Execution Calculation 최적화 구현
UKNInfiniteModifierExecution::UKNInfiniteModifierExecution()
{
    // Snapshot = false : 장착 시점의 값이 아닌, 실시간 기반 어트리뷰트 증감을 위해 false를 유지합니다.
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

const TMap<FGameplayTag, FGameplayAttribute>& UKNInfiniteModifierExecution::GetCachedAttributeMap()
{
    // 정적(Static) 캐시를 통해 런타임 리플렉션 부하를 원천 차단합니다.
    static TMap<FGameplayTag, FGameplayAttribute> CachedMap;
    static bool bIsInitialized = false;

    if (!bIsInitialized)
    {
        for (TFieldIterator<FProperty> It(UKNAttributeSet::StaticClass()); It; ++It)
        {
            // 컴파일 에러를 방지하기 위해 const 키워드를 제외한 캐스팅을 수행합니다.
            if (FStructProperty* StructProp = CastField<FStructProperty>(*It))
            {
                if (StructProp->Struct->GetFName() == TEXT("GameplayAttributeData"))
                {
                    // 언리얼 엔진 네이밍 규칙에 맞춘 태그 합성
                    FString TagName = FString::Printf(TEXT("KatanaNeon.Data.Stats.%s"), *StructProp->GetName());
                    FGameplayTag MappedTag = FGameplayTag::RequestGameplayTag(FName(*TagName), false);

                    if (MappedTag.IsValid())
                    {
                        CachedMap.Add(MappedTag, FGameplayAttribute(StructProp));
                    }
                }
            }
        }
        bIsInitialized = true;
    }

    return CachedMap;
}

void UKNInfiniteModifierExecution::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
    const TMap<FGameplayTag, FGameplayAttribute>& AttributeMap = GetCachedAttributeMap();

    // SetByCaller로 유입된 기획 데이터를 순회 적용합니다.
    for (const auto& [Tag, Magnitude] : Spec.SetByCallerTagMagnitudes)
    {
        if (!Tag.IsValid() || FMath::IsNearlyZero(Magnitude))
        {
            continue;
        }

        // 최적화된 O(1) 검색
        const FGameplayAttribute* TargetAttrPtr = AttributeMap.Find(Tag);

        if (!TargetAttrPtr || !TargetAttrPtr->IsValid())
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[KNInfiniteModifierExec] 캐시 맵에 매핑되지 않은 태그: %s"), *Tag.ToString());
            continue;
        }

        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(*TargetAttrPtr, EGameplayModOp::Additive, Magnitude));
    }
}
#pragma endregion Execution Calculation 최적화 구현