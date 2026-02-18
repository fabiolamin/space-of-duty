#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
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
	class UInputAction* ForwardAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float MaxTimeSinceLastLookInput = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float MaxSpaceshipRoll = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float SpaceshipRollInterpSpeed = 4.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float SpaceshipRollSpeed = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float MaxTimeSinceLastMoveInput = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float SpaceshipMovementInterpSpeed = 4.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float MinSpaceshipSpeed = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float MaxSpaceshipSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float YawRotationSpeed = 75.0f;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float PitchRotationSpeed = 40.0f;

	float TimeSinceLastLookInput = 0.0f;
	float TimeSinceLastMoveInput = 0.0f;
	float CurrentSpaceshipSpeed;
	float DefaultSpaceshipRoll = 0.0f;

	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
};
