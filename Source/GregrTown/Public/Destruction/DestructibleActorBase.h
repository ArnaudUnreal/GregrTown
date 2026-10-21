// GregrTown - destruction framework

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Destruction/BreakableInterface.h"
#include "DestructibleActorBase.generated.h"

class UDestructionProfile;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDestructibleImpact, ADestructibleActorBase*, Destructible, const FDestructionImpact&, Impact, bool, bFirstBreak);

/**
 * Base class of everything breakable in the level.
 * Handles the gameplay side of an impact (profile, score, effects, sound, events).
 * Subclasses only implement ApplyImpactPhysics() for their own physical representation
 * (Geometry Collection today, maybe swap-to-debris meshes or skeletal props later).
 */
UCLASS(Abstract, Blueprintable)
class GREGRTOWN_API ADestructibleActorBase : public AActor, public IBreakableInterface
{
	GENERATED_BODY()

public:
	ADestructibleActorBase();

	/** Profile used by this destructible. Empty = project default (Project Settings > GregrTown Destruction). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	TObjectPtr<UDestructionProfile> Profile;

	/** Multiplies every score awarded by this object (bigger buildings can be worth more). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0"))
	float ScoreMultiplier = 1.f;

	/** When false, impacts are ignored (e.g. indestructible variants or scripted moments). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	bool bCanBeDestroyed = true;

	/** Fired on every accepted impact. */
	UPROPERTY(BlueprintAssignable, Category = "Destruction")
	FOnDestructibleImpact OnImpact;

	/** Profile actually used: own profile, else the project default, else the profile class defaults. */
	UFUNCTION(BlueprintPure, Category = "Destruction")
	const UDestructionProfile* GetEffectiveProfile() const;

	UFUNCTION(BlueprintPure, Category = "Destruction")
	int32 GetHitCount() const { return HitCount; }

	/** Points given the first time this object breaks (before ScoreMultiplier). */
	UFUNCTION(BlueprintPure, Category = "Destruction")
	int32 GetBreakScore() const;

	/** Points given when a later hit breaks more pieces (before ScoreMultiplier). */
	UFUNCTION(BlueprintPure, Category = "Destruction")
	int32 GetExtraBreakScore() const;

	/** Size used for scoring: cube root of the local bounds volume, scale included (cm). */
	UFUNCTION(BlueprintPure, Category = "Destruction")
	virtual float GetDestructibleSize() const;

	/**
	 * Number of pieces broken off so far, used to only reward later hits that really break something.
	 * INDEX_NONE when the destructible cannot tell (every hit is then rewarded).
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction")
	virtual int32 GetBrokenPieceCount() const { return INDEX_NONE; }

	//~ IBreakableInterface
	virtual void ReceiveDestructionImpact_Implementation(const FDestructionImpact& Impact) override;
	virtual bool CanReceiveDestructionImpact_Implementation(const FDestructionImpact& Impact) const override;
	virtual bool IsBroken_Implementation() const override { return bBroken; }

protected:
	virtual void BeginPlay() override;

	/** Physical reaction to the impact. Implemented by concrete destructible types. */
	virtual void ApplyImpactPhysics(const FDestructionImpact& Impact, const UDestructionProfile& InProfile, bool bFirstBreak) {}

	/** Spawns effects and plays sounds. Override to customize feedback. */
	virtual void PlayImpactFeedback(const FDestructionImpact& Impact, const UDestructionProfile& InProfile, bool bFirstBreak);

	/** Computes and sends points to the score subsystem. Override for custom scoring rules. */
	virtual void AwardScore(const FDestructionImpact& Impact, const UDestructionProfile& InProfile, bool bFirstBreak);

	/** Applies ScoreMultiplier and sends the points. */
	void AddPoints(int32 BasePoints, const FDestructionImpact& Impact);

	/** Seconds to wait for break events before deciding whether a later hit broke something. */
	UPROPERTY(EditDefaultsOnly, Category = "Destruction")
	float ExtraBreakCheckDelay = 0.35f;

	/** Blueprint hook called after every accepted impact. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Destruction", meta = (DisplayName = "On Destruction Impact"))
	void K2_OnDestructionImpact(const FDestructionImpact& Impact, bool bFirstBreak);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Destruction")
	bool bBroken = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Destruction")
	int32 HitCount = 0;

	/** Size-based score, computed once at BeginPlay. */
	UPROPERTY(VisibleInstanceOnly, Category = "Destruction")
	int32 SizeBreakScore = 0;

	/** 0 = smallest objects, 1 = biggest. Scales feedback (effect size, sound pitch/volume). */
	UPROPERTY(VisibleInstanceOnly, Category = "Destruction")
	float SizeAlpha = 0.5f;
};
