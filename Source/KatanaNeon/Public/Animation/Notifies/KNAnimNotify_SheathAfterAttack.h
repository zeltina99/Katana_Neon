// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KNAnimNotify_SheathAfterAttack.generated.h"

#pragma region 전방 선언
class USkeletalMeshComponent;
class UAnimSequenceBase;
#pragma endregion 전방 선언

/**
 * @file    KNAnimNotify_SheathAfterAttack.h
 * @brief   납도 콤보 모션 마지막에 칼을 칼집으로 되돌리는 노티파이입니다.
 * @details WeaponDrawn GAS 태그는 변경하지 않습니다. 메시 소켓 위치만 이동합니다.
 */
UCLASS()
class KATANANEON_API UKNAnimNotify_SheathAfterAttack : public UAnimNotify
{
	GENERATED_BODY()
	
#pragma region 노티파이 오버라이드
public:
    /**
     * @brief 납도 복귀 시점에 호출됩니다.
     * @param MeshComp  애니메이션을 재생 중인 스켈레탈 메시 컴포넌트
     * @param Animation 현재 재생 중인 애니메이션 에셋
     * @param EventReference 노티파이 이벤트 레퍼런스
     */
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;
#pragma endregion 노티파이 오버라이드
};
