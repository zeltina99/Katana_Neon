// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/System/KNGASAttributeCache.h"
#include "GAS/Attributes/KNAttributeSet.h"

#pragma region 어트리뷰트 캐시 매니저 구현
const TMap<FGameplayTag, FGameplayAttribute>& FKNGASAttributeCache::Get()
{
    // 정적(Static) 변수로 선언되어 프로그램 라이프사이클 동안 단 한 번만 초기화됩니다.
    static TMap<FGameplayTag, FGameplayAttribute> CachedMap;
    static bool bIsInitialized = false;

    if (!bIsInitialized)
    {
        for (TFieldIterator<FProperty> It(UKNAttributeSet::StaticClass()); It; ++It)
        {
            if (FStructProperty* StructProp = CastField<FStructProperty>(*It))
            {
                if (StructProp->Struct->GetFName() == TEXT("GameplayAttributeData"))
                {
                    // "KatanaNeon.Data.Stats.XXX" 형태의 태그 문자열 조합
                    FString TagName = FString::Printf(TEXT("KatanaNeon.Data.Stats.%s"), *StructProp->GetName());
                    FGameplayTag MappedTag = FGameplayTag::RequestGameplayTag(FName(*TagName), false);

                    if (MappedTag.IsValid())
                    {
                        CachedMap.Add(MappedTag, FGameplayAttribute(StructProp));
                    }
                }
            }
        }
        bIsInitialized = true;
    }

    return CachedMap;
}
#pragma endregion 어트리뷰트 캐시 매니저 구현