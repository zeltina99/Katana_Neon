// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KNAnimNotify_HitboxOpen.generated.h"

#pragma region 전방 선언
class USkeletalMeshComponent;
class UAnimSequenceBase;
#pragma endregion 전방 선언

/**
 * @file    KNAnimNotify_HitboxOpen.h
 * @brief   애니메이션 몽타주에서 피격 판정(Hitbox)을 활성화하는 노티파이 클래스입니다.
 * @details 단일 책임 원칙(SRP)에 따라 오직 '히트박스 열기' 이벤트 전달 역할만 수행합니다.
 */
UCLASS()
class KATANANEON_API UKNAnimNotify_HitboxOpen : public UAnimNotify
{
	GENERATED_BODY()
	
#pragma region 노티파이 오버라이드
public:
    /**
     * @brief 몽타주에서 노티파이 시점에 도달했을 때 호출됩니다.
     * @param MeshComp 애니메이션을 재생 중인 스켈레탈 메시 컴포넌트
     * @param Animation 현재 재생 중인 애니메이션 에셋
     * @param EventReference 노티파이 이벤트 레퍼런스 (UE 5.0 이상 규격)
     */
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
#pragma endregion 노티파이 오버라이드
};
