// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "KNBossController.generated.h"

#pragma region 전방 선언
class UBehaviorTree;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
#pragma endregion 전방 선언

/**
 * @file    KNBossController.h
 * @class   AKNBossController
 * @brief   보스 캐릭터 전용 AI 컨트롤러입니다.
 *
 * @details
 * [SRP 책임]
 * - 비헤이비어 트리 실행, 플레이어 감지(Perception), 블랙보드 키 갱신만 담당합니다.
 * - 전투 판단 로직은 비헤이비어 트리 태스크/서비스에 위임합니다.
 *
 * [블랙보드 키 규약]
 * - "TargetPlayer"  : Object(AActor) — 감지된 플레이어
 * - "bIsAttacking"  : Bool           — 공격 중 여부
 * - "CurrentPhase"  : Int            — 현재 페이즈 인덱스
 * - "bIsStunned"    : Bool           — 그로기(행동 불능) 여부
 * - "DistToPlayer"  : Float          — 플레이어까지 거리 (BTService 갱신)
 */
UCLASS()
class KATANANEON_API AKNBossController : public AAIController
{
	GENERATED_BODY()
	
#pragma region 기본 생성자 및 초기화
public:
    /** @brief Perception 컴포넌트와 시야 설정을 초기화합니다. */
    AKNBossController();

protected:
    /**
     * @brief 폰 빙의 시 블랙보드/비헤이비어 트리를 시작합니다.
     * @param InPawn 빙의 대상 폰
     */
    virtual void OnPossess(APawn* InPawn) override;

    /**
     * @brief 폰 빙의 해제 시 비헤이비어 트리를 정지합니다.
     * @param InPawn 빙의 해제 대상 폰
     */
    virtual void OnUnPossess() override;
#pragma endregion 기본 생성자 및 초기화

#pragma region 블랙보드 키 이름 상수
public:
    /** @brief 감지된 플레이어 액터 키 */
    static const FName BBKey_TargetPlayer;
    /** @brief 공격 중 여부 키 */
    static const FName BBKey_IsAttacking;
    /** @brief 현재 페이즈 인덱스 키 */
    static const FName BBKey_CurrentPhase;
    /** @brief 그로기(행동 불능) 여부 키 */
    static const FName BBKey_IsStunned;
    /** @brief 플레이어까지 거리 키 */
    static const FName BBKey_DistToPlayer;
#pragma endregion 블랙보드 키 이름 상수

#pragma region 블랙보드 갱신 인터페이스
public:
    /**
     * @brief TargetPlayer 블랙보드 키를 갱신합니다.
     * @param NewTarget 새로 감지된 플레이어 (nullptr = 감지 해제)
     */
    void SetTargetPlayer(AActor* NewTarget);

    /**
     * @brief CurrentPhase 블랙보드 키를 갱신합니다.
     * @details KNBossBase::OnPhaseTransition에서 호출됩니다.
     * @param NewPhase 새 페이즈 인덱스
     */
    void SetCurrentPhase(int32 NewPhase);

    /**
     * @brief bIsStunned 블랙보드 키를 갱신합니다.
     * @details 그로기 GE 적용/해제 시 호출됩니다.
     * @param bStunned 그로기 여부
     */
    void SetIsStunned(bool bStunned);
#pragma endregion 블랙보드 갱신 인터페이스

#pragma region Perception 콜백
private:
    /**
     * @brief 시야 감지 업데이트 콜백.
     * @details 플레이어가 시야에 들어오거나 사라질 때 블랙보드를 갱신합니다.
     * @param UpdatedActors 감지 상태가 변경된 액터 목록
     */
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
#pragma endregion Perception 콜백

#pragma region 런타임 상태
private:
    /** @brief 플레이어 시야 감지 전용 Perception 설정 */
    UPROPERTY(VisibleAnywhere, Category = "KatanaNeon|Boss|AI")
    TObjectPtr<UAISenseConfig_Sight> SightConfig = nullptr;
#pragma endregion 런타임 상태
};
