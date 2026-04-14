// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/KNAnimNotifyState_WeaponTrail.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Characters/Player/KNPlayerCharacter.h"

#pragma region 노티파이 오버라이드 구현
void UKNAnimNotifyState_WeaponTrail::NotifyBegin(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    // 이전 콤보의 Trail이 잔존할 경우 먼저 정리합니다.
    // (두 번째 콤보에서 Trail이 안 나오는 버그 방지)
    if (UNiagaraComponent* Root = SpawnedTrailRoot.Get())
    {
        Root->Deactivate();
        SpawnedTrailRoot = nullptr;
    }
    if (UNiagaraComponent* Tip = SpawnedTrailTip.Get())
    {
        Tip->Deactivate();
        SpawnedTrailTip = nullptr;
    }

    if (!MeshComp || !TrailVFX) return;

    // 최적화: FindComponentByClass O(N) 배제 → GetKatanaMesh() O(1) Getter 사용
    AKNPlayerCharacter* Player = Cast<AKNPlayerCharacter>(MeshComp->GetOwner());
    if (!Player) return;

    UStaticMeshComponent* KatanaMesh = Player->GetKatanaMesh();
    if (!KatanaMesh) return;

    // 소켓 존재 여부 사전 검사
    if (!KatanaMesh->DoesSocketExist(FirstSocketName) ||
        !KatanaMesh->DoesSocketExist(SecondSocketName))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[WeaponTrail] 소켓을 찾을 수 없습니다. First: %s / Second: %s"),
            *FirstSocketName.ToString(), *SecondSocketName.ToString());
        return;
    }

    // 코등이 소켓에 Trail 스폰
    UNiagaraComponent* TrailRoot = UNiagaraFunctionLibrary::SpawnSystemAttached(
        TrailVFX,
        KatanaMesh,
        FirstSocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::SnapToTarget,
        /*bAutoDestroy=*/false);

    // 칼끝 소켓에 Trail 스폰
    UNiagaraComponent* TrailTip = UNiagaraFunctionLibrary::SpawnSystemAttached(
        TrailVFX,
        KatanaMesh,
        SecondSocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::SnapToTarget,
        /*bAutoDestroy=*/false);

    // Niagara 크기 파라미터 전달
    if (TrailRoot)
    {
        TrailRoot->SetFloatParameter(TEXT("_EnchantSize"), EnchantSize);
        TrailRoot->ResetSystem();
    }
    if (TrailTip)
    {
        TrailTip->SetFloatParameter(TEXT("_EnchantSize"), EnchantSize);
        TrailTip->ResetSystem();
    }

    SpawnedTrailRoot = TrailRoot;
    SpawnedTrailTip = TrailTip;
}

void UKNAnimNotifyState_WeaponTrail::NotifyEnd(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    // 파티클이 자연스럽게 사그라들도록 Destroy가 아닌 Deactivate 사용
    if (UNiagaraComponent* Root = SpawnedTrailRoot.Get())
    {
        Root->Deactivate();
        SpawnedTrailRoot = nullptr;
    }
    if (UNiagaraComponent* Tip = SpawnedTrailTip.Get())
    {
        Tip->Deactivate();
        SpawnedTrailTip = nullptr;
    }

    Super::NotifyEnd(MeshComp, Animation, EventReference);
}
#pragma endregion 노티파이 오버라이드 구현