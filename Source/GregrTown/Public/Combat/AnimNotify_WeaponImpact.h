// GregrTown - combat

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_WeaponImpact.generated.h"

/** Place on the frame where the weapon lands: triggers the holder's weapon impact. */
UCLASS(meta = (DisplayName = "Weapon Impact"))
class GREGRTOWN_API UAnimNotify_WeaponImpact : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override { return TEXT("Weapon Impact"); }
};
