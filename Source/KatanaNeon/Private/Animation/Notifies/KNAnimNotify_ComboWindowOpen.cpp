// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/KNAnimNotify_ComboWindowOpen.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/Abilities/KNAbilityComboAttack.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 노티파이 구현
void UKNAnimNotify_ComboWindowOpen::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp) return;

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
    if (!ASC) return;

    TArray<FGameplayAbilitySpec*> Specs;
    ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
        FGameplayTagContainer(KatanaNeon::Ability::Combat::Attack),
        Specs, false);

    for (FGameplayAbilitySpec* Spec : Specs)
    {
        if (!Spec || !Spec->IsActive()) continue;

        for (UGameplayAbility* Instance : Spec->GetAbilityInstances())
        {
            if (UKNAbilityComboAttack* ComboAbility = Cast<UKNAbilityComboAttack>(Instance))
            {
                // 어빌리티 내부에 구현된 OpenComboWindow()를 직접 호출 (선입력 큐 처리 진입점)
                ComboAbility->OpenComboWindow();
                return;
            }
        }
    }
}
#pragma endregion 노티파이 구현