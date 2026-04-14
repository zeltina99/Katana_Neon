// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "KNAbilityComboAttack.generated.h"

#pragma region 전방 선언
class UKNStatsComponent;
class UAbilityTask_PlayMontageAndWait;
#pragma endregion 전방 선언

#pragma region 열거형 데이터
/**
 * @enum    EKNComboAttackType
 * @brief   콤보 공격의 타입을 정의하여 매직 넘버를 방지합니다.
 */
UENUM(BlueprintType)
enum class EKNComboAttackType : uint8
{
    Light = 0 UMETA(DisplayName = "약공격"),
    Heavy = 1 UMETA(DisplayName = "강공격")
};
#pragma endregion 열거형 데이터

/**
 * @file    KNAbilityComboAttack.h
 * @class   UKNAbilityComboAttack
 * @brief   플레이어 약공격(Light) / 강공격(Heavy) 5단계 콤보 어빌리티입니다.
 * @details MVC 및 SRP 패턴을 준수하여 로직과 데이터를 엄격히 분리합니다.
 *
 * [콤보 트리 – DataTable 행 키 규약]
 * - 약공격 : LightAttack_1 ~ LightAttack_5
 * - 강공격 : HeavyAttack_1 ~ HeavyAttack_5
 *
 * [입력 버퍼링 흐름]
 * - 비활성 상태 → TryActivate → ActivateAbility (1단계 시작)
 * - 활성 상태   → Controller가 BufferNextInput 직접 호출
 * - AN_ComboWindowOpen 도달 → 버퍼 확인 → 즉시 AdvanceCombo 또는 윈도우 오픈
 *
 * [ComboWindowTime 의미]
 * - > 0.0f : 다음 입력 대기 창이 열림
 * - = 0.0f : 피니셔(Finisher) — Heavy 또는 Light 5단계 종료
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
     * @brief 몽타주 재생 중 입력을 버퍼에 저장합니다.
     * @details Controller에서 콤보 어빌리티가 활성화 상태일 때 호출됩니다.
     * @param bIsHeavy 강공격 입력인지 여부
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Ability|Combo")
    void BufferNextInput(bool bIsHeavy);

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

    /**
     * @brief 콤보가 비활성 상태일 때 첫 타를 Heavy로 예약합니다.
     * @details Input_HeavyAttack에서 TryActivate 직전에 호출됩니다.
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Ability|Combo")
    void PrepareHeavyStart();
#pragma endregion 블루프린트 / AnimNotify 연동 인터페이스

#pragma region 에디터 설정 데이터
protected:
    /**
     * @brief 발도 상태 콤보 DataTable.
     * @details 행 구조: FKNComboAttackRow / 행 키: LightAttack_1~5, HeavyAttack_1~5
     *          각 행의 ComboMontage 필드에 발도 전용 몽타주를 직접 할당합니다.
     */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Ability|Combo|DataTable")
    TObjectPtr<UDataTable> DrawnComboDataTable = nullptr;

    /**
     * @brief 납도 상태 콤보 DataTable.
     * @details 행 구조: FKNComboAttackRow / 행 키: LightAttack_1~5, HeavyAttack_1~5
     *          각 행의 ComboMontage 필드에 납도 전용 몽타주를 직접 할당합니다.
     */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Ability|Combo|DataTable")
    TObjectPtr<UDataTable> SheathComboDataTable = nullptr;

    /**
     * @brief 히트박스 판정 시 적용할 데미지 Instant GE 클래스.
     * @details SetByCaller(Health, -최종데미지)로 적용됩니다.
     */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Ability|Combo|GAS")
    TSubclassOf<UGameplayEffect> DamageGEClass = nullptr;

    /**
     * @brief 기준 공격력. DataTable의 DamageMultiplier와 곱해져 최종 데미지가 됩니다.
     */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Ability|Combo|Config",
        meta = (ClampMin = 1.0f))
    float BaseAttackDamage = 10.0f;

    /** @brief 스태미나 소모 Instant GE 클래스 */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Ability|Combo|GAS")
    TSubclassOf<UGameplayEffect> StaminaCostGEClass = nullptr;

    /** @brief 오버클럭 1단계 수치 행 핸들 (데미지 배율 조회용) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Combo|Overclock")
    FDataTableRowHandle OverclockLv1RowHandle;

    /** @brief 오버클럭 3단계 수치 행 핸들 (시간 정지 데미지 배율 조회용) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|Combo|Overclock")
    FDataTableRowHandle OverclockLv3RowHandle;
#pragma endregion 에디터 설정 데이터

#pragma region 런타임 콤보 상태
private:
    /** @brief 현재 재생 중인 몽타주 태스크 (이전 태스크 Delegate 해제용으로 필수!) */
    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask = nullptr;

    /** @brief 현재 진행 중인 콤보 단계 (1 ~ 5, 0 = 비활성) */
    int32 CurrentComboStep = 0;

    /** @brief 데이터 일관성을 위해 int32 대신 Enum 사용 */
    EKNComboAttackType CurrentAttackType = EKNComboAttackType::Light;

    /** @brief 다음 입력 수신 창 활성 여부 */
    bool bComboWindowOpen = false;

    /** @brief 다음 단계가 강공격으로 예약되었는지 여부 */
    bool bNextIsHeavy = false;

    /**
     * @brief 이번 콤보가 발도 스탠스로 시작되었는지 여부.
     * @details ActivateAbility 첫 진입 시 WeaponDrawn 태그로 확정되며,
     *          콤보 종료(EndAbility)까지 변경되지 않습니다.
     */
    bool bIsDrawnCombo = false;

    /** @brief 콤보 윈도우 만료 타이머 핸들 */
    FTimerHandle ComboWindowTimerHandle;

    /** @brief 어빌리티 최대 지속 시간 안전망 타이머 — 모든 경로에서 EndAbility 미호출 시 강제 종료 */
    FTimerHandle SafetyTimerHandle;

    /** @brief 현재 단계의 DataTable 데이터 캐시 */
    FKNComboAttackRow CachedComboRow;

    /** @brief 콤보 윈도우가 열리기 전에 입력이 들어왔을 때 버퍼링합니다. */
    bool bInputBuffered = false;

    /** @brief 버퍼링된 입력이 Heavy인지 여부 */
    bool bBufferedInputIsHeavy = false;
