// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/KNAnimNotify_HitboxOpen.h"
#include "Components/SkeletalMeshComponent.h" 
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/Abilities/KNAbilityComboAttack.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 노티파이 구현
void UKNAnimNotify_HitboxOpen::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp) return;

    // 1. 오너의 ASC를 안전하게 가져옵니다. (캐릭터 클래스에 대한 의존성 없음)
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
    if (!ASC) return;

    // 2. 현재 ASC에 등록된 '공격(Attack)' 태그를 가진 어빌리티 스펙들을 모두 찾습니다.
    TArray<FGameplayAbilitySpec*> Specs;
    ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
        FGameplayTagContainer(KatanaNeon::Ability::Combat::Attack),
        Specs, false);

    // 3. 찾은 스펙 중 현재 '실행 중(Active)'인 인스턴스를 찾아 직접 명령을 내립니다.
    for (FGameplayAbilitySpec* Spec : Specs)
    {
        if (!Spec || !Spec->IsActive()) continue;

        for (UGameplayAbility* Instance : Spec->GetAbilityInstances())
        {
            if (UKNAbilityComboAttack* ComboAbility = Cast<UKNAbilityComboAttack>(Instance))
            {
                // 어빌리티 내부에 구현된 ActivateHitbox()를 직접 호출 (SRP 준수)
                ComboAbility->ActivateHitbox();
                return; // 찾았으니 즉시 종료
            }
        }
    }
}
#pragma endregion 노티파이 구현
