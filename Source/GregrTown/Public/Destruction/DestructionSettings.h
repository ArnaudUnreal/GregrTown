// GregrTown - destruction framework

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DestructionSettings.generated.h"

class AGCDestructibleActor;
class UDestructionProfile;
class UStaticMesh;

/** Assigns a profile to every mesh whose path contains PathContains (e.g. "/building/"). */
USTRUCT(BlueprintType)
struct GREGRTOWN_API FDestructionProfileRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
	FString PathContains;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
	TSoftObjectPtr<UDestructionProfile> Profile;
};

/** Project Settings > Game > GregrTown Destruction */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "GregrTown Destruction"))
class GREGRTOWN_API UDestructionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDestructionSettings();

	/** Class spawned when replacing a Static Mesh actor. Can be a Blueprint child of GCDestructibleActor. */
	UPROPERTY(Config, EditAnywhere, Category = "Replacement")
	TSoftClassPtr<AGCDestructibleActor> DestructibleActorClass;

	/** Folder, relative to the Static Mesh folder, holding the Geometry Collections. */
	UPROPERTY(Config, EditAnywhere, Category = "Replacement")
	FString GeometryCollectionSubfolder = TEXT("GC");

	/** Prefix added to the Static Mesh name to get the Geometry Collection name. */
	UPROPERTY(Config, EditAnywhere, Category = "Replacement")
	FString GeometryCollectionPrefix = TEXT("GC_");

	/** Meshes whose path contains one of these strings are never replaced (e.g. "/cliff/"). */
	UPROPERTY(Config, EditAnywhere, Category = "Replacement")
	TArray<FString> ExcludedMeshPathFilters;

	/** Profile used by destructibles without their own profile. */
	UPROPERTY(Config, EditAnywhere, Category = "Profiles")
	TSoftObjectPtr<UDestructionProfile> DefaultProfile;

	/** Profiles assigned by the replacement tool according to the mesh path. First match wins. */
	UPROPERTY(Config, EditAnywhere, Category = "Profiles")
	TArray<FDestructionProfileRule> ProfileRules;

	/** Points of the smallest objects. */
	UPROPERTY(Config, EditAnywhere, Category = "Score", meta = (ClampMin = "0"))
	int32 MinBreakScore = 10;

	/** Points of the biggest objects. */
	UPROPERTY(Config, EditAnywhere, Category = "Score", meta = (ClampMin = "0"))
	int32 MaxBreakScore = 100;

	/** Object size (cube root of its bounds volume, cm) giving MinBreakScore or less. */
	UPROPERTY(Config, EditAnywhere, Category = "Score", meta = (ClampMin = "1"))
	float SizeForMinScore = 40.f;

	/** Object size (cube root of its bounds volume, cm) giving MaxBreakScore or more. */
	UPROPERTY(Config, EditAnywhere, Category = "Score", meta = (ClampMin = "1"))
	float SizeForMaxScore = 800.f;

	/** Integer score for an object of this size, on a logarithmic scale between Min and Max. */
	int32 ComputeBreakScore(float SizeCm) const;

	/** 0 for the smallest objects, 1 for the biggest (same log scale as the score). Drives feedback too. */
	float ComputeSizeAlpha(float SizeCm) const;

	/** Path of the Geometry Collection expected for this mesh, following the naming convention above. */
	FSoftObjectPath GetGeometryCollectionPathForMesh(const UStaticMesh* Mesh) const;

	bool IsMeshExcluded(const UStaticMesh* Mesh) const;

	/** Profile rule matching the mesh path, or null when none matches. */
	TSoftObjectPtr<UDestructionProfile> FindProfileForMesh(const UStaticMesh* Mesh) const;
};
