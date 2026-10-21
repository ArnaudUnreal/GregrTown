// GregrTown - combat

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AController;
class UDamageType;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDied, AActor*, Killer);

/**
 * Health of its owner (player or monster), fed by the engine damage pipeline:
 * anything calls UGameplayStatics::ApplyDamage on the owner, this component listens to OnTakeAnyDamage.
 */
UCLASS(ClassGroup = (Gregr), meta = (BlueprintSpawnableComponent))
class GREGRTOWN_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1"))
	float MaxHealth = 100.f;

	/** No damage is taken during this time after a hit, so a crowd cannot spam blows. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "0"))
	float InvulnerabilityTime = 0.f;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return Health <= 0.f; }

	/** Debug / tests: sets the health directly (0 kills). */
	UFUNCTION(BlueprintCallable, Category = "Health|Debug")
	void SetHealth(float NewHealth);

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDied OnDied;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	void ChangeHealth(float NewHealth, AActor* Killer);

	float Health = 0.f;
	double InvulnerableUntil = -1.0;
};
