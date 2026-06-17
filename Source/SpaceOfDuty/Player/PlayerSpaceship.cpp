#include "PlayerSpaceship.h"
#include <InputTriggers.h>
#include "EnhancedInputComponent.h" 
#include "Camera/CameraComponent.h" 
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

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

	MuzzleLeft = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLeft"));
	MuzzleLeft->SetupAttachment(SpaceshipMesh);

	MuzzleRight = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleRight"));
	MuzzleRight->SetupAttachment(SpaceshipMesh);
}

void APlayerSpaceship::BeginPlay()
{
	Super::BeginPlay();

	DefaultSpaceshipRoll = SpaceshipMesh->GetRelativeRotation().Roll;
	DefaultCameraFOV = SpaceshipCamera->FieldOfView;

	CurrentSpaceshipSpeed = MinSpaceshipSpeed;

	TargetSpaceshipRoll = DefaultSpaceshipRoll;
	TargetSpaceshipFOV = DefaultCameraFOV;
	TargetSpaceshipSpeed = MaxSpaceshipSpeed;
	TargetSpaceshipFOVInterpSpeed = SpaceshipMovementInterpSpeed;

	TimeSinceLastShot = ShootingInterval;
}

TArray<USceneComponent*> APlayerSpaceship::GetMuzzleComponents() const
{
	TArray<USceneComponent*> Muzzles;
	if (MuzzleLeft) Muzzles.Add(MuzzleLeft);
	if (MuzzleRight) Muzzles.Add(MuzzleRight);
	return Muzzles;
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
	const float BaseSensitivity = 0.095f;
	const float AccelerationStrength = 0.087f;
	const float MaxSensitivity = 0.12f;
	const float AccelPower = 1.2f;

	const FVector2D LookVector = Value.Get<FVector2D>();

	float MouseSpeed = LookVector.Size();
	float Acceleration = FMath::Pow(MouseSpeed, AccelPower) * AccelerationStrength;

	float SensitivityMultiplier = FMath::Clamp(Acceleration, BaseSensitivity, MaxSensitivity);

	const FVector2D NewLookVector = LookVector * SensitivityMultiplier;

	if (Controller != nullptr)
	{
		TimeSinceLastLookInput = 0.0f;

		float DeltaTime = GetWorld()->GetDeltaSeconds();

		FRotator NewRot = GetActorRotation();

		NewRot.Yaw += NewLookVector.X;
		NewRot.Pitch += NewLookVector.Y;

		NewRot.Pitch = FMath::Clamp(NewRot.Pitch, -MaxPitch, MaxPitch);

		SetActorRotation(NewRot);

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
}

void APlayerSpaceship::StartBoost(const FInputActionValue& Value)
{
	IsBoosting = true;
	IsAiming = false;

	TargetSpaceshipSpeed = BoostSpaceshipSpeed;
	TargetSpaceshipFOV = DefaultCameraFOV + BoostCameraDeltaFOV;
	TargetSpaceshipFOVInterpSpeed = SpaceshipBoostInterpSpeed;

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Purple, FString::Printf(TEXT("Boosting: %s"), IsBoosting ? TEXT("True") : TEXT("False")));
}

void APlayerSpaceship::StopBoost(const FInputActionValue& Value)
{
	IsBoosting = false;

	TargetSpaceshipSpeed = MaxSpaceshipSpeed;
	TargetSpaceshipFOV = DefaultCameraFOV;
	TargetSpaceshipFOVInterpSpeed = SpaceshipBoostInterpSpeed;

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Purple, FString::Printf(TEXT("Boosting: %s"), IsBoosting ? TEXT("True") : TEXT("False")));
}

void APlayerSpaceship::StartAim(const FInputActionValue& Value)
{
	if (IsBoosting) return;

	IsAiming = true;

	TargetSpaceshipFOV = SpaceshipAimCameraFOV;
	TargetSpaceshipFOVInterpSpeed = SpaceshipFOVAimInterpSpeed;

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("Aiming: %s"), IsAiming ? TEXT("True") : TEXT("False")));
}

void APlayerSpaceship::StopAim(const FInputActionValue& Value)
{
	if (!IsAiming) return;

	IsAiming = false;

	TargetSpaceshipFOV = DefaultCameraFOV;
	TargetSpaceshipFOVInterpSpeed = SpaceshipFOVAimInterpSpeed;

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("Aiming: %s"), IsAiming ? TEXT("True") : TEXT("False")));
}

void APlayerSpaceship::StartShoot(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Magenta, TEXT("Start Shooting"));

	IsShooting = true;
}

void APlayerSpaceship::StopShoot(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Magenta, TEXT("Stop Shooting"));

	IsShooting = false;
}

void APlayerSpaceship::CheckShooting(float DeltaTime)
{
	if (IsShooting)
	{
		TimeSinceLastShot += DeltaTime;

		if (TimeSinceLastShot >= ShootingInterval)
		{
			Shoot();

			TimeSinceLastShot = 0.0f;
		}
	}
	else
	{
		if (TimeSinceLastShot != ShootingInterval)
		{
			TimeSinceLastShot = ShootingInterval;
		}
	}
}

