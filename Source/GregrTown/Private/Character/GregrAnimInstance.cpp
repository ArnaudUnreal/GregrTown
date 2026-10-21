// GregrTown - character

#include "Character/GregrAnimInstance.h"

#include "Combat/CombatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGregrAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (const ACharacter* Character = Cast<ACharacter>(TryGetPawnOwner()))
	{
		MovementComponent = Character->GetCharacterMovement();
	}
}

void UGregrAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!MovementComponent)
	{
		return;
	}

	GroundSpeed = MovementComponent->Velocity.Size2D();
	bShouldMove = GroundSpeed > 3.f && !MovementComponent->GetCurrentAcceleration().IsNearlyZero();
	bIsFalling = MovementComponent->IsFalling();

	const AActor* Owner = MovementComponent->GetOwner();
	const UCombatComponent* Combat = Owner ? Owner->FindComponentByClass<UCombatComponent>() : nullptr;
	bIsAttacking = Combat && Combat->IsAttacking();
}
