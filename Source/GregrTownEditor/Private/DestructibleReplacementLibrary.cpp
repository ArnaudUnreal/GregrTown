// GregrTown - editor tools

#include "DestructibleReplacementLibrary.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/StaticMeshComponent.h"
#include "Destruction/DestructionProfile.h"
#include "Destruction/DestructionSettings.h"
#include "Destruction/GCDestructibleActor.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "Misc/ScopedSlowTask.h"
#include "ScopedTransaction.h"
#include "Subsystems/EditorActorSubsystem.h"

#define LOCTEXT_NAMESPACE "GregrTownDestructibleReplacement"

DEFINE_LOG_CATEGORY_STATIC(LogGregrReplacement, Log, All);

FString FDestructibleReplacementReport::ToString() const
{
	return FString::Printf(TEXT("%d replaced, %d without Geometry Collection (%d meshes), %d excluded"),
		Replaced, SkippedNoGeometryCollection, MeshesWithoutGeometryCollection.Num(), SkippedExcluded);
}

UGeometryCollection* UDestructibleReplacementLibrary::FindGeometryCollectionForMesh(UStaticMesh* Mesh)
{
	const FSoftObjectPath Path = GetDefault<UDestructionSettings>()->GetGeometryCollectionPathForMesh(Mesh);
	if (!Path.IsValid())
	{
		return nullptr;
	}

	// Ask the registry first so missing assets don't spam load warnings.
	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	if (!AssetRegistry.GetAssetByObjectPath(Path).IsValid())
	{
		return nullptr;
	}

	return Cast<UGeometryCollection>(Path.TryLoad());
}

FDestructibleReplacementReport UDestructibleReplacementLibrary::ReplaceStaticMeshActors(bool bSelectedOnly, bool bDryRun)
{
	FDestructibleReplacementReport Report;

	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		return Report;
	}

	const UDestructionSettings* Settings = GetDefault<UDestructionSettings>();
	UClass* DestructibleClass = Settings->DestructibleActorClass.LoadSynchronous();
	if (!DestructibleClass)
	{
		DestructibleClass = AGCDestructibleActor::StaticClass();
	}

	TArray<AStaticMeshActor*> Candidates;
	if (bSelectedOnly)
	{
		GEditor->GetSelectedActors()->GetSelectedObjects<AStaticMeshActor>(Candidates);
	}
	else
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			Candidates.Add(*It);
		}
	}

	FScopedSlowTask SlowTask(Candidates.Num(), LOCTEXT("Replacing", "Replacing Static Meshes by destructibles..."));
	SlowTask.MakeDialogDelayed(0.5f, true);

	TOptional<FScopedTransaction> Transaction;
	if (!bDryRun)
	{
		Transaction.Emplace(LOCTEXT("ReplaceTransaction", "Replace Static Meshes by destructibles"));
	}

	TMap<UStaticMesh*, UGeometryCollection*> Cache;
	TArray<AActor*> ToDestroy;
	TArray<AActor*> Spawned;

	for (AStaticMeshActor* Source : Candidates)
	{
		SlowTask.EnterProgressFrame(1.f);
		if (SlowTask.ShouldCancel())
		{
			break;
		}

		UStaticMesh* Mesh = Source && Source->GetStaticMeshComponent() ? Source->GetStaticMeshComponent()->GetStaticMesh() : nullptr;
		if (!Mesh)
		{
			continue;
		}

		if (Settings->IsMeshExcluded(Mesh))
		{
			++Report.SkippedExcluded;
			continue;
		}

		UGeometryCollection** Cached = Cache.Find(Mesh);
		UGeometryCollection* GeometryCollection = Cached ? *Cached : Cache.Add(Mesh, FindGeometryCollectionForMesh(Mesh));
		if (!GeometryCollection)
		{
			++Report.SkippedNoGeometryCollection;
			Report.MeshesWithoutGeometryCollection.AddUnique(Mesh->GetPathName());
			continue;
		}

		++Report.Replaced;
		if (bDryRun)
		{
			continue;
		}

		if (AGCDestructibleActor* NewActor = SpawnReplacement(Source, GeometryCollection, DestructibleClass))
		{
			Spawned.Add(NewActor);
			ToDestroy.Add(Source);
		}
		else
		{
			--Report.Replaced;
		}
	}

	if (!bDryRun && ToDestroy.Num() > 0)
	{
		GEditor->SelectNone(false, true);
		GEditor->GetEditorSubsystem<UEditorActorSubsystem>()->DestroyActors(ToDestroy);

		if (bSelectedOnly)
		{
			for (AActor* Actor : Spawned)
			{
				GEditor->SelectActor(Actor, true, false);
			}
			GEditor->NoteSelectionChange();
		}
	}

	if (!bDryRun && Report.Replaced == 0 && Transaction.IsSet())
	{
		Transaction->Cancel();
	}

	UE_LOG(LogGregrReplacement, Display, TEXT("%s%s"), bDryRun ? TEXT("[Dry run] ") : TEXT(""), *Report.ToString());
	for (const FString& MeshPath : Report.MeshesWithoutGeometryCollection)
	{
		UE_LOG(LogGregrReplacement, Display, TEXT("  No Geometry Collection for %s (expected %s)"), *MeshPath,
			*Settings->GetGeometryCollectionPathForMesh(LoadObject<UStaticMesh>(nullptr, *MeshPath)).ToString());
	}

	return Report;
}

