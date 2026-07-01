#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PoolManagerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPACEOFDUTY_API UPoolManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPoolManagerComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	AActor* GetPooledActor();
	void EnablePooledActor(AActor* Actor, bool bEnable);

protected:

	virtual void BeginPlay() override;

private:	
		
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	int PoolSize = 25;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> PooledActorTemplate;

	TArray<AActor*> PooledActors;

	void InitializePool();
	AActor* SpawnPooledActor();
};
