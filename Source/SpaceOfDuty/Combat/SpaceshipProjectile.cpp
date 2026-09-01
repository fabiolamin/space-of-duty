#include "Combat/SpaceshipProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/Engine.h"
#include "Tools/PoolManagerComponent.h"

ASpaceshipProjectile::ASpaceshipProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(Cast<USceneComponent>(SphereComponent));

	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Block);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	SphereComponent->OnComponentHit.AddDynamic(this, &ASpaceshipProjectile::OnProjectileHit);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(SphereComponent);

	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetGenerateOverlapEvents(false);
	ProjectileMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = SphereComponent;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bForceSubStepping = true;	
}

void ASpaceshipProjectile::BeginPlay()
{
	Super::BeginPlay();

	SphereComponent->IgnoreActorWhenMoving(GetOwner(), true);
}

void ASpaceshipProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ProjectilePool)
	{
		ProjectilePool->EnablePooledActor(this, false);

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("Projectile hit: %s"), *OtherActor->GetName()));
	}
}

void ASpaceshipProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckLifeSpan(DeltaTime);
}

void ASpaceshipProjectile::CheckLifeSpan(float DeltaTime)
{
	CurrentLifeSpan += DeltaTime;

	if (CurrentLifeSpan >= LifeSpan)
	{
		if (ProjectilePool)
		{
			ProjectilePool->EnablePooledActor(this, false);
		}
	}
}

void ASpaceshipProjectile::FireProjectileInDirection(const FVector& Direction, UPoolManagerComponent* InProjectilePool)
{
	FireProjectileInDirection(Direction, ProjectileSpeed, InProjectilePool);
}

void ASpaceshipProjectile::FireProjectileInDirection(const FVector& Direction, float Speed, UPoolManagerComponent* InProjectilePool)
{
	SetProjectilePool(InProjectilePool);

	if (ProjectileMovement)
	{
		CurrentLifeSpan = 0.f;

		ProjectileMovement->Velocity = Direction.GetSafeNormal() * Speed;
	}
}

void ASpaceshipProjectile::SetProjectilePool(UPoolManagerComponent* InProjectilePool)
{
	if (ProjectilePool == nullptr && InProjectilePool != nullptr)
	{
		ProjectilePool = InProjectilePool;
	}
}