void APlayerSpaceship::Shoot()
{
	TArray<USceneComponent*> Muzzles = GetMuzzleComponents();

	if (!ProjectileClass || Muzzles.Num() == 0) return;

	FVector CameraLocation = SpaceshipCamera->GetComponentLocation();
	FVector CameraForward = SpaceshipCamera->GetForwardVector();

	float TraceDistance = 100000.f;

	FVector TraceEnd = CameraLocation + (CameraForward * TraceDistance);

	FHitResult HitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, QueryParams);

	FVector TargetPoint = bHit ? HitResult.ImpactPoint : TraceEnd;

	for (USceneComponent* Muzzle : Muzzles)
	{
		if (!Muzzle) continue;

		FVector ToMuzzle = (Muzzle->GetComponentLocation() - GetActorLocation()).GetSafeNormal();
		float Dot = FVector::DotProduct(ToMuzzle, GetActorRightVector());
		bool IsRight = Dot > 0.1f; 

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = GetInstigator();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector SpawnLocation = Muzzle->GetComponentLocation();
		FVector LocalMuzzleLocation = Muzzle->GetRelativeLocation();

		float MuzzleOffset = 150;
		FVector MuzzleOffsetVector = GetActorRightVector() * MuzzleOffset;
		FVector MuzzleTargetPoint = TargetPoint + (IsRight ? MuzzleOffsetVector : -MuzzleOffsetVector);

		FVector ShootDirection = (MuzzleTargetPoint - SpawnLocation).GetSafeNormal();
		FRotator SpawnRotation = ShootDirection.Rotation();

		ASpaceshipProjectile* Projectile = GetWorld()->SpawnActor<ASpaceshipProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params);

		if (Projectile)
		{
			Projectile->InitProjectile(ShootDirection);
		}

		//DrawDebugLine(GetWorld(), CameraLocation, TargetPoint, FColor::Red, false, 2.f);
		//DrawDebugSphere(GetWorld(), MuzzleTargetPoint, 100.f, 12, FColor::Green, false, 5.f);

		//DrawDebugLine(
		//	GetWorld(),
		//	SpawnLocation,
		//	SpawnLocation + ShootDirection * 5000.f,
		//	FColor::Blue,
		//	false,
		//	10.f,
		//	0,
		//	5.f
		//);
	}
}

void APlayerSpaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, FString::Printf(TEXT("Current Speed: %.2f"), CurrentSpaceshipSpeed));
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString::Printf(TEXT("Current FOV: %.2f"), SpaceshipCamera->FieldOfView));

	AddActorWorldOffset(GetActorForwardVector() * DeltaTime * CurrentSpaceshipSpeed, true);

	if (!IsBoosting && TimeSinceLastMoveInput >= MaxTimeSinceLastMoveInput)
	{
		CurrentSpaceshipSpeed = FMath::FInterpTo(
			CurrentSpaceshipSpeed,
			MinSpaceshipSpeed,
			DeltaTime,
			SpaceshipMovementInterpSpeed);
	}

	CheckShooting(DeltaTime);
	CheckSpaceshipBoosting(DeltaTime);

	UpdateSpaceshipRoll(DeltaTime);
	UpdateSpaceshipCameraFOV(DeltaTime);

	TimeSinceLastLookInput += DeltaTime;
	TimeSinceLastMoveInput += DeltaTime;

	if (TimeSinceLastLookInput >= MaxTimeSinceLastLookInput)
	{
		TargetSpaceshipRoll = DefaultSpaceshipRoll;
	}
}

void APlayerSpaceship::UpdateSpaceshipCameraFOV(float DeltaTime)
{
	if (!FMath::IsNearlyEqual(SpaceshipCamera->FieldOfView, TargetSpaceshipFOV, 0.1f))
	{
		float NewFOV = FMath::FInterpTo(SpaceshipCamera->FieldOfView, TargetSpaceshipFOV, DeltaTime, TargetSpaceshipFOVInterpSpeed);

		SpaceshipCamera->SetFieldOfView(NewFOV);
	}
}

void APlayerSpaceship::CheckSpaceshipBoosting(float DeltaTime)
{
	if (IsBoosting)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());

		if (PC)
		{
			CurrentSpaceshipSpeed = FMath::FInterpTo(
				CurrentSpaceshipSpeed,
				BoostSpaceshipSpeed,
				DeltaTime,
				SpaceshipBoostInterpSpeed);

			float Intensity = CurrentSpaceshipSpeed / BoostSpaceshipSpeed;

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
	}
	else
	{
		APlayerController* PC = Cast<APlayerController>(GetController());

		if (PC && ActiveCameraShake)
		{
			PC->PlayerCameraManager->StopCameraShake(ActiveCameraShake);
			ActiveCameraShake = nullptr;
		}
	}
}

void APlayerSpaceship::UpdateSpaceshipRoll(float DeltaTime)
{
	FRotator CurrentRotation = SpaceshipMesh->GetRelativeRotation();

	if (!FMath::IsNearlyEqual(CurrentRotation.Roll, TargetSpaceshipRoll, 0.1f))
	{
		float NewRoll = FMath::FInterpTo(CurrentRotation.Roll, TargetSpaceshipRoll, DeltaTime, SpaceshipRollInterpSpeed);
		SpaceshipMesh->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, NewRoll));
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

		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &APlayerSpaceship::StartShoot);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &APlayerSpaceship::StopShoot);
	}
}
