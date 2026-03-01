// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataTable.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "KNAbilityComboAttack.generated.h"

#pragma region 전방 선언
class UAnimMontage;
class UKNStatsComponent;
class UAbilityTask_PlayMontageAndWait;
struct FKNComboAttackRow;
#pragma endregion 전방 선언

/**
 * @file    KNAbility_ComboAttack.h
 * @class   UKNAbility_ComboAttack
 * @brief   플레이어 약공격(Light) / 강공격(Heavy) 5단계 콤보 어빌리티입니다.
 *
 * @details
 * [콤보 트리 – DT_PlayerComboAttack 행 키 규약]
 * - 약공격  : LightAttack_1 ~ LightAttack_5  (AttackType = 0)
 * - 강공격  : HeavyAttack_1 ~ HeavyAttack_5  (AttackType = 1)
 *
 * [콤보 입력 경로 예시]
 * - L → L → L → H → H  :  약1 → 약2 → 약3 → 강4 → 강5
 * - H (1단계)            :  즉시 강공1 (ComboWindowTime=0 → 콤보 종료)
 *
 * [ComboWindowTime 의미]
 * - > 0.0f : 다음 입력 대기 창이 열림
 * - = 0.0f : 피니셔(Finisher) — 다음 입력 없이 어빌리티 종료
 *
 * [SRP 책임 분리]
 * - 히트박스 판정 : AnimNotify → ActivateHitbox() 콜백
 * - 스태미나 소모 : KNInstantModifier GE 경유
 * - 오버클럭 획득 : KNStatsComponent::GainOverclockPoint() 위임
 * - 몽타주 재생   : UAbilityTask_PlayMontageAndWait Task 위임
 */
UCLASS()
class KATANANEON_API UKNAbilityComboAttack : public UGameplayAbility
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    /**
     * @brief 어빌리티 기본값 초기화.
     * @details AbilityTag, InstancingPolicy, NetExecutionPolicy를 설정합니다.
     */
    UKNAbilityComboAttack();
#pragma endregion 기본 생성자 및 초기화

#pragma region GAS 핵심 오버라이드
public:
    /**
     * @brief 활성화 조건 검사.
     * @details 콤보 윈도우 진행 중이거나, 스태미나가 1단계 비용 이상이면 허용합니다.
     * @param Handle            어빌리티 스펙 핸들
     * @param ActorInfo         소유 액터 정보
     * @param SourceTags        소스 태그 컨테이너 (옵션)
     * @param TargetTags        타겟 태그 컨테이너 (옵션)
     * @param OptionalRelevantTags 실패 원인 출력 태그 (옵션)
     * @return 활성화 가능하면 true
     */
    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr,
        const FGameplayTagContainer* TargetTags = nullptr,
        OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

    /**
     * @brief 어빌리티 활성화.
     * @details 콤보 윈도우가 열려 있으면 다음 단계로 진행하고,
     *          아니면 1단계부터 시작합니다.
     * @param Handle            어빌리티 스펙 핸들
     * @param ActorInfo         소유 액터 정보
     * @param ActivationInfo    활성화 방식 정보
     * @param TriggerEventData  트리거 이벤트 데이터 (nullptr 가능)
     */
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    /**
     * @brief 어빌리티 종료.
     * @details 콤보 상태를 초기화하고 타이머를 해제합니다.
     * @param Handle                    어빌리티 스펙 핸들
     * @param ActorInfo                 소유 액터 정보
     * @param ActivationInfo            활성화 방식 정보
     * @param bReplicateEndAbility      복제 여부
     * @param bWasCancelled             취소 여부
     */
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;
#pragma endregion GAS 핵심 오버라이드

