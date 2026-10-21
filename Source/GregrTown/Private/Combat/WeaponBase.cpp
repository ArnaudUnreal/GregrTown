// GregrTown - combat

#include "Combat/WeaponBase.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->CanCharacterStepUpOn = ECB_No;
	RootComponent = Mesh;
}

void AWeaponBase::OnAttackStarted()
{
	if (SwingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SwingSound, GetActorLocation(), 1.f, FMath::FRandRange(0.95f, 1.08f));
	}
}

void AWeaponBase::OnEquipped(ACharacter* NewHolder, FName AttachSocket)
{
	Holder = NewHolder;
	SetOwner(NewHolder);
	SetInstigator(NewHolder);

	if (NewHolder)
	{
		AttachToComponent(NewHolder->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocket);

		FTransform Relative = GripOffset;
		const UStaticMeshSocket* GripSocket = Mesh->GetStaticMesh() && !GripSocketName.IsNone() ? Mesh->GetStaticMesh()->FindSocket(GripSocketName) : nullptr;
		if (GripSocket)
		{
			// Bring the grip socket onto the holder socket: rotate/scale its local position like the mesh, then cancel it.
			const FVector GripLocation = GripOffset.GetRotation().RotateVector(GripOffset.GetScale3D() * GripSocket->RelativeLocation);
			Relative.SetLocation(GripOffset.GetLocation() - GripLocation);
		}
		Mesh->SetRelativeTransform(Relative);
	}
}
