#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FEnemyTargetDistanceInfo.generated.h"

USTRUCT()
struct SPACEOFDUTY_API FEnemyTargetDistanceInfo
{
	GENERATED_BODY()

public:
	FEnemyTargetDistanceInfo()
		: TargetActor(nullptr)
		, Distance(0.0f)
		, ScreenPosition(FVector2D::ZeroVector)
	{
	}

	FEnemyTargetDistanceInfo(AActor* InTargetActor, float InDistance)
		: TargetActor(InTargetActor)
		, Distance(InDistance)
	{
	}

	FEnemyTargetDistanceInfo(AActor* InTargetActor, float InDistance, FVector2D InScreenPosition)
		: TargetActor(InTargetActor)
		, Distance(InDistance)
		, ScreenPosition(InScreenPosition)
	{
	}

	UPROPERTY()
	AActor* TargetActor;

	UPROPERTY()
	float Distance;

	UPROPERTY()
	FVector2D ScreenPosition;
};
