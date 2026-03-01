// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/KNAbilityOverclockLv2.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "NiagaraFunctionLibrary.h"
#include "Characters/Base/KNCharacterBase.h" // ACharacter 교체
#include "Engine/DataTable.h"
#include "Objects/Projectiles/KNSlashProjectile.h"
#include "GAS/Components/KNStatsComponent.h"
// 존재하지 않는 KNAbilityTags.h 대신 올바른 태그 사전 인클루드
#include "GAS/Tags/KNStatsTags.h" 

#pragma region 기본 생성자 및 초기화 구현
UKNAbilityOverclockLv2::UKNAbilityOverclockLv2()
{
    // ── UE 5.5 최신 규약 적용 (경고 제거) ──
    FGameplayTagContainer TempTags;
    TempTags.AddTag(KatanaNeon::Ability::Overclock::Lv2);
    SetAssetTags(TempTags);

    // 오버클럭 Lv2 태그 없으면 자동 차단
    ActivationRequiredTags.AddTag(KatanaNeon::State::Overclock::Lv2);

    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region GAS 핵심 오버라이드 구현
void UKNAbilityOverclockLv2::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    bSlashReleased = false;

    // ── 1. DataTable 수치 로드 (DDD) ──
    if (!LoadLv2Setting())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── 2. 오버클럭 포인트 소모 ──
    if (!ConsumeOverclockLevel())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ── 3. 발동 몽타주 재생 (AbilityTask) ──
    if (!SlashMontage)
    {
        // 몽타주가 세팅되지 않았을 경우 즉시 참격파 발사 후 종료 (예외 처리)
        AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo());
        SpawnSlashProjectile(Owner);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    UAbilityTask_PlayMontageAndWait* MontageTask =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, SlashMontage, 1.0f, NAME_None, false);

    // 몽타주가 끝나거나 끊기면 어빌리티를 완전히 종료합니다.
    // (주의: 발사는 OnSlashReleaseNotify에서 처리되므로 여기선 종료만 담당합니다)
    MontageTask->OnCompleted.AddDynamic(this, &UKNAbilityOverclockLv2::OnSlashReleaseNotify);
    MontageTask->OnBlendOut.AddDynamic(this, &UKNAbilityOverclockLv2::OnSlashReleaseNotify);
    MontageTask->OnInterrupted.AddDynamic(this, &UKNAbilityOverclockLv2::OnSlashReleaseNotify);
    MontageTask->ReadyForActivation();
}

void UKNAbilityOverclockLv2::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion GAS 핵심 오버라이드 구현

#pragma region AnimNotify 연동 인터페이스 구현
void UKNAbilityOverclockLv2::OnSlashReleaseNotify()
{
    // 중복 발사 완벽 차단
    if (bSlashReleased) return;
    bSlashReleased = true;

    AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo());
    SpawnSlashProjectile(Owner);

    // 투사체 발사 후 어빌리티는 목표를 달성했으므로 안전하게 종료합니다.
    EndAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo(),
        true, false);
}
#pragma endregion AnimNotify 연동 인터페이스 구현

#pragma region 내부 헬퍼 함수 구현
bool UKNAbilityOverclockLv2::LoadLv2Setting()
{
    // 하드코딩 배제 및 안전 검사
    if (!Lv2SettingRowHandle.DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[KNAbilityOverclockLv2] Lv2SettingRowHandle이 에디터에 할당되지 않았습니다!"));
        return false;
    }

    const FKNOverclockLv2Row* Row = Lv2SettingRowHandle.GetRow<FKNOverclockLv2Row>(TEXT("LoadLv2Setting"));

    if (!ensureAlwaysMsgf(Row, TEXT("[KNAbilityOverclockLv2] 지정된 오버클럭 Lv2 설정 행을 찾을 수 없습니다.")))
    {
        return false;
    }

    CachedSetting = *Row;
    return true;
}

bool UKNAbilityOverclockLv2::ConsumeOverclockLevel()
{
    // 최적화: 범용 ACharacter가 아닌 우리 고유의 베이스 캐릭터로 캐스팅
    AKNCharacterBase* Owner = Cast<AKNCharacterBase>(GetAvatarActorFromActorInfo());
    if (!Owner) return false;

    // 활성화 시 1회만 호출되므로 FindComponentByClass 사용이 허용됩니다.
    UKNStatsComponent* Stats = Owner->FindComponentByClass<UKNStatsComponent>();
    if (!Stats)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KNAbilityOverclockLv2] KNStatsComponent를 찾을 수 없습니다!"));
        return false;
    }

    return Stats->ConsumeOverclockLevel(2);
}

void UKNAbilityOverclockLv2::SpawnSlashProjectile(AKNCharacterBase* Owner)
{
    if (!Owner || !SlashProjectileClass) return;

    // 투사체에 주입할 본체의 ASC
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC) return;

    // ── 베테랑 최적화: 스폰 위치 및 방향 계산 ──
    const FVector Forward = Owner->GetActorForwardVector();
    // 캡슐 중앙에서 전방으로 60cm 떨어진 곳에서 스폰하여 플레이어 본인과 충돌하는 것을 방지합니다.
    const FVector SpawnPos = Owner->GetActorLocation() + (Forward * 60.0f);
    const FRotator SpawnRot = Forward.Rotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Instigator = Owner;
    SpawnParams.Owner = Owner;
    // 액터 겹침이 발생해도 강제로 스폰시킵니다 (액션 게임 필수 설정)
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 지연 스폰(Deferred Spawn) 최적화: 스폰 즉시 BeginPlay가 불리는 것을 막고 데이터 주입 후 수동 시작
    AKNSlashProjectile* Projectile = GetWorld()->SpawnActorDeferred<AKNSlashProjectile>(
        SlashProjectileClass, FTransform(SpawnRot, SpawnPos), Owner, Owner, SpawnParams.SpawnCollisionHandlingOverride);

    if (Projectile)
    {
        // BeginPlay가 불리기 전에 완벽하게 데이터 주입 (우리가 만든 최적화된 InitProjectile 호출)
        Projectile->InitProjectile(ASC, SlashDamageGEClass, GrogyGEClass, CachedSetting);

        // 데이터 주입 완료 후 발사체 생명주기 시작
        Projectile->FinishSpawning(FTransform(SpawnRot, SpawnPos));
    }

    // ── 부가 이펙트 스폰 ──
    if (SlashNiagara)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SlashNiagara, SpawnPos, SpawnRot);
    }
}
#pragma endregion 내부 헬퍼 함수 구현
