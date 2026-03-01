// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KNDataManagerSubsystem.generated.h"

#pragma region 전방 선언
struct FKNBaseStatRow;
struct FKNActionCostRow;
struct FKNJumpSettingRow;
struct FKNComboAttackRow;
struct FKNOverclockSettingRow;
struct FKNOverclockLv1Row;
struct FKNOverclockLv2Row;
struct FKNOverclockLv3Row;
struct FKNChronosSettingRow;
struct FKNEnemyBaseStatRow;
struct FKNEnemyRangedStatRow;
struct FKNBossPhaseRow;
#pragma endregion 전방 선언

/**
 * @file    KNDataManagerSubsystem.h
 * @class   UKNDataManagerSubsystem
 * @brief   KatanaNeon 프로젝트의 모든 데이터 테이블 조회를 담당하는 전역 매니저입니다.
 */
UCLASS()
class KATANANEON_API UKNDataManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
#pragma region 서브시스템 생명주기
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
#pragma endregion 서브시스템 생명주기

#pragma region 글로벌 데이터 조회 인터페이스
public:
    // ── 플레이어 데이터 조회 ──
    const FKNBaseStatRow* GetPlayerBaseStat(const FName& RowName = TEXT("Default")) const;
    const FKNActionCostRow* GetActionCost(const FName& RowName = TEXT("Default")) const;
    const FKNJumpSettingRow* GetJumpSetting(const FName& RowName = TEXT("Default")) const;
    const FKNComboAttackRow* GetComboAttackData(const FName& RowName) const;

    // ── 시스템(오버클럭, 크로노스) 데이터 조회 ──
    const FKNOverclockSettingRow* GetOverclockSetting(const FName& RowName = TEXT("Default")) const;
    const FKNOverclockLv1Row* GetOverclockLv1Setting(const FName& RowName = TEXT("Default")) const;
    const FKNOverclockLv2Row* GetOverclockLv2Setting(const FName& RowName = TEXT("Default")) const;
    const FKNOverclockLv3Row* GetOverclockLv3Setting(const FName& RowName = TEXT("Default")) const;
    const FKNChronosSettingRow* GetChronosSetting(const FName& RowName = TEXT("Default")) const;

    // ── 적 및 보스 데이터 조회 ──
    const FKNEnemyBaseStatRow* GetEnemyStat(const FName& RowName) const;
    const FKNEnemyRangedStatRow* GetEnemyRangedStat(const FName& RowName = TEXT("Default")) const;

    /**
     * @brief 보스 페이즈 전환 수치를 가져옵니다.
     * @param RowName "MidBoss" 또는 "FinalBoss" 등 기획자가 지정한 행 이름
     */
    const FKNBossPhaseRow* GetBossPhase(const FName& RowName) const;
#pragma endregion 글로벌 데이터 조회 인터페이스
};
