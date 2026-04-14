// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "KNAnimNotifyState_WeaponTrail.generated.h"

#pragma region 전방 선언
class UNiagaraSystem;
class UNiagaraComponent;
#pragma endregion 전방 선언

/**
 * @file    KNAnimNotifyState_WeaponTrail.h
 * @class   UKNAnimNotifyState_WeaponTrail
 * @brief   공격 모션 구간 동안 카타나 스태틱 메시의 두 소켓에 궤적 VFX를 생성합니다.
 *
 * @details
 * [SRP 책임]
 * - 오직 "카타나 메시 소켓 기반 궤적 VFX 생성/종료"만 담당합니다. (View 전담)
 * - 히트박스 판정, 데미지 적용은 KNAbilityComboAttack에 완전 위임합니다.
 *
 * [최적화]
 * - AKNPlayerCharacter::GetKatanaMesh() O(1) Getter로 FindComponentByClass O(N) 배제
 * - TWeakObjectPtr로 Niagara 컴포넌트를 보관하여 댕글링 포인터 크래시 방지
 * - NotifyBegin 진입 시 이전 Trail 잔존 여부를 검사하여 두 번째 콤보도 정상 동작 보장
 */
UCLASS(meta = (DisplayName = "무기 궤적 (Weapon Trail)"))
class KATANANEON_API UKNAnimNotifyState_WeaponTrail : public UAnimNotifyState
{
	GENERATED_BODY()
	
#pragma region 에디터 설정 데이터
public:
    /**
     * @brief 재생할 Niagara 궤적 이펙트.
     * @details BeamStart, BeamEnd Vector 파라미터를 받는 Niagara 에셋을 할당해야 합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|VFX")
    TObjectPtr<UNiagaraSystem> TrailVFX = nullptr;

    /**
     * @brief 칼날 시작점(코등이) 소켓 이름.
     * @details SM_KatanaBlade에 추가한 Socket_Blade_Root와 일치해야 합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|VFX")
    FName FirstSocketName = TEXT("Socket_Blade_Root");

    /**
     * @brief 칼날 끝점 소켓 이름.
     * @details SM_KatanaBlade에 추가한 Socket_Blade_Tip과 일치해야 합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|VFX")
    FName SecondSocketName = TEXT("Socket_Blade_Tip");

    /**
     * @brief Trail 전체 크기 배율 (_EnchantSize Niagara 파라미터로 전달됩니다).
     * @details 몽타주마다 다른 크기를 설정하려면 이 값을 조정합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KatanaNeon|VFX",
        meta = (ClampMin = 0.1f, ClampMax = 10.0f))
    float EnchantSize = 3.0f;
#pragma endregion 에디터 설정 데이터

#pragma region 런타임 상태
private:
    /**
     * @brief 코등이 소켓에 생성된 Niagara 컴포넌트.
     * @details TWeakObjectPtr 사용 — 파괴된 컴포넌트 참조 시 크래시 방지.
     */
    TWeakObjectPtr<UNiagaraComponent> SpawnedTrailRoot = nullptr;

    /**
     * @brief 칼끝 소켓에 생성된 Niagara 컴포넌트.
     * @details TWeakObjectPtr 사용 — 파괴된 컴포넌트 참조 시 크래시 방지.
     */
    TWeakObjectPtr<UNiagaraComponent> SpawnedTrailTip = nullptr;
#pragma endregion 런타임 상태

#pragma region 노티파이 오버라이드
public:
    /**
     * @brief 노티파이 구간 시작 — 카타나 메시에 궤적 VFX를 부착합니다.
     * @param MeshComp       애니메이션을 재생 중인 캐릭터 스켈레탈 메시
     * @param Animation      현재 재생 중인 애니메이션 에셋
     * @param TotalDuration  노티파이 바의 전체 지속 시간
     * @param EventReference 노티파이 이벤트 레퍼런스
     */
    virtual void NotifyBegin(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        float TotalDuration,
        const FAnimNotifyEventReference& EventReference) override;

    /**
     * @brief 노티파이 구간 종료 — 궤적 VFX를 비활성화합니다.
     * @param MeshComp       애니메이션을 재생 중인 캐릭터 스켈레탈 메시
     * @param Animation      현재 재생 중인 애니메이션 에셋
     * @param EventReference 노티파이 이벤트 레퍼런스
     */
    virtual void NotifyEnd(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;
#pragma endregion 노티파이 오버라이드
};
