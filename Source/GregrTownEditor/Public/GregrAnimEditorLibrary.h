// GregrTown - editor tools

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GregrAnimEditorLibrary.generated.h"

class UAnimSequence;
class UBlendSpace;

/** Animation asset helpers for scripting (Python / Editor Utility) what the asset editors do by hand. */
UCLASS()
class GREGRTOWNEDITOR_API UGregrAnimEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Replaces all samples of a blend space and rebuilds its runtime data,
	 * like dragging animations into the blend space editor.
	 * SampleValues[i] is the position of Animations[i] (X for 1D blend spaces).
	 */
	UFUNCTION(BlueprintCallable, Category = "GregrTown|Animation")
	static bool SetBlendSpaceSamples(UBlendSpace* BlendSpace, const TArray<UAnimSequence*>& Animations, const TArray<FVector>& SampleValues);
};
