#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Combat/SpaceshipProjectile.h"
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


private:

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* SpaceshipMesh;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* SpaceshipCamera;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Camera")
	TSubclassOf<UCameraShakeBase> SpeedCameraShake;

	UCameraShakeBase* ActiveCameraShake;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpaceshipSpringArm;

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
	float MaxTimeSinceLastLookInput = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float MaxSpaceshipRoll = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float SpaceshipRollInterpSpeed = 4.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float SpaceshipFOVInterpSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float SpaceshipRollSpeed = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float MaxTimeSinceLastMoveInput = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float SpaceshipMovementInterpSpeed = 4.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float SpaceshipBoostInterpSpeed = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float MinSpaceshipSpeed = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float MaxSpaceshipSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float BoostSpaceshipSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float YawRotationSpeed = 75.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float PitchRotationSpeed = 40.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float MaxPitch = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float BoostCameraDeltaFOV = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* MuzzleLeft;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* MuzzleRight;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ASpaceshipProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ShootingInterval;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float BaseProjectileSpeed = 35000;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))

	float TimeSinceLastLookInput = 0.0f;
	float TimeSinceLastMoveInput = 0.0f;
	float TimeSinceLastShot = 0.0f;
	float CurrentSpaceshipSpeed;
	float TargetSpaceshipSpeed;
	float DefaultSpaceshipRoll = 0.0f;
	float DefaultCameraFOV;

	bool IsBoosting = false;
	bool IsAiming = false;
	bool IsShooting = false;

	TArray<USceneComponent*> GetMuzzleComponents() const;

	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);

	void StartBoost(const struct FInputActionValue& Value);
	void StopBoost(const struct FInputActionValue& Value);

	void StartAim(const struct FInputActionValue& Value);
	void StopAim(const struct FInputActionValue& Value);

	void StartShoot(const struct FInputActionValue& Value);
	void StopShoot(const struct FInputActionValue& Value);

	void CheckShooting(float DeltaTime);
	void Shoot();
};
