// GregrTown - combat

#pragma once

#include "CoreMinimal.h"
#include "Combat/WeaponBase.h"
#include "MeleeWeapon.generated.h"

class UCameraShakeBase;
class UNiagaraSystem;
class USceneComponent;
class USoundBase;

/**
 * Weapon hitting an area around its ImpactPoint (hammer, club, giant spoon...).
 * Sends a FDestructionImpact to every breakable in range, plus its own feedback
 * (dust, thud, camera shake) so a blow in the void still feels good.
 */
UCLASS(Blueprintable)
class GREGRTOWN_API AMeleeWeapon : public AWeaponBase
{
	GENERATED_BODY()

public:
	AMeleeWeapon();

	virtual void PerformImpact() override;

protected:
	/** Where the blow lands (e.g. the hammer head). Child of the mesh, so it follows its scale. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> ImpactPoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Impact", meta = (ClampMin = "0"))
	float ImpactRadius = 220.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Impact", meta = (ClampMin = "0"))
	float ImpactStrength = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Impact")
	FName ImpactType = TEXT("Melee");

	/** Effect spawned at the impact point on every blow. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	/** Sound played on every blow. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback")
	TObjectPtr<USoundBase> ImpactSound;

	/** Camera shake played on the holder's camera on every blow. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback")
	TSubclassOf<UCameraShakeBase> ImpactCameraShake;

	/** Extra camera shake scale when something was broken. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback", meta = (ClampMin = "0"))
	float BreakCameraShakeScale = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Debug")
	bool bDrawDebugImpact = false;

	/** Blueprint hook after each blow, with the actors that received it. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon", meta = (DisplayName = "On Impact"))
	void K2_OnImpact(const FVector& Location, const TArray<AActor*>& HitActors);
};
