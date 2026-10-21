// GregrTown - destruction framework

#include "Destruction/DestructibleActorBase.h"

#include "Destruction/DestructionProfile.h"
#include "Destruction/DestructionSettings.h"
#include "Scoring/ScoreSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogGregrDestruction);

ADestructibleActorBase::ADestructibleActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADestructibleActorBase::BeginPlay()
{
	Super::BeginPlay();

	const UDestructionSettings* Settings = GetDefault<UDestructionSettings>();
	const float Size = GetDestructibleSize();
	SizeBreakScore = Settings->ComputeBreakScore(Size);
	SizeAlpha = Settings->ComputeSizeAlpha(Size);
}

int32 ADestructibleActorBase::GetExtraBreakScore() const
{
	const UDestructionProfile* EffectiveProfile = GetEffectiveProfile();
	return FMath::Max(EffectiveProfile->MinExtraBreakScore, FMath::RoundToInt32(GetBreakScore() * EffectiveProfile->ExtraBreakScoreRatio));
}

float ADestructibleActorBase::GetDestructibleSize() const
{
	// Local bounds so rotated objects score like their unrotated twins.
	const FVector Extent = CalculateComponentsBoundingBoxInLocalSpace(true).GetSize() * GetActorScale3D().GetAbs();
	return FMath::Pow(FMath::Max(Extent.X, 1.0) * FMath::Max(Extent.Y, 1.0) * FMath::Max(Extent.Z, 1.0), 1.0 / 3.0);
}

int32 ADestructibleActorBase::GetBreakScore() const
{
	const UDestructionProfile* EffectiveProfile = GetEffectiveProfile();
	return EffectiveProfile && !EffectiveProfile->bScoreFromSize ? EffectiveProfile->ScoreOnFirstBreak : SizeBreakScore;
}

const UDestructionProfile* ADestructibleActorBase::GetEffectiveProfile() const
{
	if (Profile)
	{
		return Profile;
	}

	if (const UDestructionProfile* DefaultProfile = GetDefault<UDestructionSettings>()->DefaultProfile.LoadSynchronous())
	{
		return DefaultProfile;
	}

	return GetDefault<UDestructionProfile>();
}

bool ADestructibleActorBase::CanReceiveDestructionImpact_Implementation(const FDestructionImpact& Impact) const
{
	return bCanBeDestroyed && !IsActorBeingDestroyed();
}

void ADestructibleActorBase::ReceiveDestructionImpact_Implementation(const FDestructionImpact& Impact)
{
	if (!Execute_CanReceiveDestructionImpact(this, Impact))
	{
		return;
	}

	const UDestructionProfile* EffectiveProfile = GetEffectiveProfile();
	check(EffectiveProfile);

	const bool bFirstBreak = !bBroken;
	bBroken = true;
	++HitCount;

	ApplyImpactPhysics(Impact, *EffectiveProfile, bFirstBreak);
	PlayImpactFeedback(Impact, *EffectiveProfile, bFirstBreak);
	AwardScore(Impact, *EffectiveProfile, bFirstBreak);

	OnImpact.Broadcast(this, Impact, bFirstBreak);
	K2_OnDestructionImpact(Impact, bFirstBreak);
}

void ADestructibleActorBase::PlayImpactFeedback(const FDestructionImpact& Impact, const UDestructionProfile& InProfile, bool bFirstBreak)
{
	UNiagaraSystem* Effect = bFirstBreak ? InProfile.BreakEffect.Get() : InProfile.HitEffect.Get();
	if (!Effect)
	{
		Effect = InProfile.BreakEffect;
	}
	if (Effect)
	{
		// Bigger objects make bigger effects and deeper, louder sounds.
		const FRotator Rotation = Impact.Direction.IsNearlyZero() ? FRotator::ZeroRotator : Impact.Direction.Rotation();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Effect, Impact.Location, Rotation, FVector(InProfile.EffectScale * FMath::Lerp(0.6f, 1.6f, SizeAlpha)));
	}

	USoundBase* Sound = bFirstBreak ? InProfile.BreakSound.Get() : InProfile.HitSound.Get();
	if (!Sound)
	{
		Sound = InProfile.BreakSound;
	}
	if (Sound)
	{
		const float Volume = FMath::Lerp(0.6f, 1.f, SizeAlpha);
		const float Pitch = FMath::Lerp(1.25f, 0.8f, SizeAlpha) * FMath::FRandRange(0.94f, 1.06f);
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Impact.Location, Volume, Pitch);
	}
}

void ADestructibleActorBase::AwardScore(const FDestructionImpact& Impact, const UDestructionProfile& InProfile, bool bFirstBreak)
{
	if (bFirstBreak)
	{
		AddPoints(GetBreakScore(), Impact);
		return;
	}

	// Later hits: only reward them if they actually broke more pieces.
	const int32 BrokenBefore = GetBrokenPieceCount();
	if (BrokenBefore == INDEX_NONE)
	{
		AddPoints(GetExtraBreakScore(), Impact);
		return;
	}

	FTimerHandle Handle;
	TWeakObjectPtr<ADestructibleActorBase> WeakThis(this);
	GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([WeakThis, BrokenBefore, Impact]()
	{
		if (ADestructibleActorBase* This = WeakThis.Get(); This && This->GetBrokenPieceCount() > BrokenBefore)
		{
			This->AddPoints(This->GetExtraBreakScore(), Impact);
		}
	}), ExtraBreakCheckDelay, false);
}

void ADestructibleActorBase::AddPoints(int32 BasePoints, const FDestructionImpact& Impact)
{
	const int32 Points = FMath::RoundToInt32(BasePoints * ScoreMultiplier);
	if (Points == 0)
	{
		return;
	}

	if (UScoreSubsystem* ScoreSubsystem = UWorld::GetSubsystem<UScoreSubsystem>(GetWorld()))
	{
		ScoreSubsystem->AddScore(Points, this, Impact.Instigator, Impact.Location);
	}
}
