// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/KNAnimNotify_SheathAfterAttack.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Characters/Player/KNPlayerCharacter.h"

#pragma region 노티파이 구현
void UKNAnimNotify_SheathAfterAttack::Notify(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);
    if (!MeshComp) return;

    AKNPlayerCharacter* Player = Cast<AKNPlayerCharacter>(MeshComp->GetOwner());
    if (!Player) return;

    // 칼을 칼집 소켓으로 복귀 — WeaponDrawn 태그 변경 없음
    if (UStaticMeshComponent* Katana = Player->GetKatanaMesh())
    {
        Katana->AttachToComponent(MeshComp,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            TEXT("SheathSocket_L"));
    }
}
#pragma endregion 노티파이 구현
