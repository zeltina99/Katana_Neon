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
            TEXT("[WeaponTrail] 소켓을 찾을 수 없습니다. FirstSocket: %s / SecondSocket: %s"),
            *FirstSocketName.ToString(), *SecondSocketName.ToString());
        return;
    }

    // 카타나 메시에 궤적 VFX 부착 (bAutoDestroy = false → NotifyEnd에서 수동 종료)
    UNiagaraComponent* Trail = UNiagaraFunctionLibrary::SpawnSystemAttached(
        TrailVFX,
        KatanaMesh,
        FirstSocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::SnapToTarget,
        /*bAutoDestroy=*/false);

    if (!Trail) return;

    // Niagara 파라미터로 시작/끝 소켓 위치 전달
    Trail->SetVectorParameter(
        TEXT("BeamStart"),
        KatanaMesh->GetSocketLocation(FirstSocketName));
    Trail->SetVectorParameter(
        TEXT("BeamEnd"),
        KatanaMesh->GetSocketLocation(SecondSocketName));

    SpawnedTrailComp = Trail;
}

void UKNAnimNotifyState_WeaponTrail::NotifyEnd(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    // 궤적 VFX 비활성화 — 파티클이 자연스럽게 사그라들도록 Deactivate 사용
    if (UNiagaraComponent* Trail = SpawnedTrailComp.Get())
    {
        Trail->Deactivate();
        SpawnedTrailComp = nullptr;
    }

    Super::NotifyEnd(MeshComp, Animation, EventReference);
}
#pragma endregion 노티파이 오버라이드 구현