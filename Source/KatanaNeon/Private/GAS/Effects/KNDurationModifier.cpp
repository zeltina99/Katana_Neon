// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/KNDurationModifier.h"
#include "GAS/Attributes/KNAttributeSet.h"
#include "GAS/Tags/KNStatsTags.h"
#include "GAS/System/KNGASAttributeCache.h"

#pragma region Duration Gameplay Effect 구현
UKNDurationModifier::UKNDurationModifier()
{
    // 지정된 Duration 동안만 효과 유지 후 '자동 해제(롤백)' 됨
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    Modifiers.Empty(); // 기본 하드코딩 모디파이어는 비웁니다.

    // UPROPERTY 시스템을 거치지 않으므로 규정 외의 값(0.0f)으로 명시적 세팅
    Period = FScalableFloat(0.0f);

    Executions.Add(FGameplayEffectExecutionDefinition(
        UKNDurationModifierExecution::StaticClass()));
}
#pragma endregion Duration Gameplay Effect 구현

// ────────────────────────────────────────────────────────────

#pragma region Execution Calculation 최적화 구현
UKNDurationModifierExecution::UKNDurationModifierExecution()
{
    // Snapshot = false : 버프 적용 시점의 과거 값이 아닌, 실시간 현재 값을 기준으로 캡처합니다.
    for (TFieldIterator<FProperty> It(UKNAttributeSet::StaticClass()); It; ++It)
    {
        // 컴파일 에러 원인이었던 const 키워드 제거
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

void UKNDurationModifierExecution::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
    const TMap<FGameplayTag, FGameplayAttribute>& AttributeMap = FKNGASAttributeCache::Get();

    // SetByCallerTagMagnitudes 순회하여 데이터 주도적 수치 적용
    for (const auto& [Tag, Magnitude] : Spec.SetByCallerTagMagnitudes)
    {
        // 유효하지 않거나 0인 수치는 무시하여 불필요한 Modifier 생성을 방지합니다.
        if (!Tag.IsValid() || FMath::IsNearlyZero(Magnitude))
        {
            continue;
        }

        // 최적화: 리플렉션 대신 O(1) 해시 맵 검색을 수행합니다.
        const FGameplayAttribute* TargetAttrPtr = AttributeMap.Find(Tag);

        if (!TargetAttrPtr || !TargetAttrPtr->IsValid())
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[KNDurationModifierExec] 캐시 맵에 매핑되지 않은 태그: %s"), *Tag.ToString());
            continue;
        }

        // Additive 방식으로 어트리뷰트 수정을 출력에 추가합니다.
        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(*TargetAttrPtr, EGameplayModOp::Additive, Magnitude));
    }
}
#pragma endregion Execution Calculation 최적화 구현