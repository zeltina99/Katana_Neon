// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/KNAbilityLockOn.h"
#include "Characters/Player/KNPlayerCharacter.h"
#include "AbilitySystemComponent.h"

#pragma region 초기화 및 설정 구현
UKNAbilityLockOn::UKNAbilityLockOn()
{
    // 언리얼 엔진 5 최신 버전 가이드라인에 맞춰 NonInstanced 대신 InstancedPerActor 사용 (C4996 경고 해결)
    // 어빌리티 인스턴싱 정책: 상태를 저장할 필요가 없는 토글 로직이지만, 엔진의 델리게이트 및 리플리케이션 안정성을 위해 액터당 1개의 인스턴스만 생성 및 유지합니다.
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // 이 어빌리티는 클라이언트 측의 즉각적인 조작감을 위해 로컬 예측(Local Predicted)으로 실행되도록 설정
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
#pragma endregion 초기화 및 설정 구현

#pragma region 어빌리티 생명주기 구현
void UKNAbilityLockOn::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // 방어적 프로그래밍: 포인터 유효성 검사
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 언리얼 스마트 포인터(TWeakObjectPtr) 패턴을 활용한 안전한 캐릭터 캐스팅
    TWeakObjectPtr<AKNPlayerCharacter> PlayerCharacterPtr = Cast<AKNPlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (PlayerCharacterPtr.IsValid())
    {
        // Model(Character)의 현재 락온 상태를 가져와서 반대로(Toggle) 설정
        const bool bCurrentLockOn = PlayerCharacterPtr->GetIsLockOn();
        PlayerCharacterPtr->SetLockOnState(!bCurrentLockOn);

        // TODO: 향후 타겟팅 액터 탐색(SphereTrace) 및 카메라 록온 연출 로직 연결
    }

    // 토글 동작 완료 후, 불필요한 메모리 및 틱 낭비를 막기 위해 즉시 어빌리티 종료
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
#pragma endregion 어빌리티 생명주기 구현
