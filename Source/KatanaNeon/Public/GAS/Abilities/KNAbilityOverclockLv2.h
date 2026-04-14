// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/Structs/KNPlayerStatTable.h"
#include "KNAbilityOverclockLv2.generated.h"

#pragma region 전방 선언
class UAnimMontage;
class UNiagaraSystem;
class AKNSlashProjectile;
class UKNStatsComponent;
class AKNCharacterBase;
class UGameplayEffect;
#pragma endregion 전방 선언

/**
 * @file    KNAbilityOverclockLv2.h
 * @class   UKNAbilityOverclockLv2
 * @brief   오버클럭 2단계 — 전방으로 참격파를 발사하여 맞은 적/보스를 그로기 상태로 만드는 어빌리티입니다.
 *
 * @details
 * [아키텍처 맞춤형 설계]
 * - FDataTableRowHandle을 사용하여 하드코딩을 배제한 데이터 주도적 설계(DDD)를 적용했습니다.
 * - 범용 ACharacter 캐스팅을 지양하고 프로젝트 고유의 AKNCharacterBase를 사용합니다.
 *
 * [참격파 발사 흐름]
 * 1. ActivateAbility: 오버클럭 소모 → 발동 몽타주 재생 (PlayMontageAndWait Task)
 * 2. 몽타주 실행 중 AnimNotify "SlashRelease" 수신 → 참격파(AKNSlashProjectile) 스폰
 * 3. AKNSlashProjectile이 이동/피격 처리를 전담 (SRP 준수)
 *
 * [SRP 책임 분리]
 * - 참격파 로직     : AKNSlashProjectile에 완전 위임
 * - 오버클럭 소모   : KNStatsComponent::ConsumeOverclockLevel(2) 위임
 * - 수치/에셋 관리  : DT_OverclockLv2Setting DataTable
 */
UCLASS()
class KATANANEON_API UKNAbilityOverclockLv2 : public UGameplayAbility
{
    GENERATED_BODY()

#pragma region 기본 생성자 및 초기화
public:
    /**
     * @brief AbilityTag, ActivationRequiredTags, InstancingPolicy를 설정합니다.
     * @details UE 5.5 규약에 맞게 SetAssetTags를 사용합니다.
     */
    UKNAbilityOverclockLv2();
#pragma endregion 기본 생성자 및 초기화

#pragma region GAS 핵심 오버라이드
public:
    /**
     * @brief 참격파 어빌리티 활성화.
     * @details 오버클럭 소모 → 몽타주 재생 태스크 실행 순으로 진행합니다.
     */
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    /**
     * @brief 어빌리티 종료.
     */
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;
#pragma endregion GAS 핵심 오버라이드

#pragma region AnimNotify 연동 인터페이스
public:
    /**
     * @brief 몽타주의 AnimNotify "SlashRelease" 발생 시 호출됩니다.
     * @details 이 함수가 호출되는 순간 투사체가 스폰됩니다.
     */
    UFUNCTION(BlueprintCallable, Category = "KatanaNeon|Ability|OverclockLv2")
    void OnSlashReleaseNotify();
#pragma endregion AnimNotify 연동 인터페이스

#pragma region 에디터 설정 데이터 (DDD)
protected:
    /**
     * @brief 오버클럭 Lv2 설정 DataTable 행 핸들.
     * @details 기획자가 에디터에서 참격파 수치와 그로기 지속 시간을 직접 주입합니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv2|DataTable")
    FDataTableRowHandle Lv2SettingRowHandle;

    /**
     * @brief 발도 상태 참격파 발동 몽타주.
     * @details WeaponDrawn 태그가 있을 때 재생됩니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv2|Montage")
    TObjectPtr<UAnimMontage> SlashMontage_Drawn = nullptr;

    /**
     * @brief 납도 상태 참격파 발동 몽타주.
     * @details WeaponDrawn 태그가 없을 때 재생됩니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv2|Montage")
    TObjectPtr<UAnimMontage> SlashMontage_Sheath = nullptr;

    /** @brief 발사할 참격파 블루프린트 클래스 (에디터 할당) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv2|Projectile")
    TSubclassOf<AKNSlashProjectile> SlashProjectileClass = nullptr;

    /** @brief 피격 시 즉시 적용할 데미지 Instant GE 클래스 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv2|GAS")
    TSubclassOf<UGameplayEffect> SlashDamageGEClass = nullptr;

    /** @brief 그로기 상태(State.Combat.Groggy)를 부여할 Duration GE 클래스 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv2|GAS")
    TSubclassOf<UGameplayEffect> GrogyGEClass = nullptr;

    /** @brief 참격파 발사 순간 캐릭터 주변에서 터질 부가 이펙트 (옵션) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Ability|OverclockLv2|FX")
    TObjectPtr<UNiagaraSystem> SlashNiagara = nullptr;
#pragma endregion 에디터 설정 데이터 (DDD)

#pragma region 런타임 상태
private:
    /** @brief DataTable에서 읽어와 런타임에 캐싱한 Lv2 설정 데이터 */
    FKNOverclockLv2Row CachedSetting;

    /** @brief AnimNotify가 여러 번 호출되어 참격파가 중복 발사되는 것을 막는 안전장치 */
    bool bSlashReleased = false;
#pragma endregion 런타임 상태

#pragma region 내부 헬퍼 함수
private:
    /**
     * @brief 에디터에 할당된 데이터 행을 읽어와 CachedSetting에 저장합니다.
     * @return 로드 성공 여부
     */
    bool LoadLv2Setting();

    /**
     * @brief KNStatsComponent를 통해 오버클럭 Lv2 포인트를 차감합니다.
     * @return 소모 성공 여부
     */
    bool ConsumeOverclockLevel();

    /**
     * @brief 플레이어 전방에 참격파 발사체를 스폰합니다.
     * @param Owner 발사 주체 캐릭터 (AKNCharacterBase)
     */
    void SpawnSlashProjectile(AKNCharacterBase* Owner);

    /**
     * @brief 현재 WeaponDrawn 태그 유무로 발도 상태를 판별합니다.
     * @return 발도 상태이면 true
     */
    bool IsWeaponDrawn() const;
#pragma endregion 내부 헬퍼 함수
};
