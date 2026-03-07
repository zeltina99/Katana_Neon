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

    UE_LOG(LogTemp, Error, TEXT("[AttributeSet] 생성자 호출됨"));
    UE_LOG(LogTemp, Warning, TEXT("[AttributeSet] MovementSpeed BaseValue: %.1f"),
        MovementSpeed.GetBaseValue());
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 핵심 오버라이드 함수 구현
void UKNAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    // 값 변경 전 최대/최소치 클램핑
    // MaxValue가 0이면 아직 초기화 전이므로 Clamp를 건너뜁니다.
    if (Attribute == GetHealthAttribute() && GetMaxHealth() > 0.0f)
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    else if (Attribute == GetStaminaAttribute() && GetMaxStamina() > 0.0f)
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
    }
    else if (Attribute == GetChronosAttribute() && GetMaxChronos() > 0.0f)
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxChronos());
    }
    else if (Attribute == GetOverclockPointAttribute() && GetMaxOverclockPoint() > 0.0f)
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxOverclockPoint());
    }
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
        UE_LOG(LogTemp, Error, TEXT("[PostGEExec] Health 적용 시점 / Health: %.1f / MaxHealth: %.1f"),
            GetHealth(), GetMaxHealth());
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
        UE_LOG(LogTemp, Error, TEXT("[PostGEExec] Health 보정 후: %.1f"), GetHealth());
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
        UE_LOG(LogTemp, Warning, TEXT("[AttributeSet] MovementSpeed 변경: Magnitude=%.1f / 현재값=%.1f / 원인GE=%s"),
            Data.EvaluatedData.Magnitude,
            GetMovementSpeed(),
            *Data.EffectSpec.Def->GetName()); // ← 어떤 GE가 호출했는지
    }
}
void UKNAttributeSet::PostAttributeChange(
    const FGameplayAttribute& Attribute,
    float OldValue,
    float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

    // UE5.5 권장 방식: Max 어트리뷰트가 확정된 직후 현재값을 재보정합니다.
    // GAS는 GE 내 Modifier를 한꺼번에 적용하므로 PostGameplayEffectExecute 시점에
    // MaxHealth가 아직 0일 수 있습니다. PostAttributeChange는 개별 어트리뷰트가
    // 완전히 확정된 뒤 호출되므로 Max → 현재값 순서 문제를 완전히 우회합니다.
    if (Attribute == GetMaxHealthAttribute())
    {
        OldValue <= 0.0f ? SetHealth(NewValue) : SetHealth(FMath::Clamp(GetHealth(), 0.0f, NewValue));
    }
    else if (Attribute == GetMaxStaminaAttribute())
    {
        OldValue <= 0.0f ? SetStamina(NewValue) : SetStamina(FMath::Clamp(GetStamina(), 0.0f, NewValue));
    }
    else if (Attribute == GetMaxChronosAttribute())
    {
        OldValue <= 0.0f ? SetChronos(NewValue) : SetChronos(FMath::Clamp(GetChronos(), 0.0f, NewValue));
    }
    else if (Attribute == GetMaxOverclockPointAttribute())
    {
        OldValue <= 0.0f ? SetOverclockPoint(0.0f) : SetOverclockPoint(FMath::Clamp(GetOverclockPoint(), 0.0f, NewValue));
    }
    else if (Attribute == GetMovementSpeedAttribute())
    {
        // 언리얼 엔진 5.5 규격: TWeakObjectPtr를 사용하여 댕글링 포인터 참조를 방지합니다.
        TWeakObjectPtr<AActor> OwnerActorPtr = GetOwningActor();
        if (OwnerActorPtr.IsValid() && OwnerActorPtr != nullptr)
        {
            // 상단에 선언하신 GameFramework/Character.h 를 활용하여 캐스팅
            ACharacter* Character = Cast<ACharacter>(OwnerActorPtr.Get());
            if (Character != nullptr)
            {
                UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
                if (MovementComp != nullptr)
                {
                    // GAS의 스탯 값(예: 650)을 실제 캐릭터 이동 속도에 덮어씌웁니다.
                    MovementComp->MaxWalkSpeed = NewValue;
                }
            }
        }
    }
}
#pragma endregion GAS 핵심 오버라이드 함수 구현