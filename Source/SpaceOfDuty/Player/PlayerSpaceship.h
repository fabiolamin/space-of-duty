#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Tools/PoolManagerComponent.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Weapon")
	class USpaceshipWeaponComponent* SpaceshipWeaponComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Targeting")
	class USpaceshipTargetingComponent* SpaceshipTargetingComponent;


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



	float DefaultCameraFOV;
	float TargetSpaceshipFOV;
	float TargetSpaceshipFOVInterpSpeed;

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

	void CheckSpaceshipBoosting(float DeltaTime);
	void UpdateSpaceshipCameraFOV(float DeltaTime);
};
