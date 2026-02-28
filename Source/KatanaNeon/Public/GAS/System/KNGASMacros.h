// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"

#pragma region GAS 어트리뷰트 공통 매크로
/**
 * @brief 어트리뷰트에 대한 Getter, Setter, Initter를 자동 생성하는 공용 매크로입니다.
 * @details 여러 AttributeSet 클래스에서 보일러플레이트 코드를 줄이기 위해 별도 헤더로 캡슐화했습니다.
 * @param ClassName AttributeSet 클래스 이름
 * @param PropertyName 어트리뷰트 프로퍼티 이름
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
#pragma endregion GAS 어트리뷰트 공통 매크로 끝