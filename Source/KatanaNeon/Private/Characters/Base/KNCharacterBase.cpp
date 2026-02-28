// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Base/KNCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/CapsuleComponent.h"
#include "GAS/Attributes/KNAttributeSet.h"

#pragma region 기본 생성자 및 초기화 구현
AKNCharacterBase::AKNCharacterBase()
{
    // 매 프레임 Tick 업데이트를 기본적으로 비활성화하여 액션 게임의 CPU 연산을 최적화합니다.
    PrimaryActorTick.bCanEverTick = false;

    // 1. 능력 시스템 컴포넌트(ASC) 생성
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

    // 2. 핵심 스탯 데이터 셋 생성
    AttributeSet = CreateDefaultSubobject<UKNAttributeSet>(TEXT("AttributeSet"));
}

void AKNCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // 싱글 플레이어 게임이므로 BeginPlay에서 즉시 GAS를 초기화합니다.
    if (ensure(AbilitySystemComponent))
    {
        // 액터가 시스템의 소유자(Owner)이자 화신(Avatar)임을 엔진에 명시적으로 알립니다.
        AbilitySystemComponent->InitAbilityActorInfo(this, this);

        // 등록된 기본 어빌리티들(점프, 기본 공격 등)을 부여합니다.
        GiveDefaultAbilities();
    }
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 인터페이스 구현
UAbilitySystemComponent* AKNCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}
#pragma endregion GAS 인터페이스 구현

#pragma region 사망 처리 구현
void AKNCharacterBase::Die()
{
    // 중복 사망 이벤트를 방지하기 위해 캡슐과 메시의 콜리전을 비활성화합니다.
    if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
    {
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 외부 시스템(UI, 퀘스트 매니저 등)에 사망 사실을 알립니다.
    OnCharacterDeath.Broadcast(this);

    // 구체적인 AI 정지나 래그돌(Ragdoll) 전환 로직은 각각의 파생 클래스에서 처리합니다.
}
#pragma endregion 사망 처리 구현

#pragma region 어빌리티 부여 구현
void AKNCharacterBase::GiveDefaultAbilities()
{
    if (!AbilitySystemComponent) return;

    // 에디터에서 등록한 기본 어빌리티 목록을 순회하며 ASC에 부여합니다.
    for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
    {
        if (AbilityClass)
        {
            // 어빌리티를 레벨 1로 부여합니다. 
            // 1주차 프로토타입 단계이므로 입력 ID(InputID)는 Enhanced Input 방식에 맞춰 INDEX_NONE으로 처리합니다.
            AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
        }
    }
}
#pragma endregion 어빌리티 부여 구현
