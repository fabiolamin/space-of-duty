#pragma once

#include "CoreMinimal.h"
#include "Combat/SpaceshipProjectile.h"
#include "MissileSpaceshipProjectile.generated.h"

enum class EMissileState : uint8
{
	Launch,
	Homing,
};

UCLASS()
class SPACEOFDUTY_API AMissileSpaceshipProjectile : public ASpaceshipProjectile
{
	GENERATED_BODY()

public:
	AMissileSpaceshipProjectile();
	virtual void Tick(float DeltaTime) override;

	void CheckMissileState(float DeltaTime);

	void LaunchMissile(const FVector& Direction, float InheritedVelocity, AActor* Target, UPoolManagerComponent* InProjectilePool);

private:
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float LaunchDuration = 2.f;

	AActor* TargetActor = nullptr;
	float CurrentLaunchTime = 0.f;
	EMissileState MissileState = EMissileState::Launch;
	FVector LaunchInheritedVelocity = FVector::ZeroVector;

protected:
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
};
