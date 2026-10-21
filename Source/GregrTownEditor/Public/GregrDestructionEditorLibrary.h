// GregrTown - editor tools

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GregrDestructionEditorLibrary.generated.h"

class UGeometryCollection;

/** Batch edits of Geometry Collection assets (what Fracture Mode does by hand). */
UCLASS()
class GREGRTOWNEDITOR_API UGregrDestructionEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Enables "Remove On Break" on every piece: once broken off, a piece waits BreakTimer seconds
	 * then shrinks away during RemovalTimer seconds (random between X and Y). Marks the asset dirty.
	 */
	UFUNCTION(BlueprintCallable, Category = "GregrTown|Destruction")
	static bool SetRemoveOnBreak(UGeometryCollection* GeometryCollection, bool bEnable, FVector2D BreakTimer, FVector2D RemovalTimer, bool bClusterCrumbling = false);

	/** Remove On Break settings of the first piece, X<0 when disabled or missing. */
	UFUNCTION(BlueprintPure, Category = "GregrTown|Destruction")
	static FVector4 GetRemoveOnBreak(const UGeometryCollection* GeometryCollection);
};
