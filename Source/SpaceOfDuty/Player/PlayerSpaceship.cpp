#include "PlayerSpaceship.h"
#include <InputTriggers.h>
#include "EnhancedInputComponent.h" 
#include "Camera/CameraComponent.h" 

APlayerSpaceship::APlayerSpaceship()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SpaceshipCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SpaceshipCamera->SetupAttachment(RootComponent);
	SpaceshipCamera->bUsePawnControlRotation = false;

	SpaceshipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	SpaceshipMesh->SetupAttachment(RootComponent);

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = false;

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
	else if(MovementVector.Y > 0.0f)
	{
		TimeSinceLastMoveInput = 0.0f;

		CurrentSpaceshipSpeed = FMath::FInterpTo(
			CurrentSpaceshipSpeed, 
			MovementVector.Y * 2000.0f, 
			GetWorld()->GetDeltaSeconds(), 
			SpaceshipMovementInterpSpeed);
	}

	//FVector Forward = GetActorForwardVector() * MovementVector.Y;
	//FVector DeltaLocation = Forward.GetSafeNormal() * 1000 * GetWorld()->GetDeltaSeconds();
	//AddActorWorldOffset(DeltaLocation, true);


	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, MovementVector.ToString());
}

void APlayerSpaceship::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>().GetSafeNormal() * GetWorld()->GetDeltaSeconds();
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, LookVector.ToString());

	if (Controller != nullptr)
	{
		TimeSinceLastLookInput = 0.0f;

		AddControllerPitchInput(-LookVector.Y * PitchRotationSpeed);
		AddControllerYawInput(LookVector.X * YawRotationSpeed);

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

