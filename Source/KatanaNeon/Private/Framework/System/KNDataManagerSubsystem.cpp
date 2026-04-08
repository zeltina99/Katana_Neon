// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/System/KNDataManagerSubsystem.h"
#include "Framework/Core/KNGameInstance.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "Data/Structs/KNEnemyStatTable.h"

#pragma region 서브시스템 생명주기 구현
void UKNDataManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[KNDataManagerSubsystem] 12종 마스터 데이터 테이블 시스템 활성화"));
}

void UKNDataManagerSubsystem::Deinitialize()
{
    Super::Deinitialize();
}
#pragma endregion 서브시스템 생명주기 구현

#pragma region 글로벌 데이터 조회 구현
const FKNBaseStatRow* UKNDataManagerSubsystem::GetPlayerBaseStat(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetPlayerBaseStatTable()) ? GI->GetPlayerBaseStatTable()->FindRow<FKNBaseStatRow>(RowName, TEXT("GetPlayerBaseStat")) : nullptr;
}

const FKNActionCostRow* UKNDataManagerSubsystem::GetActionCost(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetActionCostTable()) ? GI->GetActionCostTable()->FindRow<FKNActionCostRow>(RowName, TEXT("GetActionCost")) : nullptr;
}

const FKNJumpSettingRow* UKNDataManagerSubsystem::GetJumpSetting(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetJumpSettingTable()) ? GI->GetJumpSettingTable()->FindRow<FKNJumpSettingRow>(RowName, TEXT("GetJumpSetting")) : nullptr;
}

const FKNComboAttackRow* UKNDataManagerSubsystem::GetDrawnComboAttackData(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetDrawnComboAttackTable())
        ? GI->GetDrawnComboAttackTable()->FindRow<FKNComboAttackRow>(RowName, TEXT("GetDrawnComboAttackData"))
        : nullptr;
}

const FKNComboAttackRow* UKNDataManagerSubsystem::GetSheathComboAttackData(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetSheathComboAttackTable())
        ? GI->GetSheathComboAttackTable()->FindRow<FKNComboAttackRow>(RowName, TEXT("GetSheathComboAttackData"))
        : nullptr;
}

const FKNOverclockSettingRow* UKNDataManagerSubsystem::GetOverclockSetting(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetOverclockSettingTable()) ? GI->GetOverclockSettingTable()->FindRow<FKNOverclockSettingRow>(RowName, TEXT("GetOverclockSetting")) : nullptr;
}

const FKNOverclockLv1Row* UKNDataManagerSubsystem::GetOverclockLv1Setting(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetOverclockLv1Table()) ? GI->GetOverclockLv1Table()->FindRow<FKNOverclockLv1Row>(RowName, TEXT("GetOverclockLv1Setting")) : nullptr;
}

const FKNOverclockLv2Row* UKNDataManagerSubsystem::GetOverclockLv2Setting(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetOverclockLv2Table()) ? GI->GetOverclockLv2Table()->FindRow<FKNOverclockLv2Row>(RowName, TEXT("GetOverclockLv2Setting")) : nullptr;
}

const FKNOverclockLv3Row* UKNDataManagerSubsystem::GetOverclockLv3Setting(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetOverclockLv3Table()) ? GI->GetOverclockLv3Table()->FindRow<FKNOverclockLv3Row>(RowName, TEXT("GetOverclockLv3Setting")) : nullptr;
}

const FKNChronosSettingRow* UKNDataManagerSubsystem::GetChronosSetting(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetChronosSettingTable()) ? GI->GetChronosSettingTable()->FindRow<FKNChronosSettingRow>(RowName, TEXT("GetChronosSetting")) : nullptr;
}

const FKNEnemyBaseStatRow* UKNDataManagerSubsystem::GetEnemyStat(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetEnemyStatTable()) ? GI->GetEnemyStatTable()->FindRow<FKNEnemyBaseStatRow>(RowName, TEXT("GetEnemyStat")) : nullptr;
}

const FKNEnemyRangedStatRow* UKNDataManagerSubsystem::GetEnemyRangedStat(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetEnemyRangedTable()) ? GI->GetEnemyRangedTable()->FindRow<FKNEnemyRangedStatRow>(RowName, TEXT("GetEnemyRangedStat")) : nullptr;
}

const FKNBossPhaseRow* UKNDataManagerSubsystem::GetBossPhase(const FName& RowName) const
{
    UKNGameInstance* GI = Cast<UKNGameInstance>(GetGameInstance());
    return (GI && GI->GetBossPhaseTable()) ? GI->GetBossPhaseTable()->FindRow<FKNBossPhaseRow>(RowName, TEXT("GetBossPhase")) : nullptr;
}
#pragma endregion 글로벌 데이터 조회 구현