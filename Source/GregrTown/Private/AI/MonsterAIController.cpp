// GregrTown - AI

#include "AI/MonsterAIController.h"

#include "AI/GregrMonster.h"
#include "Combat/HealthComponent.h"
#include "Game/GregrGameState.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TimerManager.h"

AMonsterAIController::AMonsterAIController()
{
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	Perception->ConfigureSense(*SightConfig);
	Perception->SetDominantSense(UAISense_Sight::StaticClass());
	SetPerceptionComponent(*Perception);
}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Values of this class (or of a Blueprint child) win over the config defaults.
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = FMath::Max(LoseSightRadius, SightRadius);
	SightConfig->PeripheralVisionAngleDegrees = VisionHalfAngle;
	GetPerceptionComponent()->ConfigureSense(*SightConfig);
	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AMonsterAIController::HandleTargetPerceptionUpdated);

	// Spread the crowd over the frames.
	NextWanderTime = GetWorld()->GetTimeSeconds() + FMath::FRandRange(WanderPause.X, WanderPause.Y);
	GetWorldTimerManager().SetTimer(ThinkTimer, this, &AMonsterAIController::Think, ThinkInterval, true, FMath::FRandRange(0.01f, ThinkInterval));
}

void AMonsterAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(ThinkTimer);
	Super::OnUnPossess();
}

void AMonsterAIController::SetTarget(AActor* NewTarget)
{
	Target = NewTarget;
	if (NewTarget)
	{
		LastKnownLocation = NewTarget->GetActorLocation();
		bHasLastKnownLocation = true;
	}
}

void AMonsterAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	const APawn* SeenPawn = Cast<APawn>(Actor);
	if (!SeenPawn || !SeenPawn->IsPlayerControlled())
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		Target = Actor;
	}
	else if (Target == Actor)
	{
		Target.Reset();
		LastKnownLocation = Stimulus.StimulusLocation;
		bHasLastKnownLocation = true;
	}
}

bool AMonsterAIController::IsAlive(const AActor* Actor)
{
	const UHealthComponent* Health = Actor ? Actor->FindComponentByClass<UHealthComponent>() : nullptr;
	return Actor && (!Health || !Health->IsDead());
}

void AMonsterAIController::SetState(EMonsterAIState NewState)
{
	State = NewState;
}

void AMonsterAIController::Think()
{
	AGregrMonster* Monster = Cast<AGregrMonster>(GetPawn());
	if (!Monster)
	{
		return;
	}

	if (Monster->IsDead())
	{
		StopMovement();
		ClearFocus(EAIFocusPriority::Gameplay);
		GetWorldTimerManager().ClearTimer(ThinkTimer);
		SetState(EMonsterAIState::Dead);
		return;
	}

	// Nothing moves before the challenge starts nor after it ends (time up or player dead).
	const AGregrGameState* GameState = GetWorld()->GetGameState<AGregrGameState>();
	if (GameState && !GameState->IsChallengeRunning())
	{
		if (State != EMonsterAIState::Idle)
		{
			StopMovement();
			ClearFocus(EAIFocusPriority::Gameplay);
			Monster->SetRunning(false);
			SetState(EMonsterAIState::Idle);
		}
		return;
	}

	if (Monster->IsStunned())
	{
		if (State != EMonsterAIState::Stunned)
		{
			StopMovement();
			SetState(EMonsterAIState::Stunned);
		}
		return;
	}

	AActor* ChaseTarget = Target.Get();
	if (ChaseTarget && !IsAlive(ChaseTarget))
	{
		Target.Reset();
		bHasLastKnownLocation = false;
		ChaseTarget = nullptr;
	}

	if (ChaseTarget)
	{
		Chase(Monster, ChaseTarget);
	}
	else if (bHasLastKnownLocation)
	{
		Search(Monster);
	}
	else
	{
		Wander(Monster);
	}
}

void AMonsterAIController::Chase(AGregrMonster* Monster, AActor* ChaseTarget)
{
	SetFocus(ChaseTarget);
	Monster->SetRunning(true);
	LastKnownLocation = ChaseTarget->GetActorLocation();
	bHasLastKnownLocation = true;

	const double Now = GetWorld()->GetTimeSeconds();
	const float Distance = FVector::Dist2D(Monster->GetActorLocation(), ChaseTarget->GetActorLocation());
	if (Distance <= Monster->AttackRange)
	{
		if (State != EMonsterAIState::Attack)
		{
			StopMovement();
			SetState(EMonsterAIState::Attack);
		}
		Monster->Attack(ChaseTarget);
		return;
	}
	if (Monster->IsAttacking())
	{
		// Finish the swing before running after the target.
		return;
	}

	const bool bMoving = GetMoveStatus() == EPathFollowingStatus::Moving;
	if (State != EMonsterAIState::Chase || !bMoving || Now >= NextChaseUpdate)
	{
		NextChaseUpdate = Now + 1.0;
		const float Acceptance = Monster->AttackRange * 0.5f;
		// The navmesh is built with the houses intact: no full path out of a house through a broken wall.
		// The monster sees its target, so a straight line works then (walls still standing block it).
		bDirectChase = false;
		const EPathFollowingRequestResult::Type Result = MoveToActor(ChaseTarget, Acceptance, true, true, false, nullptr, true);
		const FNavPathSharedPtr Path = GetPathFollowingComponent() ? GetPathFollowingComponent()->GetPath() : nullptr;
		if (Result == EPathFollowingRequestResult::Failed || (Path.IsValid() && Path->IsPartial()))
		{
			bDirectChase = true;
			MoveToActor(ChaseTarget, Acceptance, true, false, false, nullptr, true);
		}
	}
	SetState(EMonsterAIState::Chase);
}

void AMonsterAIController::Search(AGregrMonster* Monster)
{
	ClearFocus(EAIFocusPriority::Gameplay);
	Monster->SetRunning(false);

	if (State != EMonsterAIState::Search)
	{
		SetState(EMonsterAIState::Search);
		if (MoveToLocation(LastKnownLocation, 80.f) != EPathFollowingRequestResult::RequestSuccessful)
		{
			bHasLastKnownLocation = false;
		}
	}
	else if (GetMoveStatus() != EPathFollowingStatus::Moving)
	{
		// Reached the last known location (or could not): back to wandering.
		bHasLastKnownLocation = false;
		NextWanderTime = GetWorld()->GetTimeSeconds() + FMath::FRandRange(WanderPause.X, WanderPause.Y);
		SetState(EMonsterAIState::Idle);
	}
}

void AMonsterAIController::Wander(AGregrMonster* Monster)
{
	ClearFocus(EAIFocusPriority::Gameplay);
	Monster->SetRunning(false);

	const double Now = GetWorld()->GetTimeSeconds();
	if (GetMoveStatus() == EPathFollowingStatus::Moving)
	{
		return;
	}
	if (State != EMonsterAIState::Idle)
	{
		// Arrived (or was interrupted): pause before the next walk.
		SetState(EMonsterAIState::Idle);
		NextWanderTime = Now + FMath::FRandRange(WanderPause.X, WanderPause.Y);
		return;
	}
	if (Now < NextWanderTime)
	{
		return;
	}

	FNavLocation Destination;
	const UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (Navigation && Navigation->GetRandomReachablePointInRadius(Monster->GetActorLocation(), WanderRadius, Destination)
		&& MoveToLocation(Destination.Location, 50.f) == EPathFollowingRequestResult::RequestSuccessful)
	{
		SetState(EMonsterAIState::Wander);
	}
	else
	{
		NextWanderTime = Now + 2.0;
	}
}
