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
	CurrentSpaceshipSpeed = MinSpaceshipSpeed;
	TargetSpaceshipSpeed = MaxSpaceshipSpeed;

	DefaultCameraFOV = SpaceshipCamera->FieldOfView;

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

	float TraceDistance = 10000.f;

	FVector TraceEnd = CameraLocation + (CameraForward * TraceDistance);

	for (USceneComponent* Muzzle : Muzzles)
	{
		if (!Muzzle) continue;

		FHitResult HitResult;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, QueryParams);

		FVector SpawnLocation = Muzzle->GetComponentLocation();
		FVector LocalMuzzleLocation = Muzzle->GetRelativeLocation();

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString::Printf(TEXT("Spawn Location: %s"), *LocalMuzzleLocation.ToString()));

		FVector TargetPoint = bHit ? HitResult.ImpactPoint : TraceEnd;

		FVector ShootDirection = (TargetPoint - SpawnLocation).GetSafeNormal();
		FRotator SpawnRotation = ShootDirection.Rotation();

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = GetInstigator();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ASpaceshipProjectile* Projectile = GetWorld()->SpawnActor<ASpaceshipProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params);

		const float MaxProjectileMultiplier = 2.f;

		float SpeedRatio = FMath::Clamp(CurrentSpaceshipSpeed / MaxSpaceshipSpeed, 0.f, 1.f);

		float FinalProjectileSpeed = FMath::Lerp(BaseProjectileSpeed, BaseProjectileSpeed * MaxProjectileMultiplier, SpeedRatio);

		if (Projectile)
		{
			Projectile->InitProjectile(ShootDirection, FinalProjectileSpeed);
		}

		DrawDebugLine(GetWorld(), CameraLocation, TargetPoint, FColor::Red, false, 2.f);
		DrawDebugSphere(GetWorld(), TargetPoint, 10.f, 12, FColor::Green, false, 2.f);
	}
}

void APlayerSpaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, FString::Printf(TEXT("Current Speed: %.2f"), CurrentSpaceshipSpeed));
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString::Printf(TEXT("Current FOV: %.2f"), SpaceshipCamera->FieldOfView));

	AddActorWorldOffset(GetActorForwardVector() * DeltaTime * CurrentSpaceshipSpeed, true);

	TimeSinceLastLookInput += DeltaTime;
	TimeSinceLastMoveInput += DeltaTime;

	CheckShooting(DeltaTime);

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

		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &APlayerSpaceship::StartShoot);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &APlayerSpaceship::StopShoot);
	}
}
