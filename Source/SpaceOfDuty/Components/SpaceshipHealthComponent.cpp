#include "Components/SpaceshipHealthComponent.h"

USpaceshipHealthComponent::USpaceshipHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USpaceshipHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USpaceshipHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USpaceshipHealthComponent::TakeDamage(float DamageAmount)
{
	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	OnTakeDamage.Broadcast(MaxHealth > 0.0f ? FMath::Clamp((PreviousHealth - CurrentHealth) / MaxHealth, 0.0f, 1.0f) : 0.0f);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Spaceship took damage: %f"), DamageAmount));
}

void USpaceshipHealthComponent::Heal(float HealAmount)
{
	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
	OnHeal.Broadcast(MaxHealth > 0.0f ? FMath::Clamp((CurrentHealth - PreviousHealth) / MaxHealth, 0.0f, 1.0f) : 0.0f);
}
