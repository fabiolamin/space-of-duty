#include "Components/SpaceshipCameraComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

USpaceshipCameraComponent::USpaceshipCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USpaceshipCameraComponent::Initialize(UCameraComponent* InCamera)
{
	Camera = InCamera;

	if (IsValid(Camera))	
	{
		DefaultCameraFOV = Camera->FieldOfView;
		TargetFOV = DefaultCameraFOV;
	}
}

void USpaceshipCameraComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopBoostShake();

	Super::EndPlay(EndPlayReason);
}

void USpaceshipCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateCameraFOV(DeltaTime);
}

void USpaceshipCameraComponent::StartBoostEffects(float InterpSpeed)
{
	TargetFOV = DefaultCameraFOV + BoostCameraDeltaFOV;
	TargetFOVInterpSpeed = InterpSpeed;
}

void USpaceshipCameraComponent::StopBoostEffects(float InterpSpeed)
{
	TargetFOV = DefaultCameraFOV;
	TargetFOVInterpSpeed = InterpSpeed;
}

void USpaceshipCameraComponent::UpdateCameraFOV(float DeltaTime)
{
	if (IsValid(Camera) && !FMath::IsNearlyEqual(Camera->FieldOfView, TargetFOV, 0.1f))
	{
		const float NewFOV = FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaTime, TargetFOVInterpSpeed);
		Camera->SetFieldOfView(NewFOV);
	}
}

void USpaceshipCameraComponent::UpdateBoostShake(bool bIsBoosting, float ShakeScale)
{
	if (!bIsBoosting)
	{
		StopBoostShake();
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;

	if (!PC || !PC->PlayerCameraManager || !SpeedCameraShake)
	{
		return;
	}

	if (IsValid(ActiveCameraShake))
	{
		return;
	}

	if (BoostKickCameraShake)
	{
		PC->PlayerCameraManager->StartCameraShake(BoostKickCameraShake, BoostKickShakeIntensity);
	}

	ActiveCameraShake = PC->PlayerCameraManager->StartCameraShake(SpeedCameraShake, SpeedShakeIntensity);
}

void USpaceshipCameraComponent::StopBoostShake()
{
	if (!IsValid(ActiveCameraShake))
	{		
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;

	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StopCameraShake(ActiveCameraShake);
		ActiveCameraShake = nullptr;
	}
}

void USpaceshipCameraComponent::OnPlayerDamaged(float DamagePercentage)
{
	if (!StrongDamageCameraShake || DamagePercentage < StrongDamageThreshold)
	{
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;

	if (PC && PC->PlayerCameraManager)
	{
		const float Scale = StrongDamageShakeIntensity * FMath::GetMappedRangeValueClamped(FVector2D(StrongDamageThreshold, 1.0f), FVector2D(MinDamageShakeScale, MaxDamageShakeScale), DamagePercentage);
		PC->PlayerCameraManager->StartCameraShake(StrongDamageCameraShake, Scale);
	}
}
