// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KNAnimNotify_SlashRelease.generated.h"

#pragma region 전방 선언
class USkeletalMeshComponent;
class UAnimSequenceBase;
#pragma endregion 전방 선언

/**
 * @file    KNAnimNotify_SlashRelease.h
 * @brief   오버클럭 2단계 몽타주에서 참격파 발사 시점을 알리는 노티파이입니다.
 * @details ASC에서 활성 UKNAbilityOverclockLv2 인스턴스를 찾아 OnSlashReleaseNotify()를 호출합니다.
 */
UCLASS()
class KATANANEON_API UKNAnimNotify_SlashRelease : public UAnimNotify
{
	GENERATED_BODY()
	
#pragma region 노티파이 오버라이드
public:
    /**
     * @brief 몽타주에서 노티파이 시점에 도달했을 때 호출됩니다.
     * @param MeshComp       애니메이션을 재생 중인 스켈레탈 메시 컴포넌트
     * @param Animation      현재 재생 중인 애니메이션 에셋
     * @param EventReference 노티파이 이벤트 레퍼런스
     */
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;
#pragma endregion 노티파이 오버라이드
};
