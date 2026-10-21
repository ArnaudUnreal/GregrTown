// GregrTown - editor tools

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DestructibleReplacementLibrary.generated.h"

class AGCDestructibleActor;
class AStaticMeshActor;
class UGeometryCollection;
class UStaticMesh;

USTRUCT(BlueprintType)
struct GREGRTOWNEDITOR_API FDestructibleReplacementReport
{
	GENERATED_BODY()

	/** Static Mesh actors replaced (or that would be, in dry run). */
	UPROPERTY(BlueprintReadOnly, Category = "Destruction")
	int32 Replaced = 0;

	/** Actors whose mesh has no matching Geometry Collection. */
	UPROPERTY(BlueprintReadOnly, Category = "Destruction")
	int32 SkippedNoGeometryCollection = 0;

	/** Actors whose mesh matches an exclusion filter of the project settings. */
	UPROPERTY(BlueprintReadOnly, Category = "Destruction")
	int32 SkippedExcluded = 0;

	/** Meshes found in the level without a Geometry Collection (unique paths). */
	UPROPERTY(BlueprintReadOnly, Category = "Destruction")
	TArray<FString> MeshesWithoutGeometryCollection;

	FString ToString() const;
};

/**
 * Editor helpers turning Static Mesh actors into destructibles.
 * Exposed to Blueprint/Python so they can be scripted or wrapped in Editor Utility Widgets later.
 */
UCLASS()
class GREGRTOWNEDITOR_API UDestructibleReplacementLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Geometry Collection matching the mesh naming convention (Project Settings > GregrTown Destruction), or null. */
	UFUNCTION(BlueprintCallable, Category = "GregrTown|Destruction")
	static UGeometryCollection* FindGeometryCollectionForMesh(UStaticMesh* Mesh);

	/**
	 * Replaces Static Mesh actors of the current editor level by destructible actors when a matching
	 * Geometry Collection exists. Undoable as a single transaction.
	 * @param bSelectedOnly only process the selected actors
	 * @param bDryRun only count, change nothing
	 */
	UFUNCTION(BlueprintCallable, Category = "GregrTown|Destruction")
	static FDestructibleReplacementReport ReplaceStaticMeshActors(bool bSelectedOnly, bool bDryRun = false);

private:
	static AGCDestructibleActor* SpawnReplacement(AStaticMeshActor* Source, UGeometryCollection* GeometryCollection, UClass* DestructibleClass);
};
