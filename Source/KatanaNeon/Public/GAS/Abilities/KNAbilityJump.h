// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "KNAbilityJump.generated.h"

#pragma region 전방 선언
class UAnimMontage;
class AKNCharacterBase;
class UCharacterMovementComponent;
class UAbilitySystemComponent;
class UGameplayEffect;
#pragma endregion 전방 선언

/**
 * @file    KNAbilityJump.h
 * @class   UKNAbilityJump
 * @brief   플레이어의 단일 점프 및 더블 점프 로직과 스태미나 소모를 처리하는 GAS 어빌리티입니다.
 *
 * @details
 * [아키텍처 맞춤형 설계]
 * - 엔진 기본 ACharacter가 아닌, 프로젝트 고유의 AKNCharacterBase를 기준으로 작동하도록 최적화되었습니다.
 * * [데이터 주도적 설계 (DDD)]
 * - 에디터에서 FDataTableRowHandle을 통해 기획자가 점프 수치(높이, 횟수, 기력 소모량)를 직접 주입합니다.
 * * [단일 점프 vs 더블 점프]
 * - 지상: CharacterMovementComponent::JumpZVelocity를 적용하고 기본 Jump()를 호출.
 * - 공중: State.Movement.DoubleJumped 태그가 없을 때 LaunchCharacter로 즉각적인 상승력 부여 및 몽타주 재생.
 */
UCLASS()
class KATANANEON_API UKNAbilityJump : public UGameplayAbility
{
    GENERATED_BODY()

#pragma region 기본 생성자 및 초기화
public:
    /**
     * @brief AbilityTag, InstancingPolicy, NetExecutionPolicy 기본값을 설정합니다.
     */
    UKNAbilityJump();
#pragma endregion 기본 생성자 및 초기화

#pragma region GAS 핵심 오버라이드
public:
    /**
     * @brief 활성화 조건 검사. 지상 점프 또는 (공중 + 더블 점프 가능) 상태일 때 허용합니다.
     * @param Handle               어빌리티 스펙 핸들
     * @param ActorInfo            소유 액터 정보
     * @param SourceTags           소스 태그 컨테이너
     * @param TargetTags           타겟 태그 컨테이너
     * @param OptionalRelevantTags 실패 원인 출력 태그 (옵션)
     * @return 스태미나가 충분하고 점프 가능한 상태면 true 반환
     */
    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr,
        const FGameplayTagContainer* TargetTags = nullptr,
        OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

    /**
     * @brief 점프 또는 더블 점프를 실행하고 스태미나를 소모합니다.
     * @param Handle           어빌리티 스펙 핸들
     * @param ActorInfo        소유 액터 정보
     * @param ActivationInfo   활성화 방식 정보
     * @param TriggerEventData 트리거 이벤트 데이터
     */
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    /**
     * @brief 어빌리티 종료 처리.
     * @param Handle                 어빌리티 스펙 핸들
     * @param ActorInfo              소유 액터 정보
     * @param ActivationInfo         활성화 방식 정보
     * @param bReplicateEndAbility   복제 여부
     * @param bWasCancelled          취소 여부
     */
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;
#pragma endregion GAS 핵심 오버라이드

#pragma region 에디터 설정 데이터 (DDD)
protected:
    /**
     * @brief DT_JumpSetting DataTable 행 핸들.
     * @details 하드코딩을 배제하고 에디터에서 기획자가 직접 점프 데이터를 지정합니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Jump|DataTable")
    FDataTableRowHandle JumpSettingRowHandle;

    /** @brief 스태미나 소모를 처리할 GameplayEffect 클래스 (Instant GE 권장) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Jump|Cost")
    TSubclassOf<UGameplayEffect> StaminaCostGEClass = nullptr;

    /** * @brief 더블 점프 시 재생할 애니메이션 몽타주.
     * @details nullptr이면 몽타주 재생 없이 이동 처리만 수행합니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Jump|Montage")
    TObjectPtr<UAnimMontage> DoubleJumpMontage = nullptr;
#pragma endregion 에디터 설정 데이터 (DDD)

#pragma region 런타임 상태
private:
    /** @brief DataTable에서 읽어와 런타임에 캐싱된 점프 수치 */
    FKNJumpSettingRow CachedJumpSetting;

    /** @brief 어빌리티 실행 중 비용 처리를 위해 로드 성공 여부를 추적합니다. */
    bool bSettingLoaded = false;
#pragma endregion 런타임 상태

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief JumpSettingRowHandle을 통해 기획 데이터를 읽어와 CachedJumpSetting에 저장합니다.
     * @return 로드 성공 여부
     */
    bool LoadJumpSetting();

    /**
     * @brief 스태미나를 즉시 소모합니다.
     * @param CostAmount 소모할 스태미나 수치
     * @return 소모 성공 여부
     */
    bool ConsumeStamina(float CostAmount);

    /**
     * @brief 1단계 점프를 수행합니다.
     * @details 범용 ACharacter가 아닌, 프로젝트 규격인 AKNCharacterBase를 사용합니다.
     * @param Character 대상 캐릭터 (AKNCharacterBase)
     * @param Movement  대상 이동 컴포넌트
     */
    void PerformGroundJump(AKNCharacterBase* Character, UCharacterMovementComponent* Movement);

    /**
     * @brief 더블 점프를 수행합니다.
     * @details 상승력 적용, 몽타주 재생, 그리고 공중 점프 소비 태그 처리를 포함합니다.
     * @param Character 대상 캐릭터 (AKNCharacterBase)
     * @param Movement  대상 이동 컴포넌트
     * @param ASC       소유자 AbilitySystemComponent
     */
    void PerformDoubleJump(AKNCharacterBase* Character, UCharacterMovementComponent* Movement, UAbilitySystemComponent* ASC);
#pragma endregion 내부 헬퍼 함수
};
