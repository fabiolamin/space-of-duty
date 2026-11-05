#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpaceshipHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthDamagedSignature, float, DamagePercentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthHealedSignature, float, HealPercentage);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPACEOFDUTY_API USpaceshipHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USpaceshipHealthComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void TakeDamage(float DamageAmount);
	void Heal(float HealAmount);

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthDamagedSignature OnTakeDamage;
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthHealedSignature OnHeal;


protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth = 100.0f;
	UPROPERTY(VisibleAnywhere, Category = "Health")
	float CurrentHealth = 100.0f;
};
