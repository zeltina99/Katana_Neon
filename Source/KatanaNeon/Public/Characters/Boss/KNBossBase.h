// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AIUnit/KNEnemyBase.h"
#include "Engine/DataTable.h"
#include "Data/Structs/KNEnemyStatTable.h"
#include "KNBossBase.generated.h"

#pragma region 델리게이트 선언
/**
 * @brief 보스 페이즈 전환 시 외부(UI, 카메라 연출)에 알리는 델리게이트
 * @param NewPhase 전환된 새 페이즈 인덱스 (0부터 시작)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKNBossPhaseChanged, int32, NewPhase);
#pragma endregion 델리게이트 선언

#pragma region 전방 선언
struct FOnAttributeChangeData;
#pragma endregion 전방 선언

/**
 * @class  AKNBossBase
 * @brief  모든 보스 캐릭터의 공통 기반. 페이즈 전환 시스템을 제공합니다.
 * @details DataTable의 PhaseHealthThresholds 배열을 읽어 체력 비율에 따라
 * 자동으로 페이즈를 전환하고, 페이즈별 스탯 배율을 적용합니다.
 */
UCLASS()
class KATANANEON_API AKNBossBase : public AKNEnemyBase
{
    GENERATED_BODY()

#pragma region 기본 생성자 및 초기화
public:
    AKNBossBase();

protected:
    virtual void BeginPlay() override;
#pragma endregion 기본 생성자 및 초기화

#pragma region 페이즈 시스템
public:
    /** @brief 페이즈 전환 시 외부 시스템이 구독할 델리게이트 */
    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Boss|Event")
    FOnKNBossPhaseChanged OnPhaseChanged;

    /**
     * @brief 현재 보스 페이즈 인덱스를 반환합니다.
     * @return 현재 페이즈 (0 = 초기 상태)
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Boss")
    int32 GetCurrentPhase() const { return CurrentPhaseIndex; }

protected:
    /**
     * @brief 체력 변경 시 호출하여 페이즈 전환 조건을 확인합니다.
     * @details AttributeSet의 PostGameplayEffectExecute 이후 연결하거나 ASC 델리게이트로 구독하세요.
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Boss")
    void CheckPhaseTransition();

    /**
     * @brief 실제 페이즈 전환 처리. 파생 클래스에서 오버라이드하여 연출을 추가합니다.
     * @param NewPhaseIndex 전환될 페이즈 인덱스
     */
    virtual void OnPhaseTransition(int32 NewPhaseIndex);

private:
    /**
     * @brief ASC 어트리뷰트 변경 델리게이트에 AddUObject로 연결되는 내부 콜백입니다.
     * @details 람다 캡처([this]) 대신 UObject 생명주기를 추적하여 크래시를 원천 차단합니다.
     * @param Data GAS 속성 변경에 대한 세부 정보를 담은 구조체
     */
    void OnHealthChangedForPhase(const FOnAttributeChangeData& Data);

    /** @brief 타이머에 의해 호출되어 페이즈 전환 잠금을 해제합니다. (크래시 방지용) */
    void UnlockPhaseTransition();
#pragma endregion 페이즈 시스템

#pragma region 보스 데이터 테이블
protected:
    /** @brief 페이즈 수치 DataTable 행 핸들 (에디터 할당) */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Boss|DataTable")
    FDataTableRowHandle BossPhaseRowHandle;

    /** @brief 런타임 캐싱된 페이즈 설정 */
    FKNBossPhaseRow CachedPhaseData;
#pragma endregion 보스 데이터 테이블

#pragma region 사망 처리 오버라이드
protected:
    /** @brief 보스 사망 시 진행 중인 페이즈 전환 타이머를 안전하게 정리합니다. */
    virtual void Die() override;
#pragma endregion 사망 처리 오버라이드 끝

#pragma region 런타임 페이즈 상태
private:
    /** @brief 현재 활성 페이즈 인덱스 */
    int32 CurrentPhaseIndex = 0;

    /** @brief 페이즈 전환 연출 중 중복 전환 방지 플래그 */
    bool bIsTransitioning = false;

    /** @brief 페이즈 전환 타이머 핸들 (사망 시 취소를 위해 멤버 변수로 승격) */
    FTimerHandle TransitionHandle;
#pragma endregion 런타임 페이즈 상태
};