#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Combat/SpaceshipProjectile.h"
#include "Tools/PoolManagerComponent.h"
#include <Combat/FEnemyTargetDistanceInfo.h>
#include "PlayerSpaceship.generated.h"

UCLASS()
class SPACEOFDUTY_API APlayerSpaceship : public APawn
{
	GENERATED_BODY()

public:
	APlayerSpaceship();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* SpaceshipMesh;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* SpaceshipCamera;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Camera")
	TSubclassOf<UCameraShakeBase> SpeedCameraShake;

	UCameraShakeBase* ActiveCameraShake = nullptr;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpaceshipSpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Movement")
	class USpaceshipMovementComponent* SpaceshipMovementComponent;



	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* BoostAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* AimAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* ShootAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float BoostCameraDeltaFOV = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* MuzzleLeft;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* MuzzleRight;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* MuzzleCenter;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UPoolManagerComponent* BulletPool;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UPoolManagerComponent* MissilePool;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ShootingInterval;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float MissileChargeDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float MissileChargeDepletionDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float MaxMissileDistance = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float MinMissileDistance = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	int MaxMissiles = 2;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float TargetLockRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float MissileCooldownDuration = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float MissileCooldownTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool EnableMissileCharging = true;

	float TimeSinceLastShot = 0.0f;
	float TimeSinceLastCameraFOVUpdate = 0.0f;
	float MissileChargeTime = 0.0f;
	float DefaultCameraFOV;
	bool IsAiming = false;
	bool IsShooting = false;
	bool IsChargingMissile = false;
	float TargetSpaceshipFOV;
	float TargetSpaceshipFOVInterpSpeed;

	UPROPERTY()
	TArray<FEnemyTargetDistanceInfo> MissileTargets;

	UPROPERTY()
	TArray<FEnemyTargetDistanceInfo> DetectedMissileTargets;

	int CurrentMissileCount = 0;

	UPROPERTY()
	TArray<USceneComponent*> Muzzles;

	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);

	void StartBoost(const struct FInputActionValue& Value);
	void StopBoost(const struct FInputActionValue& Value);

	UFUNCTION()
	void ResetSpaceshipBoost();

	void StartChargingMissile(const struct FInputActionValue& Value);
	void StopChargingMissile(const struct FInputActionValue& Value);

	void StartShoot(const struct FInputActionValue& Value);
	void StopShoot(const struct FInputActionValue& Value);

	void CheckShooting(float DeltaTime);
	void CheckMissileCharging(float DeltaTime);
	void CheckSpaceshipBoosting(float DeltaTime);
	void UpdateSpaceshipCameraFOV(float DeltaTime);
	void Shoot(UPoolManagerComponent* BulletPool);
	void LaunchMissiles(TArray<FEnemyTargetDistanceInfo>& Targets, UPoolManagerComponent* InMissilePool);
	void ScanEnemyInView(AActor* Target);
	void ResetMissileTargets();
	void RemoveMissileTarget(AActor* Target);

	bool IsOnTheRightSide(FVector TargetLocation);

	FVector GetCrosshairDirection();

	//TODO: Temporary properties for enemy management, to be replaced with a more robust system later
	UPROPERTY()
	TArray<AActor*> Enemies;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* DefaultEnemyMaterial;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* TargetEnemyMaterial;;
};
