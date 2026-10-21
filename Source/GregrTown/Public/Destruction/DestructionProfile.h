// GregrTown - destruction framework

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DestructionProfile.generated.h"

class UNiagaraSystem;
class USoundBase;

/**
 * Shared description of how a family of objects reacts to impacts (wood, stone, plants...).
 * One asset is shared by many destructibles, so tuning the feel of a whole category is a single edit.
 * The class defaults are used when a destructible has no profile at all.
 */
UCLASS(BlueprintType)
class GREGRTOWN_API UDestructionProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- Physics ---

	/** Strain applied to clusters inside the impact radius (multiplied by impact Strength). Must exceed the GC damage thresholds to break. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "0"))
	float StrainMagnitude = 1000000.f;

	/** Multiplier applied to the impact radius for this kind of object. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "0"))
	float RadiusScale = 1.f;

	/** Velocity (cm/s) pushing pieces outwards from the impact point. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "0"))
	float RadialVelocity = 300.f;

	/** Velocity (cm/s) pushing pieces along the impact direction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "0"))
	float DirectionalVelocity = 600.f;

	/** Break the whole object on the first hit instead of only the area around the impact (good for small props). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
	bool bBreakEntirely = false;

	/** Number of consecutive frames the break fields are applied, so deeper fracture levels break too. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "1", ClampMax = "5"))
	int32 BreakPasses = 2;

	// --- Score ---

	/** First break awards points from the object size (Project Settings > GregrTown Destruction > Score). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score")
	bool bScoreFromSize = true;

	/** Points awarded the first time the object breaks, when not computed from its size. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score", meta = (EditCondition = "!bScoreFromSize"))
	int32 ScoreOnFirstBreak = 50;

	/** Later hits that break more pieces give this fraction of the object's points. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score", meta = (ClampMin = "0", ClampMax = "1"))
	float ExtraBreakScoreRatio = 0.25f;

	/** Minimum points of a later hit that breaks something. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score", meta = (ClampMin = "0"))
	int32 MinExtraBreakScore = 5;

	// --- Feedback ---

	/** Effect spawned the first time the object breaks. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	TObjectPtr<UNiagaraSystem> BreakEffect;

	/** Scale of the effects (also grows with the object size). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (ClampMin = "0.01"))
	float EffectScale = 1.f;

	/** Effect spawned on following hits. Falls back to BreakEffect when empty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	TObjectPtr<UNiagaraSystem> HitEffect;

	/** Sound played the first time the object breaks. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	TObjectPtr<USoundBase> BreakSound;

	/** Sound played on following hits. Falls back to BreakSound when empty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	TObjectPtr<USoundBase> HitSound;
};
