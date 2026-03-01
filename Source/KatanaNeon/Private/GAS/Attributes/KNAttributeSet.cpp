// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Attributes/KNAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region 기본 생성자 및 초기화 구현
UKNAttributeSet::UKNAttributeSet()
{
    // 초기화 로직
    // 속성들의 기본값은 하드코딩하지 않고 데이터 주도적 설계를 따르기 위해 비워둡니다.
    // ASC(AbilitySystemComponent)에서 DefaultAttribute DataTable을 적용할 때 값이 덮어씌워집니다.
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 핵심 오버라이드 함수 구현
void UKNAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    // 값 변경 전 최대/최소치 클램핑
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
    else if (Attribute == GetOverclockPointAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxOverclockPoint());
    }
    // Modifier 적용 전 음수/0 방어 ──
    else if (Attribute == GetAttackSpeedAttribute())
    {
        NewValue = FMath::Max(0.1f, NewValue);
    }
}

void UKNAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    // GE 적용 후 최종 값 보정 및 외부 시스템 동기화
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
    }
    else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    {
        SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
    }
    else if (Data.EvaluatedData.Attribute == GetChronosAttribute())
    {
        SetChronos(FMath::Clamp(GetChronos(), 0.0f, GetMaxChronos()));
    }
    else if (Data.EvaluatedData.Attribute == GetOverclockPointAttribute())
    {
        SetOverclockPoint(FMath::Clamp(GetOverclockPoint(), 0.0f, GetMaxOverclockPoint()));
    }
    else if (Data.EvaluatedData.Attribute == GetAttackSpeedAttribute())
    {
        // 공속은 최소 0.1 이상 유지 (0이 되면 애니메이션 멈춤 방지)
        SetAttackSpeed(FMath::Max(0.1f, GetAttackSpeed()));
    }
    else if (Data.EvaluatedData.Attribute == GetMovementSpeedAttribute())
    {
        SetMovementSpeed(FMath::Max(0.0f, GetMovementSpeed()));

        // 이동 속도 변경 시 실제 캐릭터 컴포넌트에 즉각 동기화
        if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwningActor()))
        {
            if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
            {
                MovementComp->MaxWalkSpeed = GetMovementSpeed();
            }
        }
    }
}
#pragma endregion GAS 핵심 오버라이드 함수 구현