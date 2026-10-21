// GregrTown - destruction framework

#pragma once

#include "CoreMinimal.h"
#include "Destruction/DestructibleActorBase.h"
#include "Physics/Experimental/ChaosEventType.h"
#include "GCDestructibleActor.generated.h"

class UGeometryCollection;
class UGeometryCollectionComponent;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Destructible shown as a plain Static Mesh until its first impact, then swapped for a Chaos
 * Geometry Collection created on the fly. Intact objects cost exactly as much as a Static Mesh
 * actor (no Chaos proxy, no GC render data); only the objects actually hit pay for physics.
 * The Geometry Collection starts kinematic and breaks locally around each impact using
 * targeted physics fields, so only this actor is affected by its own hit.
 */
UCLASS(Blueprintable)
class GREGRTOWN_API AGCDestructibleActor : public ADestructibleActorBase
{
	GENERATED_BODY()

public:
	AGCDestructibleActor();

	/** Sets the intact mesh and the fractured asset used once hit. */
	UFUNCTION(BlueprintCallable, Category = "Destruction")
	void SetupDestructible(UStaticMesh* InStaticMesh, UGeometryCollection* InGeometryCollection);

	UFUNCTION(BlueprintPure, Category = "Destruction")
	UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

	/** Null until the first impact. */
	UFUNCTION(BlueprintPure, Category = "Destruction")
	UGeometryCollectionComponent* GetGeometryCollectionComponent() const { return GeometryCollectionComponent; }

	UFUNCTION(BlueprintPure, Category = "Destruction")
	UGeometryCollection* GetGeometryCollection() const { return GeometryCollection; }

#if WITH_EDITOR
	/** Breaks the object from its center, handy to tune profiles in PIE/Simulate. */
	UFUNCTION(CallInEditor, Category = "Destruction")
	void DebugSmash();
#endif

	virtual int32 GetBrokenPieceCount() const override { return BrokenPieceCount; }

	/** Pieces still rendered (removed debris shrink to nothing). 0 before the first impact. */
	UFUNCTION(BlueprintPure, Category = "Destruction")
	int32 GetVisiblePieceCount() const;

	/** Average distance (cm) the visible pieces moved down from their rest position. 0 before the first impact. */
	UFUNCTION(BlueprintPure, Category = "Destruction")
	float GetAveragePieceDrop() const;

	/**
	 * Once hit, the whole object becomes dynamic (after the local break): what was not broken
	 * falls or settles on the ground instead of floating, and can be smashed again.
	 * When false, unbroken parts stay in place.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
	bool bCollapseOnHit = true;

	/** Delay after the impact before the rest of the object is released (lets the local break happen first). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction", meta = (EditCondition = "bCollapseOnHit", ClampMin = "0"))
	float CollapseDelay = 0.25f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBreakEvent(const FChaosBreakEvent& BreakEvent);

	/** Turns every piece of the object dynamic. */
	void Collapse();

	int32 BrokenPieceCount = 0;

	virtual void ApplyImpactPhysics(const FDestructionImpact& Impact, const UDestructionProfile& InProfile, bool bFirstBreak) override;

	/** Creates the Geometry Collection component and hides the Static Mesh. Returns false if no GC asset is set. */
	virtual bool SwapToGeometryCollection();

	/** Override to tweak the freshly created component (damage, collision, removal...) before it is registered. */
	virtual void ConfigureGeometryCollectionComponent(UGeometryCollectionComponent* Component);

	/** Applies one pass of strain + dynamic state + velocity fields. */
	void ApplyBreakFields(const FVector& Location, const FVector& Direction, float Radius, float Strain, const UDestructionProfile& InProfile, float Strength);

	/** Intact representation. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	/** Fractured asset swapped in on the first impact. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
	TObjectPtr<UGeometryCollection> GeometryCollection;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Destruction")
	TObjectPtr<UGeometryCollectionComponent> GeometryCollectionComponent;
};
