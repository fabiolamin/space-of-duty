#include "MissileSpaceshipProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/World.h"

AMissileSpaceshipProjectile::AMissileSpaceshipProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	ProjectileMovement->bIsHomingProjectile = true;
	ProjectileMovement->HomingAccelerationMagnitude = 30000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void AMissileSpaceshipProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckMissileState(DeltaTime);
}

void AMissileSpaceshipProjectile::CheckMissileState(float DeltaTime)
{
	if (MissileState == EMissileState::Launch)
	{
		CurrentLaunchTime += DeltaTime;

		if (CurrentLaunchTime >= LaunchDuration)
		{
			MissileState = EMissileState::Homing;
			ProjectileMovement->bIsHomingProjectile = true;
			ProjectileMovement->HomingTargetComponent = TargetActor->GetRootComponent();
		}
	}
}

void AMissileSpaceshipProjectile::LaunchMissile(const FVector& Direction, float InheritedVelocity, AActor* Target, UPoolManagerComponent* InProjectilePool)
{
	if (!Target || !InProjectilePool)
	{
		return;
	}

	TargetActor = Target;
	CurrentLaunchTime = 0.f;

	MissileState = EMissileState::Launch;

	ProjectileMovement->bIsHomingProjectile = false;

	float MissileSpeed = ProjectileSpeed + InheritedVelocity;

	FireProjectileInDirection(Direction, MissileSpeed, InProjectilePool);
}

void AMissileSpaceshipProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ProjectileMovement->HomingTargetComponent = nullptr;

	Super::OnProjectileHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}