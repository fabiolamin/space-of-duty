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

	//TODO: Enable when the turbo is implemented
	//SpaceshipSpringArm->bEnableCameraLag = true;
	//SpaceshipSpringArm->CameraLagSpeed = 8.f;

	SpaceshipCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SpaceshipCamera->SetupAttachment(SpaceshipSpringArm);
	SpaceshipCamera->bUsePawnControlRotation = false;

	SpaceshipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	SpaceshipMesh->SetupAttachment(RootComponent);

	DefaultSpaceshipRoll = SpaceshipMesh->GetRelativeRotation().Roll;
	CurrentSpaceshipSpeed = MinSpaceshipSpeed;
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
	else if (MovementVector.Y > 0.0f)
	{
		TimeSinceLastMoveInput = 0.0f;

		CurrentSpaceshipSpeed = FMath::FInterpTo(
			CurrentSpaceshipSpeed,
			MovementVector.Y * MaxSpaceshipSpeed,
			GetWorld()->GetDeltaSeconds(),
			SpaceshipMovementInterpSpeed);
	}

	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, MovementVector.ToString());
}

void APlayerSpaceship::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>().GetSafeNormal() * GetWorld()->GetDeltaSeconds();
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, LookVector.ToString());

	if (Controller != nullptr)
	{
		TimeSinceLastLookInput = 0.0f;

		FRotator NewRot = GetActorRotation();

		NewRot.Yaw += LookVector.X * YawRotationSpeed;
		NewRot.Pitch += LookVector.Y * PitchRotationSpeed;

		NewRot.Pitch = FMath::Clamp(NewRot.Pitch, -80.f, 80.f);

		SetActorRotation(NewRot);

		float TargetRoll = LookVector.X * SpaceshipRollSpeed * YawRotationSpeed;
		TargetRoll = FMath::Clamp(TargetRoll, -MaxSpaceshipRoll, MaxSpaceshipRoll);

		FRotator CurrentRotation = SpaceshipMesh->GetRelativeRotation();

		float NewRoll = FMath::FInterpTo(CurrentRotation.Roll, TargetRoll, GetWorld()->GetDeltaSeconds(), SpaceshipRollInterpSpeed);
		SpaceshipMesh->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, NewRoll));
	}
}

void APlayerSpaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, FString::Printf(TEXT("Current Speed: %.2f"), CurrentSpaceshipSpeed));

	AddActorWorldOffset(GetActorForwardVector() * DeltaTime * CurrentSpaceshipSpeed, true);

	TimeSinceLastLookInput += DeltaTime;
	TimeSinceLastMoveInput += DeltaTime;

	if (TimeSinceLastLookInput >= MaxTimeSinceLastLookInput)
	{
		FRotator CurrentRotation = SpaceshipMesh->GetRelativeRotation();
		float NewRoll = FMath::FInterpTo(CurrentRotation.Roll, DefaultSpaceshipRoll, DeltaTime, SpaceshipRollInterpSpeed);
		SpaceshipMesh->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, NewRoll));
	}

	if (TimeSinceLastMoveInput >= MaxTimeSinceLastMoveInput)
	{
		CurrentSpaceshipSpeed = FMath::FInterpTo(
			CurrentSpaceshipSpeed,
			MinSpaceshipSpeed,
			DeltaTime,
			SpaceshipMovementInterpSpeed);
	}

	if (CurrentSpaceshipSpeed > 1000)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (!PC) return;

		float Intensity = CurrentSpaceshipSpeed / MaxSpaceshipSpeed;

		// Sempre reinicia com nova intensidade
		if (ActiveCameraShake)
		{
			PC->PlayerCameraManager->StopCameraShake(ActiveCameraShake);
			ActiveCameraShake = nullptr;
		}

		ActiveCameraShake =
			PC->PlayerCameraManager->StartCameraShake(
				SpeedCameraShake,
				Intensity
			);

		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Camera Shake Intensity: %.2f"), Intensity));
	}
	else
	{
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
		EnhancedInputComponent->BindAction(ForwardAction, ETriggerEvent::Triggered, this, &APlayerSpaceship::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerSpaceship::Look);
	}
}

