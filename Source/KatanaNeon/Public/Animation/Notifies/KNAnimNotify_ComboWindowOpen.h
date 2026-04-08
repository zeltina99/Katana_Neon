// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KNAnimNotify_ComboWindowOpen.generated.h"

#pragma region 전방 선언
class USkeletalMeshComponent;
class UAnimSequenceBase;
#pragma endregion 전방 선언

/**
 * @file    KNAnimNotify_ComboWindowOpen.h
 * @brief   애니메이션 몽타주에서 다음 콤보 입력을 허용하는 창(Window)을 여는 노티파이 클래스입니다.
 * @details 하드코딩된 시간(Timer)에 의존하지 않고 애니메이션 프레임 기반의 데이터 드라이븐 방식을 지원합니다.
 */
UCLASS()
class KATANANEON_API UKNAnimNotify_ComboWindowOpen : public UAnimNotify
{
	GENERATED_BODY()
	
#pragma region 노티파이 오버라이드
public:
    /**
     * @brief 몽타주에서 노티파이 시점에 도달했을 때 호출됩니다.
     * @param MeshComp 애니메이션을 재생 중인 스켈레탈 메시 컴포넌트
     * @param Animation 현재 재생 중인 애니메이션 에셋
     * @param EventReference 노티파이 이벤트 레퍼런스
     */
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
#pragma endregion 노티파이 오버라이드
};
