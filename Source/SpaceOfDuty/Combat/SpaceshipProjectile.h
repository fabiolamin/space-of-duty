#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceshipProjectile.generated.h"


class UPoolManagerComponent;


UCLASS()
class SPACEOFDUTY_API ASpaceshipProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASpaceshipProjectile();

	virtual void Tick(float DeltaTime) override;

	void FireProjectileInDirection(const FVector& Direction, UPoolManagerComponent* InBulletPool);
	void FireProjectileInDirection(const FVector& Direction, float Speed, UPoolManagerComponent* InProjectilePool);

protected:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float ProjectileSpeed;

	UPROPERTY(EditAnywhere, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	float Damage;

	UPROPERTY(EditAnywhere, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	float LifeSpan = 5.f;

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY()
	UPoolManagerComponent* ProjectilePool;

	UFUNCTION()
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SetProjectilePool(UPoolManagerComponent* InProjectilePool);

	virtual void BeginPlay() override;

private:
	void CheckLifeSpan(float DeltaTime);

	float CurrentLifeSpan;
};