#pragma endregion 런타임 콤보 상태

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief 현재 스탠스에 해당하는 DataTable을 반환합니다.
     * @param bDrawn 발도 여부 (true = DrawnComboDataTable, false = SheathComboDataTable)
     * @return 해당 DataTable 포인터, 미할당 시 nullptr
     */
    UDataTable* GetComboDataTable(bool bDrawn) const;

    /**
     * @brief 현재 ASC의 WeaponDrawn 태그 유무로 발도 상태를 판별합니다.
     * @param ActorInfo 소유 액터 정보
     * @return 발도 상태이면 true
     */
    bool IsWeaponDrawn(const FGameplayAbilityActorInfo* ActorInfo) const;

    /**
     * @brief 단계·타입에서 DataTable 행 이름을 생성합니다.
     * @param Step       콤보 단계 (1 ~ 5)
     * @param AttackType 공격 타입 (EKNComboAttackType::Light 또는 Heavy)
     * @return "LightAttack_N" 또는 "HeavyAttack_N" 형태의 FName
     */
    FName MakeComboRowName(int32 Step, EKNComboAttackType AttackType) const;

    /**
     * @brief DataTable에서 지정 행을 읽어 CachedComboRow에 저장합니다.
     * @param RowName 조회할 행 이름
     * @return 로드 성공 여부
     */
    bool LoadComboRow(const FName& RowName);

    /**
     * @brief 현재 캐시된 StaminaCost만큼 스태미나를 GE로 즉시 소모합니다.
     * @return 소모 성공 여부
     */
    bool ConsumeStamina();

    /**
     * @brief CachedComboRow.ComboMontage를 PlayMontageAndWait Task로 재생합니다.
     *        PlayRate는 DT의 PlayRate × AttributeSet.AttackSpeed 로 계산합니다.
     * @return Task 생성 성공 여부
     */
    bool PlayComboMontage();

    /** @brief 다음 단계로 진행하거나, 최종 단계면 어빌리티를 종료합니다. */
    void AdvanceCombo();

    /** @brief 콤보 윈도우 만료 콜백 */
    UFUNCTION()
    void OnComboWindowExpired();

    /** @brief 몽타주가 완전히 종료되었을 때 호출됩니다. 어빌리티를 최종 종료합니다. */
    UFUNCTION()
    void OnMontageEnded();
#pragma endregion 내부 헬퍼 함수
};
