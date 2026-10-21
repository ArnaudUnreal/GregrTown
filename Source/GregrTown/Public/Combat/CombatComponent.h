// GregrTown - combat

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeaponBase;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEvent);

/**
 * Gives a character the ability to hold a weapon and attack with it.
 * Works for the player and could work for AI or other pawns: input only calls TryAttack().
 */
UCLASS(ClassGroup = (Gregr), meta = (BlueprintSpawnableComponent))
class GREGRTOWN_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	/** Weapon spawned and equipped at BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<AWeaponBase> DefaultWeaponClass;

	/** Bone or socket of the owner's mesh holding the weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FName WeaponAttachSocket = TEXT("righthand");

	/** Walk speed multiplier while attacking (0 = rooted). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0", ClampMax = "1"))
	float AttackMoveSpeedMultiplier = 0.3f;

	/** Turn the character towards the camera direction when an attack starts. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bFaceControlRotationOnAttack = true;

	/** Starts an attack, or queues one if already attacking. Returns true if an attack started or was queued. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool TryAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	AWeaponBase* EquipWeapon(TSubclassOf<AWeaponBase> WeaponClass);

	UFUNCTION(BlueprintPure, Category = "Combat")
	AWeaponBase* GetWeapon() const { return Weapon; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const { return bAttacking; }

	/** Called by AnimNotify_WeaponImpact. */
	void HandleWeaponImpact();

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackEvent OnAttackStarted;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackEvent OnWeaponImpact;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void StartAttack();
	void OnAttackMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void SetAttackMovement(bool bEnable);

	UPROPERTY(Transient)
	TObjectPtr<AWeaponBase> Weapon;

	bool bAttacking = false;
	bool bAttackQueued = false;
	float DefaultMaxWalkSpeed = 0.f;
};
