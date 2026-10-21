// GregrTown - combat

#include "Combat/MeleeWeapon.h"

#include "Camera/CameraShakeBase.h"
#include "Components/StaticMeshComponent.h"
#include "Destruction/DestructionStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

AMeleeWeapon::AMeleeWeapon()
{
	ImpactPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ImpactPoint"));
	ImpactPoint->SetupAttachment(Mesh);
}

void AMeleeWeapon::PerformImpact()
{
	ACharacter* HolderCharacter = GetHolder();
	const FVector Location = ImpactPoint->GetComponentLocation();

	FDestructionImpact Impact;
	Impact.Location = Location;
	Impact.Direction = HolderCharacter ? HolderCharacter->GetActorForwardVector() : GetActorForwardVector();
	Impact.Radius = ImpactRadius;
	Impact.Strength = ImpactStrength;
	Impact.Instigator = HolderCharacter;
	Impact.Causer = this;
	Impact.ImpactType = ImpactType;

	TArray<AActor*> HitActors;
	UDestructionStatics::ApplyDestructionImpactInSphere(this, Impact, HitActors, bDrawDebugImpact);

	if (ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, Location);
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Location);
	}
	if (ImpactCameraShake && HolderCharacter)
	{
		if (APlayerController* PC = Cast<APlayerController>(HolderCharacter->GetController()))
		{
			PC->ClientStartCameraShake(ImpactCameraShake, HitActors.Num() > 0 ? BreakCameraShakeScale : 1.f);
		}
	}

	K2_OnImpact(Location, HitActors);
}
