// GregrTown - character

#include "Character/FallRecoveryComponent.h"

#include "Components/CapsuleComponent.h"
#include "Destruction/DestructibleActorBase.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LandscapeProxy.h"

UFallRecoveryComponent::UFallRecoveryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.2f;
}

void UFallRecoveryComponent::BeginPlay()
{
	Super::BeginPlay();
	LastSafeLocation = GetOwner()->GetActorLocation();
}

void UFallRecoveryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement)
	{
		return;
	}

	// A character inside a floor slab still counts as walking: it never falls, so check the overlap itself.
	const bool bInside = IsInsideGeometry();
	StuckTime = bInside ? StuckTime + DeltaTime : 0.f;

	if (Movement->IsMovingOnGround())
	{
		FallingTime = 0.f;
		// Debris and breakable objects can vanish under the feet: only solid ground is a safe spot.
		const AActor* Floor = Movement->CurrentFloor.HitResult.GetActor();
		if (Movement->CurrentFloor.bBlockingHit && !Cast<ADestructibleActorBase>(Floor) && !bInside)
		{
			LastSafeLocation = Character->GetActorLocation();
		}
	}
	else if (Movement->IsFalling())
	{
		FallingTime += DeltaTime;
	}

	const bool bTooLong = FallingTime > MaxFallTime;
	const bool bTooLow = Character->GetActorLocation().Z < LastSafeLocation.Z - MaxDropBelowSafe;
	const bool bStuck = StuckTime > MaxStuckTime;
	if (bTooLong || bTooLow || bStuck || IsUnderTerrain())
	{
		RecoverNow();
	}
}

bool UFallRecoveryComponent::IsInsideGeometry() const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UCapsuleComponent* Capsule = Character ? Character->GetCapsuleComponent() : nullptr;
	if (!Capsule)
	{
		return false;
	}

	// Slightly smaller than the capsule: touching the floor or a wall is not being inside it.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FallRecoveryStuck), false, Character);
	FCollisionResponseParams ResponseParams;
	Capsule->InitSweepCollisionParams(Params, ResponseParams);
	return GetWorld()->OverlapBlockingTestByChannel(Capsule->GetComponentLocation(), Capsule->GetComponentQuat(),
		Capsule->GetCollisionObjectType(), Capsule->GetCollisionShape(-5.f), Params, ResponseParams);
}

bool UFallRecoveryComponent::IsUnderTerrain() const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const FVector Feet = Character->GetActorLocation() - FVector(0.f, 0.f, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	// Look down from high above: if the landscape is hit above the feet, we are underneath it.
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FallRecovery), true, Character);
	const FVector Start = Feet + FVector(0.f, 0.f, 20000.f);
	TArray<FHitResult> Hits;
	GetWorld()->LineTraceMultiByObjectType(Hits, Start, Feet, FCollisionObjectQueryParams(ECC_WorldStatic), Params);
	for (const FHitResult& Candidate : Hits)
	{
		if (Cast<ALandscapeProxy>(Candidate.GetActor()) && Candidate.ImpactPoint.Z > Feet.Z + 50.f)
		{
			return true;
		}
	}
	return false;
}

void UFallRecoveryComponent::RecoverNow()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	const FVector Target = LastSafeLocation + FVector(0.f, 0.f, 50.f);
	Character->TeleportTo(Target, Character->GetActorRotation(), false, true);
	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
	FallingTime = 0.f;
	StuckTime = 0.f;
	++RecoveryCount;
	OnRecovered.Broadcast(Target);
}
