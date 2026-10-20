#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/FEnemyTargetDistanceInfo.h"
#include "SpaceshipTargetingComponent.generated.h"

class UMaterialInterface;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPACEOFDUTY_API USpaceshipTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpaceshipTargetingComponent();

	// Scans all known enemies, updates the detected targets (sorted by screen distance) and highlights the first MaxTargets ones.
	void UpdateTargets(int32 MaxTargets);

	// Returns up to MaxTargets of the closest detected targets to the screen center.
	TArray<FEnemyTargetDistanceInfo> GetTargets(int32 MaxTargets) const;

	// Clears detected targets and restores the default enemy material.
	void ResetTargets();

	UFUNCTION(BlueprintPure, Category = "Targeting")
	int32 GetDetectedTargetCount() const { return DetectedTargets.Num(); }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float MaxTargetDistance = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float MinTargetDistance = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float TargetLockRadius = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	FName EnemyTag = FName(TEXT("Enemy2"));

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* DefaultEnemyMaterial;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* TargetEnemyMaterial;

	UPROPERTY()
	TArray<FEnemyTargetDistanceInfo> DetectedTargets;

	//TODO: Temporary enemy management, to be replaced with a more robust system later
	UPROPERTY()
	TArray<AActor*> Enemies;

	void ScanEnemyInView(AActor* Target);
	void RemoveTarget(AActor* Target);
	void SetEnemyMaterial(AActor* Enemy, UMaterialInterface* Material);
};
