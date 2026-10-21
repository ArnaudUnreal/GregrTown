// GregrTown - combat

#include "Combat/AnimNotify_WeaponImpact.h"

#include "Combat/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_WeaponImpact::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (UCombatComponent* Combat = Owner ? Owner->FindComponentByClass<UCombatComponent>() : nullptr)
	{
		Combat->HandleWeaponImpact();
	}
}
