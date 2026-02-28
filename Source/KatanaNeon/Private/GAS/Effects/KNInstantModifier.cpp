// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/KNInstantModifier.h"
#include "GAS/Attributes/KNAttributeSet.h"
#include "GAS/Tags/KNStatsTags.h"

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
#pragma endregion Instant Gameplay Effect 구현 끝

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

const TMap<FGameplayTag, FGameplayAttribute>& UKNInstantModifierExecution::GetCachedAttributeMap()
{
    // 정적(Static) 캐시를 통해 런타임 리플렉션 부하를 원천 차단하여 타격 프레임 드랍을 막습니다.
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
                    // "KatanaNeon.Data.Stats.Health" 형태의 태그 문자열 조합
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

void UKNInstantModifierExecution::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
    const TMap<FGameplayTag, FGameplayAttribute>& AttributeMap = GetCachedAttributeMap();

    // SetByCaller 전체를 순회하여 기획자가 넘긴 데이터만큼 즉시 증감시킵니다.
    for (const auto& [Tag, Magnitude] : Spec.SetByCallerTagMagnitudes)
    {
        // 유효하지 않거나 0이면 건너뛰어 불필요한 Modifier 연산을 아예 막아버립니다.
        if (!Tag.IsValid() || FMath::IsNearlyZero(Magnitude))
        {
            continue;
        }

        // 최적화: 캐시 맵에서 즉시 끄집어냅니다 (O(1) 속도).
        const FGameplayAttribute* TargetAttrPtr = AttributeMap.Find(Tag);

        if (!TargetAttrPtr || !TargetAttrPtr->IsValid())
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[KNInstantModifierExec] 캐시 맵에 매핑되지 않은 태그: %s"), *Tag.ToString());
            continue;
        }

        // Additive 수정 적용 (양수 = 회복/획득, 음수 = 피해/소모)
        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(*TargetAttrPtr, EGameplayModOp::Additive, Magnitude));
    }
}
#pragma endregion Execution Calculation 최적화 구현 끝