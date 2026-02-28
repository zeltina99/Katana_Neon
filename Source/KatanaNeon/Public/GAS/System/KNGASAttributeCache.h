// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"

/**
 * @file    KNGASAttributeCache.h
 * @brief   GAS 어트리뷰트 리플렉션 캐싱을 전담하는 공용 유틸리티 클래스입니다.
 * @details 단일 책임 원칙(SRP)과 DRY(Don't Repeat Yourself) 원칙에 따라,
 * 여러 Execution Calculation 클래스에서 중복되던 리플렉션 캐싱 로직을 한 곳으로 통합했습니다.
 */

#pragma region 어트리뷰트 캐시 매니저
 /**
  * @class  FKNGASAttributeCache
  * @brief  엔진 리플렉션 부하를 막기 위해 네이티브 태그와 어트리뷰트 매핑을 O(1) 해시맵으로 제공합니다.
  */
class KATANANEON_API FKNGASAttributeCache
{
public:
    /**
     * @brief 어트리뷰트 맵을 반환합니다. 최초 1회만 엔진 리플렉션을 수행하고 이후엔 캐시를 반환합니다.
     * @return FGameplayTag를 키로, FGameplayAttribute를 값으로 가지는 정적 캐시 맵
     */
    static const TMap<FGameplayTag, FGameplayAttribute>& Get();
};
#pragma endregion 어트리뷰트 캐시 매니저