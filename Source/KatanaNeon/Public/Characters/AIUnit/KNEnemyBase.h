// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Base/KNCharacterBase.h"
#include "Engine/DataTable.h"
#include "Data/Structs/KNEnemyStatTable.h"
#include "KNEnemyBase.generated.h"

#pragma region 전방 선언
class UBehaviorTree;
class UGameplayEffect;
#pragma endregion 전방 선언

#pragma region 델리게이트 선언
/**
 * @brief 공격 직전 예고 브로드캐스트 — 플레이어의 저스트 회피 윈도우 시작 트리거
 * @param WarningDuration 판정 가능 시간 (초)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKNEnemyAttackWarning, float, WarningDuration);
#pragma endregion 델리게이트 선언

/**
 * @class  AKNEnemyBase
 * @brief  KatanaNeon 모든 적 캐릭터의 공통 기반 클래스입니다.
 * @details  AI 감지/판단 로직은 AKNEnemyController로 위임하고,
 * 본 클래스는 GAS 기반의 스탯 초기화, 사망 처리, 데이터 테이블 연동에만 집중합니다.
 */
UCLASS()
class KATANANEON_API AKNEnemyBase : public AKNCharacterBase
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    AKNEnemyBase();

protected:
    virtual void BeginPlay() override;
#pragma endregion 기본 생성자 및 초기화

#pragma region AI 데이터 제공
public:
    /**
     * @brief 이 캐릭터가 소유한 비헤이비어 트리를 반환합니다.
     * @details AI Controller가 빙의(Possess)할 때 호출하여 트리 실행에 사용합니다.
     */
    UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

protected:
    /** @brief 이 적이 사용할 비헤이비어 트리 에셋입니다. (에디터 할당) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|AI")
    TObjectPtr<UBehaviorTree> BehaviorTree = nullptr;
#pragma endregion AI 데이터 제공

#pragma region 공격 예고 시스템 (저스트 회피 연동)
public:
    /** @brief 공격 예고 이벤트 — 애니메이션 노티파이 또는 BT 태스크에서 호출합니다. */
    UPROPERTY(BlueprintAssignable, Category = "KatanaNeon|Enemy|Event")
    FOnKNEnemyAttackWarning OnAttackWarning;

    /**
     * @brief 공격 직전 예고를 브로드캐스트합니다.
     * @details 애님 노티파이 또는 BT에서 호출 → 플레이어 FlurryRush 판정 윈도우 시작.
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Enemy")
    void BroadcastAttackWarning();
#pragma endregion 공격 예고 시스템

#pragma region 사망 처리 오버라이드
protected:
    /**
     * @brief 적 전용 사망 처리. AI 컨트롤러 해제 및 Ragdoll 전환을 수행합니다.
     */
    virtual void Die() override;
#pragma endregion 사망 처리 오버라이드

#pragma region 데이터 테이블 및 런타임 캐시
protected:
    /** @brief 이 적의 기본 스탯 DataTable 행 핸들 (에디터 할당) */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Enemy|DataTable")
    FDataTableRowHandle EnemyStatRowHandle;

    /** @brief 런타임에 캐싱된 적 스탯 구조체 */
    FKNEnemyBaseStatRow CachedEnemyStat;

    /** @brief 스탯 초기화용 Instant GE 클래스 (에디터 할당) */
    UPROPERTY(EditDefaultsOnly, Category = "KatanaNeon|Enemy|GAS")
    TSubclassOf<UGameplayEffect> InitStatGEClass = nullptr;

private:
    /**
     * @brief DataTable에서 스탯을 로드하고 Instant GE를 통해 어트리뷰트를 초기화합니다.
     */
    void ApplyEnemyBaseStats();
#pragma endregion 데이터 테이블 및 런타임 캐시
};
