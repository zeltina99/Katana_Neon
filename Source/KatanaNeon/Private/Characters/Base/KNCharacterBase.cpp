// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Base/KNCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/KNAttributeSet.h"
#include "GAS/Components/KNStatsComponent.h"

#pragma region 기본 생성자 및 초기화 구현
AKNCharacterBase::AKNCharacterBase()
{
    // 매 프레임 Tick 업데이트를 기본적으로 비활성화하여 액션 게임의 CPU 연산을 최적화합니다.
    // (필요한 경우 파생 클래스나 특정 로직에서만 bCanEverTick = true로 변경하여 사용)
    PrimaryActorTick.bCanEverTick = false;

    // 1. 능력 시스템 컴포넌트(ASC) 생성
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

    // 2. 핵심 스탯 데이터 셋 생성
    AttributeSet = CreateDefaultSubobject<UKNAttributeSet>(TEXT("AttributeSet"));

    // 3. 기획 데이터(DataTable) 기반 스탯 매니저 컴포넌트 생성
    StatsComponent = CreateDefaultSubobject<UKNStatsComponent>(TEXT("StatsComponent"));
}

void AKNCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // 싱글 플레이어 게임이므로, 별도의 Possess 타이밍을 기다리지 않고 BeginPlay에서 즉시 GAS를 초기화합니다.
    if (AbilitySystemComponent)
    {
        // 액터가 시스템의 소유자(Owner)이자 화신(Avatar)임을 엔진에 명시적으로 알립니다.
        AbilitySystemComponent->InitAbilityActorInfo(this, this);

        // ASC 셋업이 끝난 후, 우리가 만든 스탯 컴포넌트를 가동하여 DataTable의 엑셀 수치들을 적용합니다.
        if (StatsComponent)
        {
            StatsComponent->InitializeStatComponent(AbilitySystemComponent);
        }
    }
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 인터페이스 구현
UAbilitySystemComponent* AKNCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}
#pragma endregion GAS 인터페이스 구현
