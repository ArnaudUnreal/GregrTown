// GregrTown - AI

#pragma once

#include "CoreMinimal.h"
#include "Destruction/BreakableInterface.h"
#include "GameFramework/Character.h"
#include "GregrMonster.generated.h"

class UAnimMontage;
class UAnimSequenceBase;
class UHealthComponent;
class UNiagaraSystem;
class USoundBase;
class UWidgetComponent;

UENUM(BlueprintType)
enum class EMonsterKind : uint8
{
	Small,
	Big
};

/**
 * Base of the enemies. Children (Blueprints) only set the mesh, animations and numbers.
 * The hammer hurts them through IBreakableInterface like any breakable; they hit back with
 * UGameplayStatics::ApplyDamage. Decisions are taken by AMonsterAIController (sight perception).
 */
UCLASS(Abstract, Blueprintable)
class GREGRTOWN_API AGregrMonster : public ACharacter, public IBreakableInterface
{
	GENERATED_BODY()

public:
	AGregrMonster();

	/** Kill counter of the HUD this monster counts for. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster")
	EMonsterKind Kind = EMonsterKind::Small;

	/** Score earned by the player who kills it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster")
	int32 KillPoints = 20;

	/** Health lost per hammer blow (scaled by the blow strength). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster", meta = (ClampMin = "0"))
	float DamagePerImpact = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Movement")
	float WalkSpeed = 500.f;

	/** Speed while chasing the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Movement")
	float RunSpeed = 800.f;

	/** Pushed back this fast when hit (cm/s). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Movement", meta = (ClampMin = "0"))
	float KnockbackSpeed = 450.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Attack", meta = (ClampMin = "0"))
	float AttackDamage = 5.f;

	/** Attacks when the target is this close (2D distance between the actors). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Attack", meta = (ClampMin = "0"))
	float AttackRange = 150.f;

	/** Time between two attack starts. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Attack", meta = (ClampMin = "0"))
	float AttackInterval = 1.5f;

	/** When the blow lands, in seconds from the start of AttackAnim. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Attack", meta = (ClampMin = "0"))
	float AttackImpactTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Animation")
	TObjectPtr<UAnimSequenceBase> AttackAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Animation")
	TObjectPtr<UAnimSequenceBase> HitReactAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Animation")
	TObjectPtr<UAnimSequenceBase> DeathAnim;

	/** The body disappears this long after the end of the death animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Animation", meta = (ClampMin = "0"))
	float CorpseDelay = 2.f;

	/** Played where the hammer hits it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Feedback")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Feedback")
	TObjectPtr<UNiagaraSystem> HitEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Feedback")
	TObjectPtr<USoundBase> AttackSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Feedback")
	TObjectPtr<USoundBase> DeathSound;

	/** The health bar is hidden farther than this from the camera, and when the monster is not rendered (behind a wall). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Health Bar", meta = (ClampMin = "0"))
	float HealthBarMaxDistance = 3000.f;

	UFUNCTION(BlueprintPure, Category = "Monster")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "Monster")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Monster")
	bool IsAttacking() const { return bAttacking; }

	/** Playing the hit reaction: no move, no attack. */
	UFUNCTION(BlueprintPure, Category = "Monster")
	bool IsStunned() const;

	UFUNCTION(BlueprintPure, Category = "Monster")
	bool IsRunning() const { return bRunning; }

	UFUNCTION(BlueprintPure, Category = "Monster")
	bool IsHealthBarVisible() const;

	/** Walk (wandering) or run (chasing) speed. */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	void SetRunning(bool bRun);

	/** True when not attacking, not stunned, alive and the attack interval has elapsed. */
	UFUNCTION(BlueprintPure, Category = "Monster")
	bool CanAttack() const;

	/** Starts an attack on Target; the damage is applied at AttackImpactTime if it is still in reach. */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	bool Attack(AActor* Target);

	/** Turns towards the controller focus instead of the movement direction (while attacking). */
	void SetFaceFocus(bool bFace);

	//~ IBreakableInterface
	virtual void ReceiveDestructionImpact_Implementation(const FDestructionImpact& Impact) override;
	virtual bool CanReceiveDestructionImpact_Implementation(const FDestructionImpact& Impact) const override;
	virtual bool IsBroken_Implementation() const override { return IsDead(); }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float Delta);

	UFUNCTION()
	void HandleDied(AActor* Killer);

	void ApplyAttackDamage();
	void EndAttack();
	void UpdateHealthBar();

	/** Plays a sequence on the default slot; bHoldLastFrame keeps the final pose (death). */
	float PlayAnim(UAnimSequenceBase* Anim, bool bHoldLastFrame = false);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
	TObjectPtr<UHealthComponent> HealthComponent;

	/** Screen space health bar above the head. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
	TObjectPtr<UWidgetComponent> HealthBar;

	TWeakObjectPtr<AActor> AttackTarget;
	bool bAttacking = false;
	bool bRunning = false;
	double LastAttackTime = -1000.0;
	double StunnedUntil = -1.0;

	FTimerHandle AttackImpactTimer;
	FTimerHandle AttackEndTimer;
	FTimerHandle HealthBarTimer;
};
