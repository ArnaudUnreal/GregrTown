// GregrTown - destruction framework

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Destruction/DestructionTypes.h"
#include "DestructionStatics.generated.h"

UCLASS()
class GREGRTOWN_API UDestructionStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Sends Impact to every destructible overlapping a sphere of Impact.Radius around Impact.Location.
	 * This is what the hammer (or anything else) calls; it only relies on IBreakableInterface.
	 * @return number of actors that received the impact
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction", meta = (WorldContext = "WorldContextObject"))
	static int32 ApplyDestructionImpactInSphere(const UObject* WorldContextObject, const FDestructionImpact& Impact, TArray<AActor*>& OutHitActors, bool bDrawDebug = false);

	/** Sends Impact to a single actor if it implements IBreakableInterface and accepts it. */
	UFUNCTION(BlueprintCallable, Category = "Destruction")
	static bool ApplyDestructionImpactToActor(AActor* Target, const FDestructionImpact& Impact);
};
