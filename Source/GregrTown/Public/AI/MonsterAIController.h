// GregrTown - AI

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "MonsterAIController.generated.h"

class AGregrMonster;
class UAISenseConfig_Sight;

UENUM(BlueprintType)
enum class EMonsterAIState : uint8
{
	Idle,
	Wander,
	Chase,
	Attack,
	/** Lost sight of the player: goes where it was last seen. */
	Search,
	Stunned,
	Dead
};

/**
 * Brain of AGregrMonster, a small state machine ticked a few times per second:
 * wander around the level, chase the player once seen, attack in reach.
 * Only the player is a sight stimulus source (see DefaultGame.ini), so hundreds of monsters stay cheap.
 */
UCLASS()
class GREGRTOWN_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMonsterAIController();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster AI|Sight", meta = (ClampMin = "0"))
	float SightRadius = 2500.f;

	/** Sight is lost beyond this distance. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster AI|Sight", meta = (ClampMin = "0"))
	float LoseSightRadius = 3000.f;

	/** Half of the field of view (70 = 140 degrees). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster AI|Sight", meta = (ClampMin = "0", ClampMax = "180"))
	float VisionHalfAngle = 70.f;

	/** Each wandering walk goes to a random reachable point this close. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster AI|Wander", meta = (ClampMin = "0"))
	float WanderRadius = 3000.f;

	/** Random idle time between two wandering walks. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster AI|Wander", meta = (ClampMin = "0"))
	FVector2D WanderPause = FVector2D(1.f, 4.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster AI", meta = (ClampMin = "0.02"))
	float ThinkInterval = 0.2f;

	UFUNCTION(BlueprintPure, Category = "Monster AI")
	AActor* GetTarget() const { return Target.Get(); }

	UFUNCTION(BlueprintPure, Category = "Monster AI")
	EMonsterAIState GetAIState() const { return State; }

	/** True while chasing in a straight line because the navmesh has no full path (wall broken after it was built). */
	UFUNCTION(BlueprintPure, Category = "Monster AI")
	bool IsChasingDirectly() const { return bDirectChase; }

	/** Chase this actor as if it had been seen (also when hit from behind). */
	UFUNCTION(BlueprintCallable, Category = "Monster AI")
	void SetTarget(AActor* NewTarget);

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void Think();
	void Chase(AGregrMonster* Monster, AActor* ChaseTarget);
	void Search(AGregrMonster* Monster);
	void Wander(AGregrMonster* Monster);
	void SetState(EMonsterAIState NewState);
	static bool IsAlive(const AActor* Actor);

	UPROPERTY(VisibleAnywhere, Category = "Monster AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	TWeakObjectPtr<AActor> Target;
	FVector LastKnownLocation = FVector::ZeroVector;
	bool bHasLastKnownLocation = false;
	bool bDirectChase = false;
	EMonsterAIState State = EMonsterAIState::Idle;
	double NextWanderTime = 0.0;
	double NextChaseUpdate = 0.0;
	FTimerHandle ThinkTimer;
};
