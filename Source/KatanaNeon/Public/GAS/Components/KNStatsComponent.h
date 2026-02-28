// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "KNStatsComponent.generated.h"

/**
 * @file    KNStatsComponent.h
 * @brief   KatanaNeon 플레이어의 핵심 스탯 및 이펙트를 관리하는 컴포넌트 헤더 파일입니다.
 * @details 데이터 테이블(DataTable)을 기반으로 스탯을 초기화하고, UI 갱신 델리게이트 브로드캐스트,
 * 그리고 시간제/무한 버프의 생명주기를 관리합니다.
 */

#pragma region 전방 선언
class UAbilitySystemComponent;
class UKNAttributeSet;
class UGameplayEffect;
struct FActiveGameplayEffect;
struct FOnAttributeChangeData;
#pragma endregion 전방 선언

#pragma region 델리게이트 선언
/**
 * @brief 체력 변경 시 UI 등 외부에 알리기 위한 델리게이트입니다.
 * @param Current 변경된 현재 체력
 * @param Max 최대 체력
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKNHealthChanged, float, Current, float, Max);
/**
 * @brief 스태미나 변경 시 발동되는 델리게이트입니다.
 * @param Current 변경된 현재 스태미나
 * @param Max 최대 스태미나
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKNStaminaChanged, float, Current, float, Max);
/**
 * @brief 크로노스(시간 감속) 게이지 변경 시 발동되는 델리게이트입니다.
 * @param Current 변경된 현재 크로노스 게이지
 * @param Max 최대 크로노스 게이지
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKNChronosChanged, float, Current, float, Max);
/**
 * @brief 오버클럭 누적 포인트 변경 시 발동되는 델리게이트입니다.
 * @param Current 변경된 현재 오버클럭 포인트
 * @param Max 오버클럭 포인트 최대치 (보통 300)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKNOverclockPointChanged, float, Current, float, Max);
/**
 * @brief 오버클럭 단계(Lv)가 상승하거나 하락할 때 발동되는 델리게이트입니다.
 * @param NewLevel 도달한 새로운 오버클럭 레벨 (0~3)
 */
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
    /** @brief UKNStatsComponent 기본 생성자. 틱(Tick)을 비활성화하여 성능을 최적화합니다. */
    UKNStatsComponent();

protected:
    /** @brief 게임 시작 시 호출되는 부모 클래스 오버라이드 함수입니다. */
    virtual void BeginPlay() override;
#pragma endregion 기본 생성자 및 초기화

#pragma region 외부에 노출할 UI 동기화 이벤트
public:
    /** @brief 체력 UI 위젯이 구독할 체력 변경 이벤트 디스패처 */
    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Stats|Event")
    FOnKNHealthChanged OnHealthChanged;

    /** @brief 스태미나 UI 위젯이 구독할 스태미나 변경 이벤트 디스패처 */
    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Stats|Event")
    FOnKNStaminaChanged OnStaminaChanged;

    /** @brief 크로노스 UI 위젯이 구독할 크로노스 변경 이벤트 디스패처 */
    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Stats|Event")
    FOnKNChronosChanged OnChronosChanged;

    /** @brief 오버클럭 게이지 바 UI가 구독할 포인트 변경 이벤트 디스패처 */
    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Stats|Event")
    FOnKNOverclockPointChanged OnOverclockPointChanged;

    /** @brief 스킬 아이콘 활성화/비활성화를 위해 UI가 구독할 레벨 변경 이벤트 디스패처 */
    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Stats|Event")
    FOnKNOverclockLevelChanged OnOverclockLevelChanged;
#pragma endregion 외부에 노출할 UI 동기화 이벤트

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
#pragma endregion 핵심 조작 및 초기화 인터페이스

#pragma region 데이터 조회
public:
    /**
     * @brief 현재 플레이어가 도달한 오버클럭 레벨(0~3)을 반환합니다.
     * @details 하드코딩된 포인트 연산 대신, 부여된 네이티브 GameplayTag를 기반으로 O(1) 속도로 검사합니다.
     * @return 현재 오버클럭 레벨 (0, 1, 2, 3)
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Overclock")
    int32 GetCurrentOverclockLevel() const;
#pragma endregion 데이터 조회

#pragma region 영구 및 시간제 버프 API
public:
    /**
     * @brief 영구(Infinite) GE를 적용하고, 추후 해제를 위해 핸들을 맵에 내부적으로 추적합니다.
     * @param StatTag 변경할 타겟 어트리뷰트의 네이티브 태그 (예: KatanaNeon::Data::Stats::MovementSpeed)
     * @param Delta 적용할 증감 수치
     * @param HandleKey 해당 버프를 식별하고 해제할 때 사용할 고유 관리 태그 키
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Stats|Buff")
    void ApplyInfiniteBuff(const FGameplayTag& StatTag, float Delta, const FGameplayTag& HandleKey);

    /**
     * @brief HandleKey로 등록되었던 영구(Infinite) GE를 맵에서 찾아 명시적으로 제거합니다.
     * @param HandleKey ApplyInfiniteBuff 호출 시 사용했던 동일한 고유 관리 태그 키
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Stats|Buff")
    void RemoveInfiniteBuff(const FGameplayTag& HandleKey);

    /**
     * @brief 일정 시간 동안만 유지되는 시간 제한(Duration) GE를 적용합니다.
     * @details 지정된 시간이 만료되면 GAS 엔진 내부 메커니즘에 의해 자동으로 롤백됩니다.
     * @param StatTag 변경할 타겟 어트리뷰트의 네이티브 태그
     * @param Delta 적용할 증감 수치
     * @param Duration 버프가 유지될 시간 (초 단위)
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Stats|Buff")
    void ApplyDurationBuff(const FGameplayTag& StatTag, float Delta, float Duration);
#pragma endregion 영구 및 시간제 버프 API

#pragma region 내부 콜백 및 헬퍼 함수
protected:
    /**
     * @brief 오버클럭 포인트 어트리뷰트가 변경될 때마다 레벨 태그를 동기화하기 위해 호출되는 내부 콜백입니다.
     * @param Data GAS 속성 변경에 대한 세부 정보를 담은 구조체
     */
    void OnOverclockPointChangedInternal(const FOnAttributeChangeData& Data);

