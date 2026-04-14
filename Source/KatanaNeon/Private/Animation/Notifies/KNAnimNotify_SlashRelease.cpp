// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/KNAnimNotify_SlashRelease.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/Abilities/KNAbilityOverclockLv2.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 노티파이 구현
void UKNAnimNotify_SlashRelease::Notify(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);
    if (!MeshComp) return;

    UAbilitySystemComponent* ASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
    if (!ASC) return;

    // KNAnimNotify_HitboxOpen과 동일한 패턴 — ASC에서 활성 인스턴스 직접 탐색
    TArray<FGameplayAbilitySpec*> Specs;
    ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
        FGameplayTagContainer(KatanaNeon::Ability::Overclock::Lv2),
        Specs, false);

    for (FGameplayAbilitySpec* Spec : Specs)
    {
        if (!Spec || !Spec->IsActive()) continue;

        UGameplayAbility* Instance = Spec->GetPrimaryInstance();
        if (UKNAbilityOverclockLv2* Lv2Ability = Cast<UKNAbilityOverclockLv2>(Instance))
        {
            Lv2Ability->OnSlashReleaseNotify();
            return;
        }
    }
}
#pragma endregion 노티파이 구현