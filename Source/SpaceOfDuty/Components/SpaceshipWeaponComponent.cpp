#include "Components/SpaceshipWeaponComponent.h"
#include "Components/SpaceshipMovementComponent.h"
#include "Components/SpaceshipTargetingComponent.h"
#include "Tools/PoolManagerComponent.h"
#include "Combat/SpaceshipProjectile.h"
#include "Combat/MissileSpaceshipProjectile.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

USpaceshipWeaponComponent::USpaceshipWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USpaceshipWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	TimeSinceLastShot = ShootingInterval;
	CurrentMissileCount = MaxMissiles;
}

void USpaceshipWeaponComponent::Initialize(UCameraComponent* InCamera, USceneComponent* InMuzzleLeft, USceneComponent* InMuzzleRight, USceneComponent* InMuzzleCenter,
	UPoolManagerComponent* InBulletPool, UPoolManagerComponent* InMissilePool, USpaceshipMovementComponent* InMovementComponent,
	USpaceshipTargetingComponent* InTargetingComponent)
{
	SpaceshipCamera = InCamera;
	MuzzleLeft = InMuzzleLeft;
	MuzzleRight = InMuzzleRight;
	MuzzleCenter = InMuzzleCenter;
	BulletPool = InBulletPool;
	MissilePool = InMissilePool;
	SpaceshipMovementComponent = InMovementComponent;
	SpaceshipTargetingComponent = InTargetingComponent;

	Muzzles.Empty();

	if (MuzzleLeft && MuzzleRight)
	{
		Muzzles.Add(MuzzleLeft);
		Muzzles.Add(MuzzleRight);
	}
}

void USpaceshipWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || GetOwner()->IsActorBeingDestroyed())
	{
		return;
	}

	CheckShooting(DeltaTime);
	CheckMissileCharging(DeltaTime);
}

void USpaceshipWeaponComponent::StartChargingMissile()
{
	if (EnableMissileCharging)
	{
		bIsChargingMissile = true;
	}
}

void USpaceshipWeaponComponent::StopChargingMissile()
{
	if (bIsChargingMissile)
	{
		if (SpaceshipTargetingComponent)
		{
			TArray<FEnemyTargetDistanceInfo> MissileTargets = SpaceshipTargetingComponent->GetTargets(CurrentMissileCount);

			LaunchMissiles(MissileTargets, MissilePool);
		}

		bIsChargingMissile = false;

		ResetMissileTargets();
	}
}

void USpaceshipWeaponComponent::StartShoot()
{
	bIsShooting = true;
}

void USpaceshipWeaponComponent::StopShoot()
{
	bIsShooting = false;
}

void USpaceshipWeaponComponent::CheckShooting(float DeltaTime)
{
	if (bIsShooting)
	{
		TimeSinceLastShot += DeltaTime;

		if (TimeSinceLastShot >= ShootingInterval)
		{
			Shoot(BulletPool);

			TimeSinceLastShot = 0.0f;
		}
	}
	else
	{
		if (TimeSinceLastShot != ShootingInterval)
		{
			TimeSinceLastShot = ShootingInterval;
		}
	}
}

void USpaceshipWeaponComponent::CheckMissileCharging(float DeltaTime)
{
	if (!EnableMissileCharging)
	{
		MissileCooldownTime += DeltaTime;

		if (MissileCooldownTime >= MissileCooldownDuration)
		{
			MissileCooldownTime = 0.0f;
			CurrentMissileCount = MaxMissiles;

			EnableMissileCharging = true;
		}

		return;
	}

	if (bIsChargingMissile)
	{
		if (SpaceshipTargetingComponent)
		{
			SpaceshipTargetingComponent->UpdateTargets(CurrentMissileCount);
		}

		MissileChargeTime += DeltaTime;

		if (MissileChargeTime >= MissileChargeDuration)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("Missile Charged!"));
			}
			MissileChargeTime = 0.0f;
			bIsChargingMissile = false;

			ResetMissileTargets();
		}
	}
	else
	{
		if (MissileChargeTime != 0.0f)
		{
			MissileChargeTime = 0.0f;
		}
	}
}