private:
    /**
     * @brief DataTable 기반 기본 스탯을 즉시 적용하고 메모리를 확보합니다.
     * @param BaseStatRow 기본 스탯 데이터
     * @param OCRow 오버클럭 최대치 설정을 가져오기 위한 오버클럭 데이터
     */
    void ApplyBaseStats(const FKNBaseStatRow* BaseStatRow, const FKNOverclockSettingRow* OCRow);

    /**
     * @brief 현재 누적된 오버클럭 포인트 수치에 맞춰 오버클럭 레벨 태그(Lv1~Lv3)를 정리하고 부여합니다.
     * @param CurrentPoint 검사할 현재 오버클럭 포인트
     */
    void SyncOverclockLevelTags(float CurrentPoint);

    /**
     * @brief 단일 GameplayTag를 조건에 따라 플레이어 ASC에 부여하거나 제거하는 편의성 유틸리티 함수입니다.
     * @param Tag 제어할 대상 네이티브 GameplayTag
     * @param bGrant true일 경우 부여, false일 경우 제거
     */
    void SetGameplayTagActive(const FGameplayTag& Tag, bool bGrant);

    /**
     * @brief 내부적으로 Instant GE 객체를 동적으로 생성하여 지정된 어트리뷰트의 수치를 즉시 변경합니다.
     * @param StatTag 변경할 타겟 어트리뷰트 네이티브 태그
     * @param Delta 즉시 적용할 증감 수치
     * @return GE 스펙 적용이 성공적으로 이루어졌으면 true 반환
     */
    bool ApplyInstantGEInternal(const FGameplayTag& StatTag, float Delta);
#pragma endregion 내부 콜백 및 헬퍼 함수 끝

#pragma region 런타임 캐시 및 시스템 할당 데이터베이스
private:
    /** @brief 캐싱된 소유자 액터의 능력 시스템 컴포넌트(ASC). nullptr로 안전하게 초기화. */
    UPROPERTY(Transient)
    TObjectPtr<UAbilitySystemComponent> ASC = nullptr;

    /** @brief 캐싱된 플레이어 전용 어트리뷰트 셋. nullptr로 안전하게 초기화. */
    UPROPERTY(Transient)
    TObjectPtr<UKNAttributeSet> AttributeSet = nullptr;

    /** @brief 데이터 드라이븐 밸런싱을 위해 초기화 시 데이터 테이블에서 복사해둔 액션 비용 런타임 캐시 */
    UPROPERTY()
    FKNActionCostRow ActionCost;

    /** @brief 오버클럭 임계값 등 데이터 드라이븐 밸런싱을 위한 설정 런타임 캐시 */
    UPROPERTY()
    FKNOverclockSettingRow OverclockSetting;

    /** @brief 영구 버프(Infinite)를 외부에서 직접 해제할 수 있도록 발급된 GE 핸들을 추적하는 맵 */
    UPROPERTY()
    TMap<FGameplayTag, FActiveGameplayEffectHandle> InfiniteBuffHandles;

    /** @brief 즉시 적용할 범용 Instant Gameplay Effect 블루프린트 클래스 (에디터 할당용) */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|GAS|GE")
    TSubclassOf<UGameplayEffect> InstantGEClass = nullptr;

    /** @brief 영구 적용할 범용 Infinite Gameplay Effect 블루프린트 클래스 (에디터 할당용) */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|GAS|GE")
    TSubclassOf<UGameplayEffect> InfiniteGEClass = nullptr;

    /** @brief 시간 제한으로 적용할 범용 Duration Gameplay Effect 블루프린트 클래스 (에디터 할당용) */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|GAS|GE")
    TSubclassOf<UGameplayEffect> DurationGEClass = nullptr;

    /** @brief 초기 기본 스탯 데이터 테이블 행 핸들 (에디터 할당용) */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|GAS|DataTable")
    FDataTableRowHandle BaseStatRowHandle;

    /** @brief 액션 기력 소모량 데이터 테이블 행 핸들 (에디터 할당용) */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|GAS|DataTable")
    FDataTableRowHandle ActionCostRowHandle;

    /** @brief 오버클럭 획득/소모 데이터 테이블 행 핸들 (에디터 할당용) */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|GAS|DataTable")
    FDataTableRowHandle OverclockSettingRowHandle;
#pragma endregion 런타임 캐시 및 시스템 할당 데이터베이스
};
