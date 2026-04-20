#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceshipProjectile.generated.h"



UCLASS()
class SPACEOFDUTY_API ASpaceshipProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASpaceshipProjectile();

	virtual void Tick(float DeltaTime) override;

	void InitProjectile(const FVector& Direction, float );


protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	float Damage;

	UPROPERTY(EditAnywhere, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	float LifeSpan;

	FVector ProjectileDirection;
	float ProjectileSpeed;
};
