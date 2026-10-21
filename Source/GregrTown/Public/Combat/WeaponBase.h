// GregrTown - combat

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class ACharacter;
class UAnimMontage;
class USoundBase;
class UStaticMeshComponent;

/**
 * Anything a character can hold and attack with.
 * The weapon owns what is specific to it (mesh, grip, attack animation, what happens on impact);
 * the character's UCombatComponent only decides when to attack.
 */
UCLASS(Abstract, Blueprintable)
class GREGRTOWN_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

	/** Attaches the weapon to the holder's mesh. */
	virtual void OnEquipped(ACharacter* NewHolder, FName AttachSocket);

	/** Called by the attack animation (AnimNotify_WeaponImpact) at the moment the blow lands. */
	virtual void PerformImpact() {}

	/** Called when an attack starts (swing sound...). */
	virtual void OnAttackStarted();

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UAnimMontage* GetAttackMontage() const { return AttackMontage; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetAttackPlayRate() const { return AttackPlayRate; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	ACharacter* GetHolder() const { return Holder.Get(); }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/**
	 * Transform of the weapon relative to the holder socket/bone it is attached to.
	 * When GripSocketName is set, its location is an extra offset applied after the socket alignment.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FTransform GripOffset;

	/** Socket of the weapon mesh held in the hand. When set, the weapon is moved so this socket sits exactly on the holder's attach socket. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName GripSocketName;

	/** Montage played on attack. Must contain an AnimNotify_WeaponImpact. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0.1"))
	float AttackPlayRate = 1.f;

	/** Played on the weapon when an attack starts. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback")
	TObjectPtr<USoundBase> SwingSound;

	TWeakObjectPtr<ACharacter> Holder;
};
