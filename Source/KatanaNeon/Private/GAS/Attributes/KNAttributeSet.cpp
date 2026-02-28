// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Attributes/KNAttributeSet.h"
#include "GameplayEffectExtension.h"

#pragma region 기본 생성자 및 초기화 구현
UKNAttributeSet::UKNAttributeSet()
{
    // 초기화 로직
    // 속성들의 기본값은 하드코딩하지 않고 데이터 주도적 설계를 따르기 위해 비워둡니다.
    // ASC(AbilitySystemComponent)에서 DefaultAttribute DataTable을 적용할 때 값이 덮어씌워집니다.
}
#pragma endregion 기본 생성자 및 초기화 구현 끝

#pragma region GAS 핵심 오버라이드 함수 구현
void UKNAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    // 하드코딩 방지: 값이 변경되기 직전에 항상 각자의 Max 값에 맞추어 Clamp(제한) 시켜줍니다.
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    else if (Attribute == GetStaminaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
    }
    else if (Attribute == GetChronosAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxChronos());
    }
    // 오버클럭 포인트는 음수가 될 수 없도록 제한합니다.
    else if (Attribute == GetOverclockPointAttribute())
    {
        NewValue = FMath::Max(NewValue, 0.0f);
    }
}

void UKNAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    // GameplayEffect 적용 후, 후처리가 필요한 이벤트(예: 체력 0 이하로 사망)를 여기서 처리합니다.
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // 강제로 체력을 클램프하여 안전장치 마련
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

        if (GetHealth() <= 0.0f)
        {
            // @todo: 추후 ASC를 통해 'State.Dead' 등의 GameplayTag를 부여하여 데이터 드라이븐 사망 로직 구현
        }
    }
    else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    {
        SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
    }
    else if (Data.EvaluatedData.Attribute == GetChronosAttribute())
    {
        SetChronos(FMath::Clamp(GetChronos(), 0.0f, GetMaxChronos()));
    }
}
#pragma endregion GAS 핵심 오버라이드 함수 구현 끝