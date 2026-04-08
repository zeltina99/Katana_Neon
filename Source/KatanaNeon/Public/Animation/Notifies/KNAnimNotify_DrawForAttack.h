// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KNAnimNotify_DrawForAttack.generated.h"

/**
 * @brief 납도 콤보 시작 시 칼을 손에 붙이는 노티파이.
 * @details WeaponDrawn 태그는 변경하지 않습니다. 메시 위치만 이동합니다.
 */
UCLASS()
class KATANANEON_API UKNAnimNotify_DrawForAttack : public UAnimNotify
{
	GENERATED_BODY()
	
#pragma region 노티파이 오버라이드
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
#pragma endregion 노티파이 오버라이드
};
