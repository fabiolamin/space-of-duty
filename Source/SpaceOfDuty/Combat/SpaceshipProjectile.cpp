#include "Combat/SpaceshipProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/Engine.h"


ASpaceshipProjectile::ASpaceshipProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(Cast<USceneComponent>(SphereComponent));

	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(SphereComponent);

	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetGenerateOverlapEvents(false);
	ProjectileMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
}

void ASpaceshipProjectile::BeginPlay()
{
	Super::BeginPlay();

	SphereComponent->IgnoreActorWhenMoving(GetOwner(), true);
}

void ASpaceshipProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AddActorWorldOffset(ProjectileDirection * ProjectileSpeed * DeltaTime, true);
}

void ASpaceshipProjectile::InitProjectile(const FVector& Direction, float Speed)
{
	ProjectileDirection = Direction.GetSafeNormal();
	ProjectileSpeed = Speed;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Projectile Initialized with Direction: %s and Speed: %.2f"), *ProjectileDirection.ToString(), ProjectileSpeed));
	}
}
