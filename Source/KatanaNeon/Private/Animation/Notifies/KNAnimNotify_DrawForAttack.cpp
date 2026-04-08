// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/KNAnimNotify_DrawForAttack.h"
#include "Components/SkeletalMeshComponent.h"
#include "Characters/Player/KNPlayerCharacter.h"

void UKNAnimNotify_DrawForAttack::Notify(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);
    if (!MeshComp) return;

    AKNPlayerCharacter* Player = Cast<AKNPlayerCharacter>(MeshComp->GetOwner());
    if (!Player) return;

    // 칼을 손에 붙이기만 — 태그 변경 없음
    if (UStaticMeshComponent* Katana = Player->GetKatanaMesh())
    {
        Katana->AttachToComponent(MeshComp,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            TEXT("WeaponSocket_R"));
    }
}
