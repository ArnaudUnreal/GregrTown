// GregrTown - destruction framework

#pragma once

#include "CoreMinimal.h"
#include "DestructionTypes.generated.h"

GREGRTOWN_API DECLARE_LOG_CATEGORY_EXTERN(LogGregrDestruction, Log, All);

/**
 * Describes a single destructive hit (hammer blow, explosion, projectile...).
 * Anything able to break things fills one of these and sends it through IBreakableInterface.
 * The receiver decides how it reacts using its own UDestructionProfile.
 */
USTRUCT(BlueprintType)
struct GREGRTOWN_API FDestructionImpact
{
	GENERATED_BODY()

	/** World location of the impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	FVector Location = FVector::ZeroVector;

	/** Direction of the blow (normalized). Zero means "push outwards from Location". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	FVector Direction = FVector::ZeroVector;

	/** Radius of the area affected by the impact, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0"))
	float Radius = 150.f;

	/** Strength multiplier. 1 = a regular hammer hit. Scales strain and impulse of the receiver profile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0"))
	float Strength = 1.f;

	/** Who caused the impact (character, player controller...). Used for scoring. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	TObjectPtr<AActor> Instigator = nullptr;

	/** What caused the impact (hammer actor, projectile...). Optional. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	TObjectPtr<UObject> Causer = nullptr;

	/** Free-form type of impact (Hammer, Explosion...) so receivers can react differently later. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	FName ImpactType = NAME_None;
};
