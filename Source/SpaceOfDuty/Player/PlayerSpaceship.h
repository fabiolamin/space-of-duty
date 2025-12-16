#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerSpaceship.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class SPACEOFDUTY_API APlayerSpaceship : public APawn
{
	GENERATED_BODY()

public:
	APlayerSpaceship();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;


private:
	
	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAcess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAcess = "true"))
	class UInputAction* ForwardAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAcess = "true"))
	class UInputAction* LookAction;

	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
};
