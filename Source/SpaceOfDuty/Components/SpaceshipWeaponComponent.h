#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Combat/FEnemyTargetDistanceInfo.h"
#include "SpaceshipWeaponComponent.generated.h"

class UCameraComponent;
class USceneComponent;
class UPoolManagerComponent;
class USpaceshipMovementComponent;
class USpaceshipTargetingComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPACEOFDUTY_API USpaceshipWeaponComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USpaceshipWeaponComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Initialize(UCameraComponent* InCamera, USpaceshipMovementComponent* InMovementComponent, USpaceshipTargetingComponent* InTargetingComponent);

	void StartShoot();
	void StopShoot();
	void StartChargingMissile();
	void StopChargingMissile();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsShooting() const { return bIsShooting; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsChargingMissile() const { return bIsChargingMissile; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMissileCooldownDuration() const { return MissileCooldownDuration; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMissileCooldownTime() const { return MissileCooldownTime; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMissileChargeDuration() const { return MissileChargeDuration; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMissileChargeTime() const { return MissileChargeTime; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsMissileBeingCharged() const { return bIsChargingMissile; }

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ShootingInterval = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float MissileChargeDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	int32 MaxMissiles = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float MissileCooldownDuration = 20.0f;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float MissileCooldownTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool EnableMissileCharging = true;

	UPROPERTY()
	UCameraComponent* SpaceshipCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* MuzzleLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* MuzzleRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* MuzzleCenter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UPoolManagerComponent* BulletPool;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UPoolManagerComponent* MissilePool;

	UPROPERTY()
	USpaceshipMovementComponent* SpaceshipMovementComponent;

	UPROPERTY()
	USpaceshipTargetingComponent* SpaceshipTargetingComponent;

	UPROPERTY()
	TArray<USceneComponent*> Muzzles;

	float TimeSinceLastShot = 0.0f;
	float MissileChargeTime = 0.0f;
	int32 CurrentMissileCount = 0;
	bool bIsShooting = false;
	bool bIsChargingMissile = false;

	void CheckShooting(float DeltaTime);
	void CheckMissileCharging(float DeltaTime);
	void Shoot(UPoolManagerComponent* InBulletPool);
	void LaunchMissiles(TArray<FEnemyTargetDistanceInfo>& Targets, UPoolManagerComponent* InMissilePool);
	void ResetMissileTargets();
	bool IsOnTheRightSide(FVector TargetLocation) const;
	FVector GetCrosshairDirection() const;
};
