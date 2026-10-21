// GregrTown - editor tools

#include "GregrDestructionEditorLibrary.h"

#include "GeometryCollection/Facades/CollectionRemoveOnBreakFacade.h"
#include "GeometryCollection/GeometryCollection.h"
#include "GeometryCollection/GeometryCollectionObject.h"

bool UGregrDestructionEditorLibrary::SetRemoveOnBreak(UGeometryCollection* GeometryCollection, bool bEnable, FVector2D BreakTimer, FVector2D RemovalTimer, bool bClusterCrumbling)
{
	FGeometryCollection* Collection = GeometryCollection ? GeometryCollection->GetGeometryCollection().Get() : nullptr;
	if (!Collection)
	{
		return false;
	}

	GeometryCollection->Modify();

	GeometryCollection::Facades::FCollectionRemoveOnBreakFacade Facade(*Collection);
	Facade.DefineSchema();

	GeometryCollection::Facades::FRemoveOnBreakData Data;
	Data.SetEnabled(bEnable);
	Data.SetBreakTimer(BreakTimer.X, BreakTimer.Y);
	Data.SetRemovalTimer(RemovalTimer.X, RemovalTimer.Y);
	Data.SetClusterCrumbling(bClusterCrumbling);
	Facade.SetToAll(Data);

	GeometryCollection->InvalidateCollection();
	GeometryCollection->MarkPackageDirty();
	return true;
}

FVector4 UGregrDestructionEditorLibrary::GetRemoveOnBreak(const UGeometryCollection* GeometryCollection)
{
	const FGeometryCollection* Collection = GeometryCollection ? GeometryCollection->GetGeometryCollection().Get() : nullptr;
	if (!Collection)
	{
		return FVector4(-1, 0, 0, 0);
	}

	const GeometryCollection::Facades::FCollectionRemoveOnBreakFacade Facade(*Collection);
	if (!Facade.IsValid() || Collection->NumElements(FGeometryCollection::TransformGroup) == 0)
	{
		return FVector4(-1, 0, 0, 0);
	}

	const GeometryCollection::Facades::FRemoveOnBreakData Data = Facade.GetData(0);
	if (!Data.IsEnabled())
	{
		return FVector4(-1, 0, 0, 0);
	}
	return FVector4(Data.GetBreakTimer().X, Data.GetBreakTimer().Y, Data.GetRemovalTimer().X, Data.GetRemovalTimer().Y);
}
