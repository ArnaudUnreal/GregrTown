// GregrTown - destruction framework

#include "Destruction/GCDestructibleActor.h"

#include "Components/StaticMeshComponent.h"
#include "Destruction/DestructionProfile.h"
#include "Destruction/DestructionSettings.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Field/FieldSystemObjects.h"
#include "GeometryCollection/GeometryCollection.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "TimerManager.h"

AGCDestructibleActor::AGCDestructibleActor()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComponent->SetMobility(EComponentMobility::Static);
	RootComponent = StaticMeshComponent;
}

void AGCDestructibleActor::SetupDestructible(UStaticMesh* InStaticMesh, UGeometryCollection* InGeometryCollection)
{
	StaticMeshComponent->SetStaticMesh(InStaticMesh);
	GeometryCollection = InGeometryCollection;
}

bool AGCDestructibleActor::SwapToGeometryCollection()
{
	if (GeometryCollectionComponent)
	{
		return true;
	}
	if (!GeometryCollection)
	{
		return false;
	}

	UGeometryCollectionComponent* Component = NewObject<UGeometryCollectionComponent>(this, TEXT("GeometryCollection"));
	Component->SetMobility(EComponentMobility::Movable);
	Component->SetupAttachment(StaticMeshComponent);
	// Stays in place until hit: pieces become dynamic through the impact fields (and Collapse()).
	Component->ObjectType = EObjectStateTypeEnum::Chaos_Object_Kinematic;
	Component->bAllowRemovalOnBreak = true;
	// Pieces do not block characters: debris pushed the player into floors and trapped it.
	// The whole object collapses right after the hit (bCollapseOnHit), so nothing standing becomes walk-through for long.
	Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	// Nor the camera: flying debris made the camera boom shrink and grow again while a big object collapsed.
	Component->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Component->SetNotifyBreaks(true);
	Component->OnChaosBreakEvent.AddDynamic(this, &AGCDestructibleActor::HandleBreakEvent);
	Component->SetRestCollection(GeometryCollection);

	// Fracturing keeps the source material slots first, interior slots are appended.
	for (int32 Index = 0; Index < StaticMeshComponent->OverrideMaterials.Num(); ++Index)
	{
		if (UMaterialInterface* Material = StaticMeshComponent->OverrideMaterials[Index])
		{
			Component->SetMaterial(Index, Material);
		}
	}

	ConfigureGeometryCollectionComponent(Component);

	Component->RegisterComponent();
	AddInstanceComponent(Component);
	GeometryCollectionComponent = Component;

	StaticMeshComponent->SetVisibility(false);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	return true;
}

void AGCDestructibleActor::ConfigureGeometryCollectionComponent(UGeometryCollectionComponent* Component)
{
}

void AGCDestructibleActor::BeginPlay()
{
	// Profile rules of the project settings (by mesh folder) apply to actors without their own profile.
	if (!Profile)
	{
		Profile = GetDefault<UDestructionSettings>()->FindProfileForMesh(StaticMeshComponent->GetStaticMesh()).LoadSynchronous();
	}

	Super::BeginPlay();
}

void AGCDestructibleActor::HandleBreakEvent(const FChaosBreakEvent& BreakEvent)
{
	++BrokenPieceCount;
}

float AGCDestructibleActor::GetAveragePieceDrop() const
{
	const UGeometryCollection* Collection = GeometryCollectionComponent ? GeometryCollectionComponent->GetRestCollection() : nullptr;
	if (!Collection || !Collection->GetGeometryCollection())
	{
		return 0.f;
	}

	const TArray<FTransform3f>& Current = GeometryCollectionComponent->GetComponentSpaceTransforms3f();
	const TManagedArray<FTransform3f>& Rest = Collection->GetGeometryCollection()->Transform;
	const float ScaleZ = static_cast<float>(GeometryCollectionComponent->GetComponentScale().Z);

	double Total = 0.0;
	int32 Count = 0;
	for (int32 Index = 0; Index < Current.Num() && Index < Rest.Num(); ++Index)
	{
		if (Current[Index].GetScale3D().GetMin() > 0.05f)
		{
			// Rest transforms are parent-relative, but fractured pieces are direct children of an identity root.
			Total += FMath::Max(0.f, (Rest[Index].GetTranslation().Z - Current[Index].GetTranslation().Z) * ScaleZ);
			++Count;
		}
	}
	return Count > 0 ? static_cast<float>(Total / Count) : 0.f;
}

int32 AGCDestructibleActor::GetVisiblePieceCount() const
{
	if (!GeometryCollectionComponent)
	{
		return 0;
	}

	// Removed pieces are scaled down to nothing before being disabled.
	int32 Count = 0;
	for (const FTransform3f& Transform : GeometryCollectionComponent->GetComponentSpaceTransforms3f())
	{
		Count += Transform.GetScale3D().GetMin() > 0.05f ? 1 : 0;
	}
	return Count;
}

