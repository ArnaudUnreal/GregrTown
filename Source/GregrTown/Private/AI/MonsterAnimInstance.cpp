// GregrTown - AI

#include "AI/MonsterAnimInstance.h"

void UMonsterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	bUseAltIdle = FMath::RandBool();
}
