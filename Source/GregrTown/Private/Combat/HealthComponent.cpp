// GregrTown - combat

#include "Combat/HealthComponent.h"

#include "GameFramework/Controller.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	GetOwner()->OnTakeAnyDamage.AddUniqueDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	const double Now = GetWorld()->GetTimeSeconds();
	if (Damage <= 0.f || IsDead() || Now < InvulnerableUntil)
	{
		return;
	}

	InvulnerableUntil = Now + InvulnerabilityTime;
	AActor* Killer = InstigatedBy && InstigatedBy->GetPawn() ? InstigatedBy->GetPawn() : DamageCauser;
	ChangeHealth(Health - Damage, Killer);
}

void UHealthComponent::SetHealth(float NewHealth)
{
	if (!IsDead())
	{
		ChangeHealth(NewHealth, nullptr);
	}
}

void UHealthComponent::ChangeHealth(float NewHealth, AActor* Killer)
{
	const float OldHealth = Health;
	Health = FMath::Clamp(NewHealth, 0.f, MaxHealth);
	if (Health == OldHealth)
	{
		return;
	}

	OnHealthChanged.Broadcast(Health, Health - OldHealth);
	if (Health <= 0.f)
	{
		OnDied.Broadcast(Killer);
	}
}
