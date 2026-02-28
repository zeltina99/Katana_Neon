// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "KNStatsComponent.generated.h"

#pragma region 전방 선언
class UAbilitySystemComponent;
class UKNAttributeSet;
class UGameplayEffect;
struct FActiveGameplayEffect;
struct FOnAttributeChangeData;

struct FKNBaseStatRow;
struct FKNActionCostRow;
struct FKNOverclockSettingRow;
#pragma endregion 전방 선언

#pragma region 델리게이트 선언
/** @brief 체력 변경 이벤트 (Current, Max) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKNHealthChanged, float, Current, float, Max);
/** @brief 스태미나 변경 이벤트 (Current, Max) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKNStaminaChanged, float, Current, float, Max);
/** @brief 크로노스 변경 이벤트 (Current, Max) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKNChronosChanged, float, Current, float, Max);
/** @brief 오버클럭 포인트 변경 이벤트 (Current, Max) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKNOverclockPointChanged, float, Current, float, Max);
/** @brief 오버클럭 레벨 변경 이벤트 (NewLevel: 0~3) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKNOverclockLevelChanged, int32, NewLevel);
#pragma endregion 델리게이트 선언 끝

/**
 * @class  UKNStatsComponent
 * @brief  KatanaNeon 플레이어의 GAS 스탯을 초기화하고 GE를 통해 관리하는 ActorComponent입니다.
 * @details 단일 책임 원칙에 따라, 값의 Clamp는 AttributeSet에 위임하고
 * 본 컴포넌트는 DataTable 기반의 초기화와 UI 동기화, 오버클럭 태그 관리에만 집중합니다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KATANANEON_API UKNStatsComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region 기본 생성자 및 초기화
public:
    UKNStatsComponent();

protected:
    virtual void BeginPlay() override;
#pragma endregion 기본 생성자 및 초기화 끝

#pragma region 외부에 노출할 UI 동기화 이벤트
public:
    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Stats|Event")
    FOnKNHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Stats|Event")
    FOnKNStaminaChanged OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Stats|Event")
    FOnKNChronosChanged OnChronosChanged;

    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Stats|Event")
    FOnKNOverclockPointChanged OnOverclockPointChanged;

    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Stats|Event")
    FOnKNOverclockLevelChanged OnOverclockLevelChanged;
#pragma endregion 외부에 노출할 UI 동기화 이벤트 끝

#pragma region 핵심 조작 및 초기화 인터페이스
public:
    /**
     * @brief GAS 초기화 진입점. ASC와 AttributeSet을 연결하고 DataTable 기반 기본값을 적용합니다.
     * @param InASC 오너 Actor의 UAbilitySystemComponent
     */
    void InitializeStatComponent(UAbilitySystemComponent* InASC);

    /**
     * @brief 오버클럭 게이지를 지정량만큼 증가시킵니다.
     * @param GainAmount 획득할 오버클럭 수치 (음수 무시)
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Overclock")
    void GainOverclockPoint(float GainAmount);

    /**
     * @brief 스킬 사용 시 지정된 오버클럭 레벨을 소비합니다.
     * @param Level 소비할 레벨 (1~3)
     * @return 소비 성공 여부
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Overclock")
    bool ConsumeOverclockLevel(int32 Level);
#pragma endregion 핵심 조작 및 초기화 인터페이스 끝

#pragma region 데이터 조회 (읽기 전용)
public:
    /** @brief 현재 오버클럭 레벨(0~3)을 GameplayTag 기반으로 반환합니다. */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Overclock")
    int32 GetCurrentOverclockLevel() const;
#pragma endregion 데이터 조회 끝

#pragma region 내부 콜백 및 헬퍼 함수
protected:
    /** @brief 오버클럭 포인트 변경 시 레벨 태그를 동기화하는 내부 콜백입니다. */
    void OnOverclockPointChangedInternal(const FOnAttributeChangeData& Data);

private:
    /** @brief DataTable 기반 기본 스탯을 즉시 적용하고 메모리를 확보합니다. */
    void ApplyBaseStats(const FKNBaseStatRow* BaseStatRow);

    /** @brief 현재 포인트에 맞춰 오버클럭 레벨 태그(Lv1~Lv3)를 정리합니다. */
    void SyncOverclockLevelTags(float CurrentPoint);

    /** @brief 단일 태그를 부여하거나 제거하는 편의 함수. */
    void SetGameplayTagActive(const FGameplayTag& Tag, bool bGrant);

    /** @brief 내부에서 Instant GE를 동적으로 생성해 수치를 즉시 변경합니다. */
    bool ApplyInstantGEInternal(const FGameplayTag& StatTag, float Delta);
#pragma endregion 내부 콜백 및 헬퍼 함수 끝

#pragma region GAS 참조 및 런타임 캐시 (스마트 포인터)
private:
    UPROPERTY(Transient)
    TObjectPtr<UAbilitySystemComponent> ASC = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UKNAttributeSet> AttributeSet = nullptr;

    // TODO: 실제 프로젝트의 구조체 헤더 include 여부에 따라 컴파일 가능 여부가 결정됩니다.
    // UPROPERTY() FKNActionCostRow ActionCost;
    // UPROPERTY() FKNOverclockSettingRow OverclockSetting;
#pragma endregion GAS 참조 및 런타임 캐시 끝

#pragma region 시스템 할당 데이터베이스
private:
    /** @brief 수치 조정을 위한 범용 즉시 적용 GE 클래스 */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|GAS|GE")
    TSubclassOf<UGameplayEffect> InstantGEClass = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|GAS|DataTable")
    FDataTableRowHandle BaseStatRowHandle;

    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|GAS|DataTable")
    FDataTableRowHandle ActionCostRowHandle;

    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|GAS|DataTable")
    FDataTableRowHandle OverclockSettingRowHandle;
#pragma endregion 시스템 할당 데이터베이스 끝
};
