#include "Player/PlayerSpaceship.h"
#include "Components/SpaceshipMovementComponent.h"
#include "Components/SpaceshipWeaponComponent.h"
#include "Components/SpaceshipTargetingComponent.h"
#include "Components/SpaceshipCameraComponent.h"
#include <InputTriggers.h>
#include "EnhancedInputComponent.h" 
#include "Camera/CameraComponent.h" 
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Combat/MissileSpaceshipProjectile.h"

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

	MuzzleCenter = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleCenter"));
	MuzzleCenter->SetupAttachment(SpaceshipMesh);

	BulletPool = CreateDefaultSubobject<UPoolManagerComponent>(TEXT("BulletPool"));
	MissilePool = CreateDefaultSubobject<UPoolManagerComponent>(TEXT("MissilePool"));

	SpaceshipMovementComponent = CreateDefaultSubobject<USpaceshipMovementComponent>(TEXT("SpaceshipMovementComponent"));
	SpaceshipWeaponComponent = CreateDefaultSubobject<USpaceshipWeaponComponent>(TEXT("SpaceshipWeaponComponent"));
	SpaceshipTargetingComponent = CreateDefaultSubobject<USpaceshipTargetingComponent>(TEXT("SpaceshipTargetingComponent"));
	SpaceshipCameraComponent = CreateDefaultSubobject<USpaceshipCameraComponent>(TEXT("SpaceshipCameraComponent"));
}

void APlayerSpaceship::BeginPlay()
{
	Super::BeginPlay();

	if (SpaceshipMovementComponent)
	{
		SpaceshipMovementComponent->Initialize(SpaceshipMesh);
		SpaceshipMovementComponent->OnBoostStopped.AddUniqueDynamic(this, &APlayerSpaceship::ResetSpaceshipBoost);
	}

	if (SpaceshipWeaponComponent && SpaceshipTargetingComponent)
	{
		SpaceshipWeaponComponent->Initialize(SpaceshipCamera, MuzzleLeft, MuzzleRight, MuzzleCenter, BulletPool, MissilePool, SpaceshipMovementComponent, SpaceshipTargetingComponent);
	}

	if (SpaceshipCameraComponent)
	{
		SpaceshipCameraComponent->Initialize(SpaceshipCamera);
	}
}

void APlayerSpaceship::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetActorTickEnabled(false);

	if (SpaceshipMovementComponent)
	{
		SpaceshipMovementComponent->OnBoostStopped.RemoveDynamic(this, &APlayerSpaceship::ResetSpaceshipBoost);
	}

	Super::EndPlay(EndPlayReason);
}

void APlayerSpaceship::Move(const FInputActionValue& Value)
{
	if (SpaceshipMovementComponent)
	{
		SpaceshipMovementComponent->Accelerate(Value.Get<FVector2D>());
	}
}

void APlayerSpaceship::Look(const FInputActionValue& Value)
{
	if (SpaceshipMovementComponent)
	{
		SpaceshipMovementComponent->Look(Value.Get<FVector2D>());
	}
}

void APlayerSpaceship::StartBoost(const FInputActionValue& Value)
{
	if( !SpaceshipMovementComponent || SpaceshipMovementComponent->GetBoostAmount() <= 0.f || !SpaceshipCameraComponent)
	{
		return;
	}

	SpaceshipCameraComponent->StartBoostEffects(SpaceshipMovementComponent->GetSpaceshipBoostInterpSpeed());
	SpaceshipMovementComponent->StartBoost();
}

void APlayerSpaceship::StopBoost(const FInputActionValue& Value)
{
	if(SpaceshipMovementComponent)
	{
		SpaceshipMovementComponent->StopBoost();
	}

	ResetSpaceshipBoost();
}

void APlayerSpaceship::ResetSpaceshipBoost()
{
	if (!SpaceshipMovementComponent || IsActorBeingDestroyed())
	{
		return;
	}

	if (SpaceshipCameraComponent)
	{
		SpaceshipCameraComponent->StopBoostEffects(SpaceshipMovementComponent->GetSpaceshipBoostInterpSpeed());
	}
}

void APlayerSpaceship::StartChargingMissile(const FInputActionValue& Value)
{
	if (SpaceshipWeaponComponent)
	{
		SpaceshipWeaponComponent->StartChargingMissile();
	}
}

void APlayerSpaceship::StopChargingMissile(const FInputActionValue& Value)
{
	if (SpaceshipWeaponComponent)
	{
		SpaceshipWeaponComponent->StopChargingMissile();
	}
}

void APlayerSpaceship::StartShoot(const FInputActionValue& Value)
{
	if (SpaceshipWeaponComponent)
	{
		SpaceshipWeaponComponent->StartShoot();
	}
}

void APlayerSpaceship::StopShoot(const FInputActionValue& Value)
{
	if (SpaceshipWeaponComponent)
	{
		SpaceshipWeaponComponent->StopShoot();
	}
}

void APlayerSpaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsActorBeingDestroyed())
	{
		return;
	}

	if(SpaceshipMovementComponent)
	{
		SpaceshipCameraComponent->UpdateBoostShake(SpaceshipMovementComponent->IsSpaceshipBoosting(), SpaceshipMovementComponent->GetCurrentBoostSpeedRatio());
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

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APlayerSpaceship::StartChargingMissile);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APlayerSpaceship::StopChargingMissile);

		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &APlayerSpaceship::StartShoot);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &APlayerSpaceship::StopShoot);
	}
}
