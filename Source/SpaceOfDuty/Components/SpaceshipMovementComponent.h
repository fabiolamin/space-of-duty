#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SpaceshipMovementComponent.generated.h"


class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBoostReset);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPACEOFDUTY_API USpaceshipMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	USpaceshipMovementComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void Move(float DeltaTime);
	void Initialize(UStaticMeshComponent* InSpaceshipMesh);
	void Accelerate(FVector2D MovementVector);
	void Look(FVector2D LookVector);
	void StartBoost();
	void StopBoost();
	void ResetBoost();
	bool IsSpaceshipBoosting() const { return IsBoosting; }
	float GetCurrentBoostSpeedRatio() const { return CurrentBoostSpeedRatio; }
	float GetSpaceshipMovementInterpSpeed() const { return SpaceshipMovementInterpSpeed; }
	float GetSpaceshipBoostInterpSpeed() const { return SpaceshipBoostInterpSpeed; }
	float GetCurrentSpaceshipSpeed() const { return CurrentSpaceshipSpeed; }

	UFUNCTION(BlueprintPure, Category = "Boost")
	float GetBoostAmount() const { return BoostAmount; }

	UPROPERTY(BlueprintAssignable, Category = "Boost")
	FOnBoostReset OnBoostReset;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UStaticMeshComponent* SpaceshipMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Speed", meta = (AllowPrivateAccess = "true"))
	float SpaceshipMovementInterpSpeed = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Speed", meta = (AllowPrivateAccess = "true"))
	float SpaceshipBoostInterpSpeed = 2.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Speed", meta = (AllowPrivateAccess = "true"))
	float MinSpaceshipSpeed = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Speed", meta = (AllowPrivateAccess = "true"))
	float MaxSpaceshipSpeed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Speed", meta = (AllowPrivateAccess = "true"))
	float BoostSpaceshipSpeed = 2000.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Speed", meta = (AllowPrivateAccess = "true"))
	float SpaceshipRollInterpSpeed = 4.0f;


	UPROPERTY(EditDefaultsOnly, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float MaxSpaceshipRoll = 25.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float MaxPitch = 80.0f;


	UPROPERTY(EditDefaultsOnly, Category = "Timer", meta = (AllowPrivateAccess = "true"))
	float MaxTimeSinceLastMovement = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Timer", meta = (AllowPrivateAccess = "true"))
	float MaxTimeSinceLastLook = 0.1f;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float BoostAmount = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float BoostDepletionRate = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float BoostRechargeRate = 0.2f;

	float TimeSinceLastLook = 0.0f;
	float TimeSinceLastMovement = 0.0f;
	float TargetSpaceshipRoll;
	float CurrentSpaceshipSpeed;
	float CurrentBoostSpeedRatio = 0.0f;
	float DefaultSpaceshipRoll = 0.0f;
	bool IsBoosting = false;
	bool IsInitialized = false;
	float TargetSpaceshipSpeed;

	void CheckIdleMovement(float DeltaTime);
	void CheckSpaceshipBoosting(float DeltaTime);
	void UpdateSpaceshipRoll(float DeltaTime);
};