AGCDestructibleActor* UDestructibleReplacementLibrary::SpawnReplacement(AStaticMeshActor* Source, UGeometryCollection* GeometryCollection, UClass* DestructibleClass)
{
	UWorld* World = Source->GetWorld();

	FActorSpawnParameters Params;
	Params.OverrideLevel = Source->GetLevel();
	Params.ObjectFlags |= RF_Transactional;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGCDestructibleActor* NewActor = World->SpawnActor<AGCDestructibleActor>(DestructibleClass, Source->GetActorTransform(), Params);
	if (!NewActor)
	{
		UE_LOG(LogGregrReplacement, Warning, TEXT("Failed to spawn destructible for %s"), *Source->GetActorLabel());
		return nullptr;
	}

	// The intact look must stay identical to the source actor.
	UStaticMeshComponent* SourceComponent = Source->GetStaticMeshComponent();
	UStaticMeshComponent* MeshComponent = NewActor->GetStaticMeshComponent();
	NewActor->SetupDestructible(SourceComponent->GetStaticMesh(), GeometryCollection);
	MeshComponent->SetMobility(SourceComponent->Mobility);
	MeshComponent->SetCollisionProfileName(SourceComponent->GetCollisionProfileName());
	MeshComponent->SetCastShadow(SourceComponent->CastShadow);
	for (int32 Index = 0; Index < SourceComponent->OverrideMaterials.Num(); ++Index)
	{
		if (UMaterialInterface* Material = SourceComponent->OverrideMaterials[Index])
		{
			MeshComponent->SetMaterial(Index, Material);
		}
	}

	const UDestructionSettings* Settings = GetDefault<UDestructionSettings>();
	if (UDestructionProfile* RuleProfile = Settings->FindProfileForMesh(SourceComponent->GetStaticMesh()).LoadSynchronous())
	{
		NewActor->Profile = RuleProfile;
	}

	NewActor->SetActorLabel(Source->GetActorLabel());
	NewActor->SetFolderPath(Source->GetFolderPath());
	NewActor->Tags = Source->Tags;
	NewActor->Layers = Source->Layers;
	NewActor->SetActorHiddenInGame(Source->IsHidden());

	if (AActor* Parent = Source->GetAttachParentActor())
	{
		NewActor->AttachToActor(Parent, FAttachmentTransformRules::KeepWorldTransform, Source->GetAttachParentSocketName());
	}

	TArray<AActor*> Children;
	Source->GetAttachedActors(Children);
	for (AActor* Child : Children)
	{
		Child->Modify();
		Child->AttachToActor(NewActor, FAttachmentTransformRules::KeepWorldTransform);
	}

	return NewActor;
}

#undef LOCTEXT_NAMESPACE
