// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KNInputDataConfig.generated.h"

#pragma region 전방 선언
class UInputAction;
class UInputMappingContext;
#pragma endregion 전방 선언 끝

/**
 * @class  UKNInputDataConfig
 * @brief  캐릭터가 사용하는 모든 Enhanced Input 에셋(Context, Action)을 모아두는 데이터 에셋입니다.
 * @details 캐릭터 클래스의 멤버 변수 비대화를 막고, 기획자가 에디터에서 입력 키를
 * 쉽게 교체(Data-Driven)할 수 있도록 단일 책임 원칙(SRP)에 따라 분리되었습니다.
 */
UCLASS()
class KATANANEON_API UKNInputDataConfig : public UDataAsset
{
	GENERATED_BODY()
	
#pragma region 입력 매핑 컨텍스트
public:
    /** @brief 게임 플레이 시 기본적으로 적용될 입력 매핑 컨텍스트 (IMC) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Context")
    TObjectPtr<UInputMappingContext> DefaultMappingContext = nullptr;
#pragma endregion 입력 매핑 컨텍스트

#pragma region 기본 이동 및 시점 액션
public:
    /** @brief 캐릭터 이동 (WASD, 좌스틱) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Action")
    TObjectPtr<UInputAction> MoveAction = nullptr;

    /** @brief 카메라 시점 회전 (마우스, 우스틱) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Action")
    TObjectPtr<UInputAction> LookAction = nullptr;

    /** @brief 점프 (SpaceBar, A버튼) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Movement")
    TObjectPtr<UInputAction> JumpAction = nullptr;

    /** @brief 달리기 (Shift 유지, L3 클릭) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Movement")
    TObjectPtr<UInputAction> SprintAction = nullptr;

    /** @brief 다크소울식 타겟팅 락온 토글 (마우스 휠 클릭, R3 클릭) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Camera")
    TObjectPtr<UInputAction> LockOnAction = nullptr;
#pragma endregion 기본 이동 및 시점 액션

#pragma region 전투 액션
public:
    /** @brief 무기 휘두르기 / 콤보 공격 (좌클릭, X버튼) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Combat")
    TObjectPtr<UInputAction> AttackAction = nullptr;

    /**
     * @brief 강공격 — 콤보 도중 우클릭 시 현재 콤보 단계의 강공격으로 파생됩니다. (우클릭, Y버튼)
     * @details 단독 입력 시 강공격 1단계, 약공 N회 후 입력 시 강공격 N+1단계가 발동됩니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Combat")
    TObjectPtr<UInputAction> HeavyAttackAction = nullptr;

    /** @brief 적 공격 튕겨내기 / 가드 (E, RB/R1 버튼) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Combat")
    TObjectPtr<UInputAction> ParryAction = nullptr;

    /** @brief 순간 이동 및 저스트 회피 판정 (스태미나 소모 / Alt, B버튼) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Combat")
    TObjectPtr<UInputAction> DashAction = nullptr;
#pragma endregion 전투 액션

#pragma region 특수 스킬 액션 (Skills)
public:
    /** @brief 시간 감속 모드 토글 (크로노스 게이지 소모 / Q, LT/L2 버튼) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Skill")
    TObjectPtr<UInputAction> ChronosAction = nullptr;

    /** @brief 오버클럭 Lv1 전술기 사용 (1번 키) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Skill")
    TObjectPtr<UInputAction> OverclockLv1Action = nullptr;

    /** @brief 오버클럭 Lv2 돌파기 사용 (2번 키) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Skill")
    TObjectPtr<UInputAction> OverclockLv2Action = nullptr;

    /** @brief 오버클럭 Lv3 궁극기 사용 (3번 키) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Skill")
    TObjectPtr<UInputAction> OverclockLv3Action = nullptr;
#pragma endregion 특수 스킬 액션

#pragma region 유틸리티 및 상호작용 (Utility)
public:
    /** @brief 문 열기, NPC 대화, 아이템 줍기 등 환경 상호작용 (F, Y버튼) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Utility")
    TObjectPtr<UInputAction> InteractAction = nullptr;

    /** @brief 체력 회복 포션 사용 (R, D-Pad 위) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|Utility")
    TObjectPtr<UInputAction> PotionAction = nullptr;

    /** @brief 게임 일시정지 및 시스템 메뉴 호출 (ESC, Start 버튼) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KatanaNeon|Input|System")
    TObjectPtr<UInputAction> PauseMenuAction = nullptr;
#pragma endregion 유틸리티 및 상호작용
};