void USpaceshipWeaponComponent::Shoot(UPoolManagerComponent* InBulletPool)
{
	AActor* Owner = GetOwner();

	if (!Owner || !InBulletPool || Muzzles.Num() == 0) return;

	FVector TargetPoint = GetCrosshairDirection();

	for (USceneComponent* Muzzle : Muzzles)
	{
		if (!Muzzle) continue;

		FVector ToMuzzle = (Muzzle->GetComponentLocation() - Owner->GetActorLocation()).GetSafeNormal();
		float Dot = FVector::DotProduct(ToMuzzle, Owner->GetActorRightVector());
		bool IsRight = Dot > 0.1f;

		FVector SpawnLocation = Muzzle->GetComponentLocation();

		float MuzzleOffset = 150;
		FVector MuzzleOffsetVector = Owner->GetActorRightVector() * MuzzleOffset;
		FVector MuzzleTargetPoint = TargetPoint + (IsRight ? MuzzleOffsetVector : -MuzzleOffsetVector);

		FVector ShootDirection = (MuzzleTargetPoint - SpawnLocation).GetSafeNormal();
		FRotator SpawnRotation = ShootDirection.Rotation();

		AActor* PooledActor = InBulletPool->GetPooledActor();
		ASpaceshipProjectile* Projectile = Cast<ASpaceshipProjectile>(PooledActor);

		if (Projectile)
		{
			Projectile->SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
			Projectile->FireProjectileInDirection(ShootDirection, InBulletPool);
		}
	}
}

void USpaceshipWeaponComponent::LaunchMissiles(TArray<FEnemyTargetDistanceInfo>& Targets, UPoolManagerComponent* InMissilePool)
{
	AActor* Owner = GetOwner();

	if (!Owner || !InMissilePool || (Targets.Num() > 1 && Muzzles.Num() == 0) || (Targets.Num() == 1 && !MuzzleCenter))
	{
		return;
	}

	int index = 0;

	Targets.Sort([](const FEnemyTargetDistanceInfo& A, const FEnemyTargetDistanceInfo& B)
		{
			return A.ScreenPosition.X > B.ScreenPosition.X;
		});

	FVector TargetPoint = GetCrosshairDirection();

	for (FEnemyTargetDistanceInfo TargetInfo : Targets)
	{
		AActor* Target = TargetInfo.TargetActor;

		if (!Target) continue;

		FVector MuzzleLocation = Targets.Num() == 1 ?
			MuzzleCenter->GetComponentLocation() :
			Muzzles[index]->GetComponentLocation();

		bool IsRight = IsOnTheRightSide(MuzzleLocation);

		const float MuzzleOffset = 10000;
		FVector MuzzleOffsetVector = Owner->GetActorRightVector() * MuzzleOffset;
		FVector MuzzleTargetPoint = TargetPoint + (IsRight ? MuzzleOffsetVector : -MuzzleOffsetVector);

		FVector LaunchDirection = ((Targets.Num() == 1 ? TargetPoint : MuzzleTargetPoint) - MuzzleLocation).GetSafeNormal();

		AActor* PooledActor = InMissilePool->GetPooledActor();

		if (!PooledActor) continue;

		AMissileSpaceshipProjectile* Missile = Cast<AMissileSpaceshipProjectile>(PooledActor);

		if (Missile)
		{
			Missile->SetActorLocationAndRotation(MuzzleLocation, MuzzleLocation.Rotation());

			const float CurrentSpeed = SpaceshipMovementComponent ? SpaceshipMovementComponent->GetCurrentSpaceshipSpeed() : 0.0f;
			Missile->LaunchMissile(LaunchDirection, CurrentSpeed, Target, InMissilePool);

			CurrentMissileCount = FMath::Clamp(CurrentMissileCount - 1, 0, MaxMissiles);
		}

		index++;
		index = index % Muzzles.Num();
	}
}

void USpaceshipWeaponComponent::ResetMissileTargets()
{
	if (SpaceshipTargetingComponent)
	{
		SpaceshipTargetingComponent->ResetTargets();
	}

	EnableMissileCharging = false;
}

bool USpaceshipWeaponComponent::IsOnTheRightSide(FVector TargetLocation) const
{
	AActor* Owner = GetOwner();

	if (!Owner)
	{
		return false;
	}

	FVector ToTarget = (TargetLocation - Owner->GetActorLocation()).GetSafeNormal();
	float Dot = FVector::DotProduct(ToTarget, Owner->GetActorRightVector());

	return Dot > 0.1f;
}

FVector USpaceshipWeaponComponent::GetCrosshairDirection() const
{
	if (!SpaceshipCamera || !GetWorld())
	{
		return FVector::ZeroVector;
	}

	FVector CameraLocation = SpaceshipCamera->GetComponentLocation();
	FVector CameraForward = SpaceshipCamera->GetForwardVector();

	float TraceDistance = 100000.f;

	FVector TraceEnd = CameraLocation + (CameraForward * TraceDistance);

	FHitResult HitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, QueryParams);

	return bHit ? HitResult.ImpactPoint : TraceEnd;
}