void AGCDestructibleActor::ApplyImpactPhysics(const FDestructionImpact& Impact, const UDestructionProfile& InProfile, bool bFirstBreak)
{
	if (!SwapToGeometryCollection())
	{
		UE_LOG(LogGregrDestruction, Warning, TEXT("%s has no Geometry Collection, impact ignored."), *GetName());
		return;
	}

	FVector Location = Impact.Location;
	float Radius = Impact.Radius * InProfile.RadiusScale;

	if (InProfile.bBreakEntirely)
	{
		// Cover the whole object so every piece breaks and becomes dynamic.
		FVector Origin;
		FVector Extent;
		GetActorBounds(false, Origin, Extent);
		Radius = FMath::Max(Radius, FVector::Dist(Location, Origin) + Extent.Size());
	}

	const float Strain = InProfile.StrainMagnitude * Impact.Strength;
	const FVector Direction = Impact.Direction.GetSafeNormal();

	ApplyBreakFields(Location, Direction, Radius, Strain, InProfile, Impact.Strength);

	// Pieces released by a break only exist on the physics thread after the next step:
	// re-apply the fields for a few frames so they also become dynamic and get pushed.
	const int32 ExtraPasses = InProfile.BreakPasses - 1;
	if (ExtraPasses > 0)
	{
		TWeakObjectPtr<AGCDestructibleActor> WeakThis(this);
		TWeakObjectPtr<const UDestructionProfile> WeakProfile(&InProfile);
		TSharedRef<int32> RemainingPasses = MakeShared<int32>(ExtraPasses);
		TSharedRef<FTimerDelegate> NextPass = MakeShared<FTimerDelegate>();
		NextPass->BindLambda([WeakThis, WeakProfile, RemainingPasses, NextPass, Location, Direction, Radius, Strain, Strength = Impact.Strength]()
		{
			AGCDestructibleActor* This = WeakThis.Get();
			const UDestructionProfile* PassProfile = WeakProfile.Get();
			if (!This || !PassProfile)
			{
				NextPass->Unbind();
				return;
			}

			This->ApplyBreakFields(Location, Direction, Radius, Strain, *PassProfile, Strength);

			if (--(*RemainingPasses) > 0)
			{
				This->GetWorldTimerManager().SetTimerForNextTick(*NextPass);
			}
			else
			{
				// Breaks the self-reference cycle of the shared delegate.
				NextPass->Unbind();
			}
		});
		GetWorldTimerManager().SetTimerForNextTick(*NextPass);
	}

	// Unbroken parts would otherwise stay kinematic in the air: release the whole object
	// once the local break is done, so the rest falls or settles and can be hit again.
	if (bCollapseOnHit)
	{
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateUObject(this, &AGCDestructibleActor::Collapse), CollapseDelay, false);
	}
}

void AGCDestructibleActor::Collapse()
{
	if (GeometryCollectionComponent)
	{
		FVector Origin;
		FVector Extent;
		GetActorBounds(false, Origin, Extent);
		GeometryCollectionComponent->ApplyKinematicField(Extent.Size() * 2.f + 100.f, Origin);
	}
}

void AGCDestructibleActor::ApplyBreakFields(const FVector& Location, const FVector& Direction, float Radius, float Strain, const UDestructionProfile& InProfile, float Strength)
{
	UGeometryCollectionComponent* GC = GeometryCollectionComponent;

	// 1. Break clusters inside the radius.
	URadialFalloff* StrainField = NewObject<URadialFalloff>(this);
	StrainField->SetRadialFalloff(Strain, 0.f, 1.f, 0.f, Radius, Location, EFieldFalloffType::Field_FallOff_None);
	GC->ApplyPhysicsField(true, EGeometryCollectionPhysicsTypeEnum::Chaos_ExternalClusterStrain, nullptr, StrainField);

	// 1b. Fields are sampled at each cluster's center of mass: a big remaining chunk hit on its edge
	// would ignore them. Strain the children of every active chunk touched by the impact sphere instead.
	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, Location, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects),
		FCollisionShape::MakeSphere(Radius), FCollisionQueryParams(SCENE_QUERY_STAT(GregrBreakChunks), false));
	TSet<int32> StrainedItems;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (Overlap.GetComponent() == GC && Overlap.ItemIndex != INDEX_NONE && !StrainedItems.Contains(Overlap.ItemIndex))
		{
			StrainedItems.Add(Overlap.ItemIndex);
			GC->ApplyExternalStrain(Overlap.ItemIndex, Location, Radius, 0, 1.f, Strain);
		}
	}

	// 2. Kinematic -> dynamic inside the radius, the rest of the object stays in place.
	GC->ApplyKinematicField(Radius, Location);

	// 3. Push pieces: outwards from the impact + along the blow direction, masked by the radius.
	const float RadialSpeed = InProfile.RadialVelocity * Strength;
	const float DirectionalSpeed = Direction.IsNearlyZero() ? 0.f : InProfile.DirectionalVelocity * Strength;
	if (RadialSpeed <= 0.f && DirectionalSpeed <= 0.f)
	{
		return;
	}

	URadialVector* RadialVector = NewObject<URadialVector>(this);
	RadialVector->SetRadialVector(RadialSpeed, Location);

	UUniformVector* DirectionalVector = NewObject<UUniformVector>(this);
	DirectionalVector->SetUniformVector(DirectionalSpeed, Direction);

	UOperatorField* Velocity = NewObject<UOperatorField>(this);
	Velocity->SetOperatorField(1.f, RadialVector, DirectionalVector, EFieldOperationType::Field_Add);

	URadialFalloff* Mask = NewObject<URadialFalloff>(this);
	Mask->SetRadialFalloff(1.f, 0.f, 1.f, 0.f, Radius, Location, EFieldFalloffType::Field_Falloff_Linear);

	UOperatorField* MaskedVelocity = NewObject<UOperatorField>(this);
	MaskedVelocity->SetOperatorField(1.f, Mask, Velocity, EFieldOperationType::Field_Multiply);

	GC->ApplyPhysicsField(true, EGeometryCollectionPhysicsTypeEnum::Chaos_LinearVelocity, nullptr, MaskedVelocity);
}

#if WITH_EDITOR
void AGCDestructibleActor::DebugSmash()
{
	FDestructionImpact Impact;
	Impact.Location = GetActorLocation();
	Impact.Direction = FVector::UpVector;
	Impact.Radius = 300.f;
	Execute_ReceiveDestructionImpact(this, Impact);
}
#endif
