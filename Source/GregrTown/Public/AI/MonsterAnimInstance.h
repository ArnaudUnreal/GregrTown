// GregrTown - AI

#pragma once

#include "CoreMinimal.h"
#include "Character/GregrAnimInstance.h"
#include "MonsterAnimInstance.generated.h"

/** Monster Anim Blueprints: locomotion values of UGregrAnimInstance plus a random idle per monster. */
UCLASS()
class GREGRTOWN_API UMonsterAnimInstance : public UGregrAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;

	/** Picks one of the two idle animations, so a crowd does not breathe in sync. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bUseAltIdle = false;
};
