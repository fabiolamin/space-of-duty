#include "Tools/PoolManagerComponent.h"

UPoolManagerComponent::UPoolManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UPoolManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializePool();
}

void UPoolManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPoolManagerComponent::InitializePool()
{
	for (size_t i = 0; i < PoolSize; i++)
	{
		SpawnPooledActor();
	}
}

AActor* UPoolManagerComponent::GetPooledActor()
{
	for (AActor* Actor : PooledActors)
	{
		if (Actor && Actor->IsHidden())
		{
			Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

			EnablePooledActor(Actor, true);

			return Actor;
		}
	}

	AActor* NewActor = SpawnPooledActor();

	NewActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	EnablePooledActor(NewActor, true);

	return NewActor;
}

AActor* UPoolManagerComponent::SpawnPooledActor()
{
	AActor* Owner = GetOwner();

	if (!Owner) return nullptr;

	FActorSpawnParameters Params;
	Params.Owner = Owner;
	Params.Instigator = Owner->GetInstigator();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = GetWorld()->SpawnActor<AActor>(PooledActorTemplate, Owner->GetActorLocation(), Owner->GetActorRotation(), Params);

	if (NewActor)
	{
		NewActor->AttachToActor(Owner, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		EnablePooledActor(NewActor, false);

		PooledActors.Add(NewActor);

		return NewActor;
	}

	return nullptr;
}

void UPoolManagerComponent::EnablePooledActor(AActor* Actor, bool bEnable)
{
	if (Actor)
	{
		Actor->SetActorHiddenInGame(!bEnable);
		Actor->SetActorEnableCollision(bEnable);
		Actor->SetActorTickEnabled(bEnable);

		TArray<UActorComponent*> AllComponents;
		Actor->GetComponents(AllComponents);

		for (UActorComponent* Comp : AllComponents)
		{
			if (!Comp) continue;

			if (bEnable)
				Comp->Activate();
			else
				Comp->Deactivate();

			Comp->SetComponentTickEnabled(bEnable);

			if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Comp))
			{
				PrimitiveComp->SetVisibility(bEnable);
				PrimitiveComp->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
			}
		}
	}
}
