// GregrTown - destruction framework

#include "Destruction/DestructionSettings.h"

#include "Destruction/DestructionProfile.h"
#include "Destruction/GCDestructibleActor.h"
#include "Engine/StaticMesh.h"

UDestructionSettings::UDestructionSettings()
{
	CategoryName = TEXT("Game");
	DestructibleActorClass = AGCDestructibleActor::StaticClass();
}

float UDestructionSettings::ComputeSizeAlpha(float SizeCm) const
{
	const float MinSize = FMath::Max(1.f, SizeForMinScore);
	const float MaxSize = FMath::Max(MinSize + 1.f, SizeForMaxScore);

	// Log scale: doubling the size always adds the same amount.
	return FMath::Clamp(FMath::Loge(FMath::Max(SizeCm, 1.f) / MinSize) / FMath::Loge(MaxSize / MinSize), 0.f, 1.f);
}

int32 UDestructionSettings::ComputeBreakScore(float SizeCm) const
{
	return FMath::RoundToInt32(FMath::Lerp(static_cast<float>(MinBreakScore), static_cast<float>(MaxBreakScore), ComputeSizeAlpha(SizeCm)));
}

FSoftObjectPath UDestructionSettings::GetGeometryCollectionPathForMesh(const UStaticMesh* Mesh) const
{
	if (!Mesh)
	{
		return FSoftObjectPath();
	}

	// /Game/Foo/SM_Bar  ->  /Game/Foo/GC/GC_SM_Bar.GC_SM_Bar
	const FString MeshFolder = FPackageName::GetLongPackagePath(Mesh->GetOutermost()->GetName());
	const FString AssetName = GeometryCollectionPrefix + Mesh->GetName();
	const FString Folder = GeometryCollectionSubfolder.IsEmpty() ? MeshFolder : MeshFolder / GeometryCollectionSubfolder;
	return FSoftObjectPath(FString::Printf(TEXT("%s/%s.%s"), *Folder, *AssetName, *AssetName));
}

bool UDestructionSettings::IsMeshExcluded(const UStaticMesh* Mesh) const
{
	if (!Mesh)
	{
		return true;
	}

	const FString MeshPath = Mesh->GetPathName();
	for (const FString& Filter : ExcludedMeshPathFilters)
	{
		if (!Filter.IsEmpty() && MeshPath.Contains(Filter))
		{
			return true;
		}
	}
	return false;
}

TSoftObjectPtr<UDestructionProfile> UDestructionSettings::FindProfileForMesh(const UStaticMesh* Mesh) const
{
	if (Mesh)
	{
		const FString MeshPath = Mesh->GetPathName();
		for (const FDestructionProfileRule& Rule : ProfileRules)
		{
			if (!Rule.PathContains.IsEmpty() && MeshPath.Contains(Rule.PathContains))
			{
				return Rule.Profile;
			}
		}
	}
	return nullptr;
}
