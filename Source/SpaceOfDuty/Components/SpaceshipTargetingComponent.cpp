#include "Components/SpaceshipTargetingComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

USpaceshipTargetingComponent::USpaceshipTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USpaceshipTargetingComponent::BeginPlay()
{
	Super::BeginPlay();

	Enemies.Empty();

	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), EnemyTag, FoundEnemies);

	for (AActor* EnemyActor : FoundEnemies)
	{
		if (EnemyActor)
		{
			Enemies.Add(EnemyActor);
		}
	}
}

void USpaceshipTargetingComponent::UpdateTargets(int32 MaxTargets)
{
	for (AActor* Enemy : Enemies)
	{
		if (Enemy)
		{
			ScanEnemyInView(Enemy);
		}
	}

	DetectedTargets.Sort([](const FEnemyTargetDistanceInfo& A, const FEnemyTargetDistanceInfo& B)
		{
			return A.Distance < B.Distance;
		});

	const int32 LoopCount = FMath::Min(MaxTargets, DetectedTargets.Num());

	for (int32 i = 0; i < DetectedTargets.Num(); ++i)
	{
		SetEnemyMaterial(DetectedTargets[i].TargetActor, i < LoopCount ? TargetEnemyMaterial : DefaultEnemyMaterial);
	}
}

TArray<FEnemyTargetDistanceInfo> USpaceshipTargetingComponent::GetTargets(int32 MaxTargets) const
{
	TArray<FEnemyTargetDistanceInfo> Result;

	const int32 LoopCount = FMath::Min(MaxTargets, DetectedTargets.Num());

	for (int32 i = 0; i < LoopCount; ++i)
	{
		Result.Add(DetectedTargets[i]);
	}

	return Result;
}

void USpaceshipTargetingComponent::ResetTargets()
{
	for (AActor* Enemy : Enemies)
	{
		SetEnemyMaterial(Enemy, DefaultEnemyMaterial);
	}

	DetectedTargets.Empty();
}

void USpaceshipTargetingComponent::ScanEnemyInView(AActor* Target)
{
	AActor* Owner = GetOwner();

	if (!Owner || !Target || !GEngine || !GEngine->GameViewport || !GEngine->GameViewport->Viewport)
	{
		return;
	}

	const float DistanceToEnemy = FVector::Distance(Owner->GetActorLocation(), Target->GetActorLocation());

	if (DistanceToEnemy < MinTargetDistance || DistanceToEnemy > MaxTargetDistance)
	{
		RemoveTarget(Target);
		return;
	}

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(PawnOwner->GetController());

	if (!PlayerController)
	{
		return;
	}

	FVector2D TargetScreenPosition;
	const bool bOnScreen = PlayerController->ProjectWorldLocationToScreen(Target->GetActorLocation(), TargetScreenPosition);

	const FIntPoint ViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();

	if (!bOnScreen || TargetScreenPosition.X < 0.f || TargetScreenPosition.X > ViewportSize.X ||
		TargetScreenPosition.Y < 0.f || TargetScreenPosition.Y > ViewportSize.Y)
	{
		RemoveTarget(Target);
		return;
	}

	const FVector2D ScreenCenter = FVector2D(ViewportSize) * 0.5f;
	const float ScreenDistance = FVector2D::Distance(TargetScreenPosition, ScreenCenter);

	if (ScreenDistance <= TargetLockRadius)
	{
		const int32 ExistingIndex = DetectedTargets.IndexOfByPredicate([Target](const FEnemyTargetDistanceInfo& Info) { return Info.TargetActor == Target; });

		if (ExistingIndex == INDEX_NONE)
		{
			DetectedTargets.Add(FEnemyTargetDistanceInfo(Target, ScreenDistance, TargetScreenPosition));
		}
	}
	else
	{
		RemoveTarget(Target);
	}
}

void USpaceshipTargetingComponent::RemoveTarget(AActor* Target)
{
	if (Target && DetectedTargets.ContainsByPredicate([Target](const FEnemyTargetDistanceInfo& Info) { return Info.TargetActor == Target; }))
	{
		DetectedTargets.RemoveAll([Target](const FEnemyTargetDistanceInfo& Info) { return Info.TargetActor == Target; });

		SetEnemyMaterial(Target, DefaultEnemyMaterial);
	}
}

void USpaceshipTargetingComponent::SetEnemyMaterial(AActor* Enemy, UMaterialInterface* Material)
{
	if (!Enemy)
	{
		return;
	}

	UStaticMeshComponent* Mesh = Enemy->FindComponentByClass<UStaticMeshComponent>();

	if (Mesh && Mesh->GetMaterial(0) != Material)
	{
		Mesh->SetMaterial(0, Material);
	}
}