#pragma region 블루프린트 / AnimNotify 연동 인터페이스
public:
    /**
     * @brief AnimNotify(HitboxOpen)에서 호출 — 히트박스 구체 판정을 실행합니다.
     * @details CachedComboRow.DamageMultiplier × BaseDamage 의 데미지 GE를 적중 대상에게 적용합니다.
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Ability|Combo")
    void ActivateHitbox();

    /**
     * @brief AnimNotify(ComboWindowOpen)에서 호출 — 다음 입력 수신 창을 엽니다.
     * @details CachedComboRow.ComboWindowTime 초 뒤에 창이 닫히고 어빌리티가 종료됩니다.
     *          ComboWindowTime == 0.0f 이면 즉시 종료합니다.
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Ability|Combo")
    void OpenComboWindow();

    /**
     * @brief KNPlayerController의 Input_HeavyAttack 에서 SendGameplayEventToActor로 호출합니다.
     * @details 콤보 윈도우가 열려 있을 때 다음 단계를 강공격으로 예약합니다.
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Ability|Combo")
    void RequestHeavyAttack();
#pragma endregion 블루프린트 / AnimNotify 연동 인터페이스

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 약공격 몽타주 배열 (인덱스 0 = 1단계 … 4 = 5단계).
     * @details LightAttack_N 행의 ComboStep에 대응하는 몽타주를 에디터에서 할당합니다.
     */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Ability|Combo|Montage")
    TArray<TObjectPtr<UAnimMontage>> LightAttackMontages;

    /**
     * @brief 강공격 몽타주 배열 (인덱스 0 = 1단계 … 4 = 5단계).
     * @details HeavyAttack_N 행의 ComboStep에 대응하는 몽타주를 에디터에서 할당합니다.
     */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Ability|Combo|Montage")
    TArray<TObjectPtr<UAnimMontage>> HeavyAttackMontages;

    /**
     * @brief DT_PlayerComboAttack 에셋을 연결하는 DataTable 행 핸들.
     * @details 에디터에서 DataTable 에셋과 행 이름을 직접 선택합니다.
     *          (행 이름은 런타임에 MakeComboRowName()으로 동적 생성됩니다.)
     */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Ability|Combo|DataTable")
    TObjectPtr<UDataTable> ComboDataTable = nullptr;

    /**
     * @brief 히트박스 판정 시 적용할 데미지 Instant GE 클래스.
     * @details KNInstantModifier 기반. SetByCaller(Health, -데미지)로 적용됩니다.
     */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Ability|Combo|GAS")
    TSubclassOf<UGameplayEffect> DamageGEClass = nullptr;

    /**
     * @brief 기준 공격력. DataTable의 DamageMultiplier와 곱해져 최종 데미지가 됩니다.
     * @details 기본값 10.0f. 기획자가 에디터에서 조정합니다.
     */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Ability|Combo|Config",
        meta = (ClampMin = 1.0f))
    float BaseAttackDamage = 10.0f;

    /**
     * @brief 스태미나 소모 Instant GE 클래스.
     * @details SetByCaller(Stamina, -StaminaCost)로 즉시 소모합니다.
     */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Ability|Combo|GAS")
    TSubclassOf<UGameplayEffect> StaminaCostGEClass = nullptr;
#pragma endregion 에디터 설정 데이터

#pragma region 런타임 콤보 상태
private:
    /** @brief 현재 진행 중인 콤보 단계 (1 ~ 5, 0 = 비활성) */
    int32 CurrentComboStep = 0;

    /** @brief 현재 공격 유형 (0 = Light, 1 = Heavy) */
    int32 CurrentAttackType = 0;

    /** @brief 다음 입력 수신 창 활성 여부 */
    bool bComboWindowOpen = false;

    /** @brief 다음 단계가 강공격으로 예약되었는지 여부 */
    bool bNextIsHeavy = false;

    /** @brief 콤보 윈도우 만료 타이머 핸들 */
    FTimerHandle ComboWindowTimerHandle;

    /** @brief 현재 단계의 DataTable 데이터 캐시 */
    FKNComboAttackRow CachedComboRow;
#pragma endregion 런타임 콤보 상태

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief 단계·타입에서 DataTable 행 이름을 생성합니다.
     * @param Step       콤보 단계 (1 ~ 5)
     * @param AttackType 공격 타입 (0 = Light, 1 = Heavy)
     * @return "LightAttack_N" 또는 "HeavyAttack_N" 형태의 FName
     */
    FName MakeComboRowName(int32 Step, int32 AttackType) const;

    /**
     * @brief DataTable에서 지정 행을 읽어 CachedComboRow에 저장합니다.
     * @param RowName 조회할 행 이름
     * @return 로드 성공 여부
     */
    bool LoadComboRow(const FName& RowName);

    /**
     * @brief 현재 캐시된 StaminaCost만큼 스태미나를 GE로 즉시 소모합니다.
     * @return 소모 성공 여부 (스태미나 부족 시 false)
     */
    bool ConsumeStamina();

    /**
     * @brief 현재 단계의 몽타주를 PlayMontageAndWait Task로 재생합니다.
     * @return Task 생성 성공 여부
     */
    bool PlayComboMontage();

    /**
     * @brief 다음 단계로 진행하거나, 최종 단계면 어빌리티를 종료합니다.
     */
    void AdvanceCombo();

    /**
     * @brief 콤보 윈도우 만료 콜백 — 타이머로 바인딩됩니다.
     */
    UFUNCTION()
    void OnComboWindowExpired();
#pragma endregion 내부 헬퍼 함수
};
