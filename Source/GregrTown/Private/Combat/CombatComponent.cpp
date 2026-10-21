// GregrTown - combat

#include "Combat/CombatComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Combat/WeaponBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (DefaultWeaponClass)
	{
		EquipWeapon(DefaultWeaponClass);
	}
}

void UCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Weapon)
	{
		Weapon->Destroy();
		Weapon = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

AWeaponBase* UCombatComponent::EquipWeapon(TSubclassOf<AWeaponBase> WeaponClass)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !WeaponClass)
	{
		return nullptr;
	}

	if (Weapon)
	{
		Weapon->Destroy();
		Weapon = nullptr;
	}

	FActorSpawnParameters Params;
	Params.Owner = Character;
	Params.Instigator = Character;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Weapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, Character->GetActorTransform(), Params);
	if (Weapon)
	{
		Weapon->OnEquipped(Character, WeaponAttachSocket);
	}
	return Weapon;
}

bool UCombatComponent::TryAttack()
{
	if (!Weapon || !Weapon->GetAttackMontage())
	{
		return false;
	}

	if (bAttacking)
	{
		bAttackQueued = true;
		return true;
	}

	StartAttack();
	return bAttacking;
}

void UCombatComponent::StartAttack()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UAnimInstance* AnimInstance = Character && Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		return;
	}

	UAnimMontage* Montage = Weapon->GetAttackMontage();
	if (AnimInstance->Montage_Play(Montage, Weapon->GetAttackPlayRate()) <= 0.f)
	{
		return;
	}

	bAttacking = true;
	bAttackQueued = false;

	if (bFaceControlRotationOnAttack && Character->GetController())
	{
		Character->SetActorRotation(FRotator(0.f, Character->GetControlRotation().Yaw, 0.f));
	}
	SetAttackMovement(true);
	Weapon->OnAttackStarted();

	FOnMontageBlendingOutStarted BlendingOut;
	BlendingOut.BindUObject(this, &UCombatComponent::OnAttackMontageBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendingOut, Montage);

	OnAttackStarted.Broadcast();
}

void UCombatComponent::OnAttackMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	bAttacking = false;
	SetAttackMovement(false);

	if (bAttackQueued && !bInterrupted)
	{
		StartAttack();
	}
	bAttackQueued = false;
}

void UCombatComponent::HandleWeaponImpact()
{
	if (Weapon)
	{
		Weapon->PerformImpact();
		OnWeaponImpact.Broadcast();
	}
}

void UCombatComponent::SetAttackMovement(bool bEnable)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement)
	{
		return;
	}

	if (bEnable)
	{
		DefaultMaxWalkSpeed = Movement->MaxWalkSpeed;
		Movement->MaxWalkSpeed = DefaultMaxWalkSpeed * AttackMoveSpeedMultiplier;
	}
	else if (DefaultMaxWalkSpeed > 0.f)
	{
		Movement->MaxWalkSpeed = DefaultMaxWalkSpeed;
	}
}
