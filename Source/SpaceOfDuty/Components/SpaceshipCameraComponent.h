#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpaceshipCameraComponent.generated.h"

class UCameraComponent;
class UCameraShakeBase;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPACEOFDUTY_API USpaceshipCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpaceshipCameraComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Initialize(UCameraComponent* InCamera);

	void StartBoostEffects(float InterpSpeed);
	void StopBoostEffects(float InterpSpeed);

	void UpdateBoostShake(bool bIsBoosting, float ShakeScale);
	void StopBoostShake();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> SpeedCameraShake;

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BoostCameraDeltaFOV = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Shake", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> BoostKickCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Shake", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float SpeedShakeIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Shake", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BoostKickShakeIntensity = 1.0f;

	UPROPERTY()
	UCameraComponent* Camera = nullptr;

	UPROPERTY()
	UCameraShakeBase* ActiveCameraShake = nullptr;

	float DefaultCameraFOV = 90.0f;
	float TargetFOV = 90.0f;
	float TargetFOVInterpSpeed = 0.0f;

	void UpdateCameraFOV(float DeltaTime);
};
