// GregrTown - destruction framework

#include "Destruction/DestructionStatics.h"

#include "Destruction/BreakableInterface.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

int32 UDestructionStatics::ApplyDestructionImpactInSphere(const UObject* WorldContextObject, const FDestructionImpact& Impact, TArray<AActor*>& OutHitActors, bool bDrawDebug)
{
	OutHitActors.Reset();

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World || Impact.Radius <= 0.f)
	{
		return 0;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(DestructionImpact), false);
	if (Impact.Instigator)
	{
		Params.AddIgnoredActor(Impact.Instigator);
	}

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps, Impact.Location, FQuat::Identity,
		FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects),
		FCollisionShape::MakeSphere(Impact.Radius), Params);

	TSet<AActor*> Processed;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor || Processed.Contains(Actor))
		{
			continue;
		}
		Processed.Add(Actor);

		if (ApplyDestructionImpactToActor(Actor, Impact))
		{
			OutHitActors.Add(Actor);
		}
	}

	if (bDrawDebug)
	{
		DrawDebugSphere(World, Impact.Location, Impact.Radius, 16, OutHitActors.Num() > 0 ? FColor::Red : FColor::Yellow, false, 2.f);
	}

	return OutHitActors.Num();
}

// Debug: "Gregr.Smash [Radius] [Strength]" hits whatever the player looks at, until the hammer exists.
static FAutoConsoleCommandWithWorldAndArgs GSmashCommand(
	TEXT("Gregr.Smash"),
	TEXT("Sends a destruction impact where the player is looking. Args: [Radius=200] [Strength=1]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		if (!PC)
		{
			return;
		}

		FVector ViewLocation;
		FRotator ViewRotation;
		PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(GregrSmash), true, PC->GetPawn());
		const FVector End = ViewLocation + ViewRotation.Vector() * 10000.f;
		if (!World->LineTraceSingleByChannel(Hit, ViewLocation, End, ECC_Visibility, Params))
		{
			return;
		}

		FDestructionImpact Impact;
		Impact.Location = Hit.ImpactPoint;
		Impact.Direction = ViewRotation.Vector();
		Impact.Radius = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 200.f;
		Impact.Strength = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 1.f;
		Impact.Instigator = PC->GetPawn();
		Impact.ImpactType = TEXT("Debug");

		TArray<AActor*> HitActors;
		UDestructionStatics::ApplyDestructionImpactInSphere(World, Impact, HitActors, true);
	}));

// Debug: "Gregr.Probe [Radius]" lists the actors around the player with their collision state,
// to find meshes the player can walk through.
static FAutoConsoleCommandWithWorldAndArgs GProbeCommand(
	TEXT("Gregr.Probe"),
	TEXT("Logs and prints the actors around the player and whether they block the pawn. Args: [Radius=300]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		if (!Pawn)
		{
			return;
		}

		const float Radius = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 300.f;
		const FVector Origin = Pawn->GetActorLocation();
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor == Pawn || Actor->IsA<APlayerController>())
			{
				continue;
			}
			FVector BoundsOrigin, Extent;
			Actor->GetActorBounds(false, BoundsOrigin, Extent);
			if (Extent.IsNearlyZero() || FBox(BoundsOrigin - Extent, BoundsOrigin + Extent).ComputeSquaredDistanceToPoint(Origin) > Radius * Radius)
			{
				continue;
			}

			FString Components;
			Actor->ForEachComponent<UPrimitiveComponent>(false, [&Components](UPrimitiveComponent* Primitive)
			{
				if (!Primitive->IsVisible() && Primitive->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
				{
					return;
				}
				const bool bBlocksPawn = Primitive->IsCollisionEnabled() && Primitive->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block;
				Components += FString::Printf(TEXT(" [%s %s%s]"), *Primitive->GetName(), bBlocksPawn ? TEXT("blocks") : TEXT("NO-COLLISION"),
					Primitive->IsVisible() ? TEXT("") : TEXT(" hidden"));
			});

			const FString Line = FString::Printf(TEXT("%s (%s)%s"), *Actor->GetActorNameOrLabel(), *Actor->GetClass()->GetName(), *Components);
			UE_LOG(LogGregrDestruction, Display, TEXT("Probe: %s"), *Line);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, Line.Contains(TEXT("NO-COLLISION")) ? FColor::Orange : FColor::White, Line);
			}
		}
	}));

bool UDestructionStatics::ApplyDestructionImpactToActor(AActor* Target, const FDestructionImpact& Impact)
{
	if (!Target || !Target->Implements<UBreakableInterface>())
	{
		return false;
	}

	if (!IBreakableInterface::Execute_CanReceiveDestructionImpact(Target, Impact))
	{
		return false;
	}

	IBreakableInterface::Execute_ReceiveDestructionImpact(Target, Impact);
	return true;
}
