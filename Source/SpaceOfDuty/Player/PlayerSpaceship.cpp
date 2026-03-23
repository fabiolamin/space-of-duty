#include "PlayerSpaceship.h"
#include <InputTriggers.h>
#include "EnhancedInputComponent.h" 
#include "Camera/CameraComponent.h" 

#include "GameFramework/SpringArmComponent.h"

APlayerSpaceship::APlayerSpaceship()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SpaceshipSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpaceshipSpringArm->SetupAttachment(RootComponent);

	SpaceshipSpringArm->bUsePawnControlRotation = false;
	SpaceshipSpringArm->bInheritPitch = true;
	SpaceshipSpringArm->bInheritYaw = true;
	SpaceshipSpringArm->bInheritRoll = false;

	SpaceshipSpringArm->bEnableCameraLag = true;

	SpaceshipCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SpaceshipCamera->SetupAttachment(SpaceshipSpringArm);
	SpaceshipCamera->bUsePawnControlRotation = false;

	SpaceshipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	SpaceshipMesh->SetupAttachment(RootComponent);

	DefaultSpaceshipRoll = SpaceshipMesh->GetRelativeRotation().Roll;
	CurrentSpaceshipSpeed = MinSpaceshipSpeed;
	TargetSpaceshipSpeed = MaxSpaceshipSpeed;

	DefaultCameraFOV = SpaceshipCamera->FieldOfView;
}

void APlayerSpaceship::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerSpaceship::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (MovementVector.Y < 0.0f)
	{
		MovementVector.Y = 0.0f;
	}
	else if (!IsBoosting && MovementVector.Y > 0.0f)
	{
		TimeSinceLastMoveInput = 0.0f;

		CurrentSpaceshipSpeed = FMath::FInterpTo(
			CurrentSpaceshipSpeed,
			MovementVector.Y * MaxSpaceshipSpeed,
			GetWorld()->GetDeltaSeconds(),
			SpaceshipMovementInterpSpeed);
	}

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, MovementVector.ToString());
}

void APlayerSpaceship::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>().GetSafeNormal();
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, LookVector.ToString());

	if (Controller != nullptr)
	{
		TimeSinceLastLookInput = 0.0f;

		float DeltaTime = GetWorld()->GetDeltaSeconds();

		FRotator NewRot = GetActorRotation();

		NewRot.Yaw += LookVector.X * (IsAiming ? YawRotationSpeed * 0.6f : YawRotationSpeed) * DeltaTime;
		NewRot.Pitch += LookVector.Y * (IsAiming ? PitchRotationSpeed * 0.6f : PitchRotationSpeed) * DeltaTime;

		NewRot.Pitch = FMath::Clamp(NewRot.Pitch, -MaxPitch, MaxPitch);

		SetActorRotation(NewRot);

		float TargetRoll = LookVector.X * MaxSpaceshipRoll;
		TargetRoll = FMath::Clamp(TargetRoll, -MaxSpaceshipRoll, MaxSpaceshipRoll);

		FRotator CurrentRotation = SpaceshipMesh->GetRelativeRotation();

		float NewRoll = FMath::FInterpTo(CurrentRotation.Roll, TargetRoll, DeltaTime, SpaceshipRollInterpSpeed);
		SpaceshipMesh->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, NewRoll));
	}
}

void APlayerSpaceship::StartBoost(const FInputActionValue& Value)
{
	IsBoosting = true;
	TargetSpaceshipSpeed = BoostSpaceshipSpeed;

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Purple, FString::Printf(TEXT("Boosting: %s"), IsBoosting ? TEXT("True") : TEXT("False")));
}

void APlayerSpaceship::StopBoost(const FInputActionValue& Value)
{
	IsBoosting = false;

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Purple, FString::Printf(TEXT("Boosting: %s"), IsBoosting ? TEXT("True") : TEXT("False")));
}

void APlayerSpaceship::StartAim(const FInputActionValue& Value)
{
	IsAiming = true;

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("Aiming: %s"), IsAiming ? TEXT("True") : TEXT("False")));
}

void APlayerSpaceship::StopAim(const FInputActionValue& Value)
{
	IsAiming = false;

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("Aiming: %s"), IsAiming ? TEXT("True") : TEXT("False")));
}

void APlayerSpaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, FString::Printf(TEXT("Current Speed: %.2f"), CurrentSpaceshipSpeed));
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString::Printf(TEXT("Current FOV: %.2f"), SpaceshipCamera->FieldOfView));

	AddActorWorldOffset(GetActorForwardVector() * DeltaTime * CurrentSpaceshipSpeed, true);

	TimeSinceLastLookInput += DeltaTime;
	TimeSinceLastMoveInput += DeltaTime;

	if (TimeSinceLastLookInput >= MaxTimeSinceLastLookInput)
	{
		FRotator CurrentRotation = SpaceshipMesh->GetRelativeRotation();
		float NewRoll = FMath::FInterpTo(CurrentRotation.Roll, DefaultSpaceshipRoll, DeltaTime, SpaceshipRollInterpSpeed);
		SpaceshipMesh->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, NewRoll));
	}

	if (!IsBoosting || TimeSinceLastMoveInput >= MaxTimeSinceLastMoveInput)
	{
		CurrentSpaceshipSpeed = FMath::FInterpTo(
			CurrentSpaceshipSpeed,
			MinSpaceshipSpeed,
			DeltaTime,
			SpaceshipMovementInterpSpeed);
	}

	if (IsBoosting)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (!PC) return;

		CurrentSpaceshipSpeed = FMath::FInterpTo(
			CurrentSpaceshipSpeed,
			BoostSpaceshipSpeed,
			DeltaTime,
			SpaceshipBoostInterpSpeed);

		SpaceshipCamera->SetFieldOfView(
			FMath::FInterpTo(
				SpaceshipCamera->FieldOfView,
				IsAiming ? 60.f : DefaultCameraFOV + BoostCameraDeltaFOV,
				DeltaTime,
				IsAiming ? SpaceshipBoostInterpSpeed * 2.f : SpaceshipBoostInterpSpeed
			)
		);

		float Intensity = CurrentSpaceshipSpeed / BoostSpaceshipSpeed;

		if (ActiveCameraShake || IsAiming)
		{
			PC->PlayerCameraManager->StopCameraShake(ActiveCameraShake);
			ActiveCameraShake = nullptr;
		}

		if (IsAiming) return;

		ActiveCameraShake =
			PC->PlayerCameraManager->StartCameraShake(
				SpeedCameraShake,
				Intensity
			);

		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Camera Shake Intensity: %.2f"), Intensity));
	}
	else
	{
		SpaceshipCamera->SetFieldOfView(
			FMath::FInterpTo(
				SpaceshipCamera->FieldOfView,
				IsAiming ? 60.f : DefaultCameraFOV,
				DeltaTime,
				IsAiming ? SpaceshipBoostInterpSpeed * 2.f : SpaceshipBoostInterpSpeed
			)
		);

		APlayerController* PC = Cast<APlayerController>(GetController());
		if (!PC || !ActiveCameraShake) return;

		PC->PlayerCameraManager->StopCameraShake(ActiveCameraShake);
		ActiveCameraShake = nullptr;
	}
}

void APlayerSpaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &APlayerSpaceship::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerSpaceship::Look);

		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Started, this, &APlayerSpaceship::StartBoost);
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Completed, this, &APlayerSpaceship::StopBoost);

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APlayerSpaceship::StartAim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APlayerSpaceship::StopAim);
	}
}

