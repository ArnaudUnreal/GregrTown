// GregrTown - character

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GregrAnimInstance.generated.h"

class UCharacterMovementComponent;

/** Gameplay values read by the character Anim Blueprints, computed in C++ once per frame. */
UCLASS()
class GREGRTOWN_API UGregrAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** Horizontal speed in cm/s. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float GroundSpeed = 0.f;

	/** Moving with input (avoids sliding into the walk anim while braking). */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bShouldMove = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking = false;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> MovementComponent;
};
