#include "Components/SpaceshipMovementComponent.h"
#include "InputActionValue.h"

USpaceshipMovementComponent::USpaceshipMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USpaceshipMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USpaceshipMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if(!IsInitialized)
	{
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Move(DeltaTime);
	CheckSpaceshipBoosting(DeltaTime);
	UpdateSpaceshipRoll(DeltaTime);
	CheckIdleMovement(DeltaTime);
}

void USpaceshipMovementComponent::Move(float DeltaTime)
{
	if (PawnOwner)
	{
		PawnOwner->AddActorWorldOffset(PawnOwner->GetActorForwardVector() * DeltaTime * CurrentSpaceshipSpeed, true);
	}
}

void USpaceshipMovementComponent::CheckIdleMovement(float DeltaTime)
{
	TimeSinceLastLook += DeltaTime;
	TimeSinceLastMovement += DeltaTime;

	if (!IsBoosting && TimeSinceLastMovement >= MaxTimeSinceLastMovement)
	{
		CurrentSpaceshipSpeed = FMath::FInterpTo(
			CurrentSpaceshipSpeed,
			MinSpaceshipSpeed,
			DeltaTime,
			SpaceshipMovementInterpSpeed);
	}

	if (TimeSinceLastLook >= MaxTimeSinceLastLook)
	{
		TargetSpaceshipRoll = DefaultSpaceshipRoll;
	}
}

void USpaceshipMovementComponent::CheckSpaceshipBoosting(float DeltaTime)
{
	if (IsBoosting)
	{
		BoostAmount = FMath::Clamp(BoostAmount - (DeltaTime * BoostDepletionRate), 0.f, 1.f);
	}
	else
	{
		BoostAmount = FMath::Clamp(BoostAmount + (DeltaTime * BoostRechargeRate), 0.f, 1.f);
	}

	if (BoostAmount <= 0.f)
	{
		ResetBoost();
	}

	if (IsBoosting)
	{

		AController* Controller = PawnOwner->GetController();

		if (Controller)
		{
			CurrentSpaceshipSpeed = FMath::FInterpTo(
				CurrentSpaceshipSpeed,
				BoostSpaceshipSpeed,
				DeltaTime,
				SpaceshipBoostInterpSpeed);

			CurrentBoostSpeedRatio = CurrentSpaceshipSpeed / BoostSpaceshipSpeed;
		}
	}
}

void USpaceshipMovementComponent::UpdateSpaceshipRoll(float DeltaTime)
{
	FRotator CurrentRotation = SpaceshipMesh->GetRelativeRotation();

	if (!FMath::IsNearlyEqual(CurrentRotation.Roll, TargetSpaceshipRoll, 0.1f))
	{
		float NewRoll = FMath::FInterpTo(CurrentRotation.Roll, TargetSpaceshipRoll, DeltaTime, SpaceshipRollInterpSpeed);
		SpaceshipMesh->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, NewRoll));
	}
}

void USpaceshipMovementComponent::Initialize(UStaticMeshComponent* InSpaceshipMesh)
{
	SpaceshipMesh = InSpaceshipMesh;

	DefaultSpaceshipRoll = SpaceshipMesh->GetRelativeRotation().Roll;
	CurrentSpaceshipSpeed = MinSpaceshipSpeed;
	TargetSpaceshipRoll = DefaultSpaceshipRoll;
	TargetSpaceshipSpeed = MaxSpaceshipSpeed;

	IsInitialized = true;
}

void USpaceshipMovementComponent::Accelerate(FVector2D MovementVector)
{
	if (!IsInitialized)
	{
		return;
	}

	if (MovementVector.Y < 0.0f)
	{
		MovementVector.Y = 0.0f;
	}
	else if (!IsBoosting && MovementVector.Y > 0.0f)
	{
		TimeSinceLastMovement = 0.0f;

		CurrentSpaceshipSpeed = FMath::FInterpTo(
			CurrentSpaceshipSpeed,
			MovementVector.Y * MaxSpaceshipSpeed,
			GetWorld()->GetDeltaSeconds(),
			SpaceshipMovementInterpSpeed);
	}
}

void USpaceshipMovementComponent::Look(FVector2D LookVector)
{
	if (!IsInitialized)
	{
		return;
	}

	const float BaseSensitivity = 0.095f;
	const float AccelerationStrength = 0.087f;
	const float MaxSensitivity = 0.12f;
	const float AccelPower = 1.2f;

	float MouseSpeed = LookVector.Size();
	float Acceleration = FMath::Pow(MouseSpeed, AccelPower) * AccelerationStrength;

	float SensitivityMultiplier = FMath::Clamp(Acceleration, BaseSensitivity, MaxSensitivity);

	const FVector2D NewLookVector = LookVector * SensitivityMultiplier;

	TimeSinceLastLook = 0.0f;

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	if (PawnOwner)
	{
		FRotator NewRot = PawnOwner->GetActorRotation();

		NewRot.Yaw += NewLookVector.X;
		NewRot.Pitch += NewLookVector.Y;
		NewRot.Pitch = FMath::Clamp(NewRot.Pitch, -MaxPitch, MaxPitch);

		PawnOwner->SetActorRotation(NewRot);
	}

	const float YawInput = NewLookVector.X;

	const float YawSpeedDegPerSec = (DeltaTime > KINDA_SMALL_NUMBER) ? (YawInput / DeltaTime) : 0.0f;

	const float MaxYawSpeedDegPerSec = 120.0f;

	const float NormalizedYawSpeed = FMath::Clamp(YawSpeedDegPerSec / MaxYawSpeedDegPerSec, -1.0f, 1.0f);

	const float SpeedFactor = (MaxSpaceshipSpeed > KINDA_SMALL_NUMBER) ? FMath::Clamp(CurrentSpaceshipSpeed / MaxSpaceshipSpeed, 0.0f, 1.0f) : 0.0f;
	const float SpeedRollScale = FMath::Lerp(1.0f, 1.5f, SpeedFactor);

	float TargetRoll = NormalizedYawSpeed * MaxSpaceshipRoll * SpeedRollScale;

	TargetRoll = FMath::Clamp(TargetRoll, -MaxSpaceshipRoll, MaxSpaceshipRoll);

	TargetSpaceshipRoll = TargetRoll;
}

void USpaceshipMovementComponent::StartBoost()
{
	if (!IsInitialized || BoostAmount <= 0.f)
	{
		return;
	}

	IsBoosting = true;

	TargetSpaceshipSpeed = BoostSpaceshipSpeed;
}

void USpaceshipMovementComponent::StopBoost()
{
	if (!IsInitialized)
	{
		return;
	}

	IsBoosting = false;
	TargetSpaceshipSpeed = MaxSpaceshipSpeed;
}

void USpaceshipMovementComponent::ResetBoost()
{
	if (!IsInitialized)
	{
		return;
	}

	IsBoosting = false;
	TargetSpaceshipSpeed = MaxSpaceshipSpeed;

	OnBoostReset.Broadcast();
}
