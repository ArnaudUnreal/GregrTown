// GregrTown - character

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FallRecoveryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFallRecovered, FVector, RecoveredLocation);

/**
 * Safety net for a character: remembers the last safe ground position and puts the character
 * back there if it ends up under the terrain or falls for too long (fell through the world,
 * pushed by debris, off the map...).
 */
UCLASS(ClassGroup = (Gregr), meta = (BlueprintSpawnableComponent))
class GREGRTOWN_API UFallRecoveryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFallRecoveryComponent();

	/** Seconds of continuous falling before the character is brought back. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall Recovery", meta = (ClampMin = "0.5"))
	float MaxFallTime = 4.f;

	/** Brought back when this far (cm) below the last safe position. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall Recovery", meta = (ClampMin = "100"))
	float MaxDropBelowSafe = 3000.f;

	UFUNCTION(BlueprintCallable, Category = "Fall Recovery")
	void RecoverNow();

	UFUNCTION(BlueprintPure, Category = "Fall Recovery")
	FVector GetLastSafeLocation() const { return LastSafeLocation; }

	UFUNCTION(BlueprintPure, Category = "Fall Recovery")
	int32 GetRecoveryCount() const { return RecoveryCount; }

	UPROPERTY(BlueprintAssignable, Category = "Fall Recovery")
	FOnFallRecovered OnRecovered;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** True when the terrain is above the character's feet. */
	bool IsUnderTerrain() const;

	FVector LastSafeLocation = FVector::ZeroVector;
	float FallingTime = 0.f;
	int32 RecoveryCount = 0;
};
