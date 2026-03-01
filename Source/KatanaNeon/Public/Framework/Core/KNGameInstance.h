// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "KNGameInstance.generated.h"

#pragma region 전방 선언
class UDataTable;
#pragma endregion 전방 선언

/**
 * @file    KNGameInstance.h
 * @class   UKNGameInstance
 * @brief   KatanaNeon 프로젝트의 12종 마스터 데이터 테이블을 보관하는 싱글톤 클래스입니다.
 * @details 데이터 로드 및 런타임 제공은 UKNDataManagerSubsystem으로 위임하여 SRP를 준수합니다.
 */
UCLASS()
class KATANANEON_API UKNGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
#pragma region 글로벌 데이터 테이블 (에디터 할당용)
protected:
    // ── 플레이어 전투 및 스탯 데이터 ──
    /** @brief 플레이어 기본 스탯 마스터 테이블 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Data|Player")
    TObjectPtr<UDataTable> PlayerBaseStatTable = nullptr;

    /** @brief 플레이어 액션 기력 소모량 마스터 테이블 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Data|Player")
    TObjectPtr<UDataTable> ActionCostTable = nullptr;

    /** @brief 플레이어 점프(더블 점프 포함) 마스터 테이블 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Data|Player")
    TObjectPtr<UDataTable> JumpSettingTable = nullptr;

    /** @brief 플레이어 콤보 공격 마스터 테이블 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Data|Player")
    TObjectPtr<UDataTable> ComboAttackTable = nullptr;

    // ── 시스템(오버클럭, 크로노스) 데이터 ──
    /** @brief 오버클럭 기본 설정 및 임계값 테이블 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Data|System")
    TObjectPtr<UDataTable> OverclockSettingTable = nullptr;

    /** @brief 오버클럭 1단계(전술 강화) 설정 테이블 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Data|System")
    TObjectPtr<UDataTable> OverclockLv1Table = nullptr;

    /** @brief 오버클럭 2단계(참격파) 설정 테이블 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Data|System")
    TObjectPtr<UDataTable> OverclockLv2Table = nullptr;

    /** @brief 오버클럭 3단계(시간 정지) 설정 테이블 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Data|System")
    TObjectPtr<UDataTable> OverclockLv3Table = nullptr;

    /** @brief 크로노스 구체 설정 테이블 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Data|System")
    TObjectPtr<UDataTable> ChronosSettingTable = nullptr;

    // ── 적 및 보스 데이터 ──
    /** @brief 일반 적 기본 스탯 마스터 테이블 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Data|Enemy")
    TObjectPtr<UDataTable> EnemyStatTable = nullptr;

    /** @brief 원거리 적 전용 마스터 테이블 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Data|Enemy")
    TObjectPtr<UDataTable> EnemyRangedTable = nullptr;

    /** @brief 보스 페이즈(MidBoss, FinalBoss) 마스터 테이블 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Data|Enemy")
    TObjectPtr<UDataTable> BossPhaseTable = nullptr;
#pragma endregion 글로벌 데이터 테이블

#pragma region 서브시스템 접근 인터페이스
public:
    // Subsystem이 데이터를 캐싱할 때 호출하는 Getter 함수들
    UDataTable* GetPlayerBaseStatTable() const { return PlayerBaseStatTable; }
    UDataTable* GetActionCostTable() const { return ActionCostTable; }
    UDataTable* GetJumpSettingTable() const { return JumpSettingTable; }
    UDataTable* GetComboAttackTable() const { return ComboAttackTable; }
    UDataTable* GetOverclockSettingTable() const { return OverclockSettingTable; }
    UDataTable* GetOverclockLv1Table() const { return OverclockLv1Table; }
    UDataTable* GetOverclockLv2Table() const { return OverclockLv2Table; }
    UDataTable* GetOverclockLv3Table() const { return OverclockLv3Table; }
    UDataTable* GetChronosSettingTable() const { return ChronosSettingTable; }
    UDataTable* GetEnemyStatTable() const { return EnemyStatTable; }
    UDataTable* GetEnemyRangedTable() const { return EnemyRangedTable; }
    UDataTable* GetBossPhaseTable() const { return BossPhaseTable; }
#pragma endregion 서브시스템 접근 인터페이스
};
