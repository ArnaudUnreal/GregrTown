// GregrTown - destruction framework

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Destruction/DestructionTypes.h"
#include "BreakableInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UBreakableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by anything that can be hit by a destructive impact.
 * Hitters (hammer, explosions...) only know about this interface, never about concrete classes,
 * so new breakable things (NPCs, cars, fences...) can be added without touching the hitters.
 */
class GREGRTOWN_API IBreakableInterface
{
	GENERATED_BODY()

public:
	/** Apply a destructive impact to this object. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Destruction")
	void ReceiveDestructionImpact(const FDestructionImpact& Impact);

	/** Whether this object currently accepts the given impact. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Destruction")
	bool CanReceiveDestructionImpact(const FDestructionImpact& Impact) const;

	/** Whether this object has already been broken at least once. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Destruction")
	bool IsBroken() const;
};
