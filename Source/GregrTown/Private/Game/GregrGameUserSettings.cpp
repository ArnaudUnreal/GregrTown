// GregrTown - game

#include "Game/GregrGameUserSettings.h"

#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UGregrGameUserSettings* UGregrGameUserSettings::GetGregrUserSettings()
{
	return GEngine ? Cast<UGregrGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

void UGregrGameUserSettings::SetChallengeDuration(float Seconds)
{
	ChallengeDuration = FMath::Clamp(Seconds, MinChallengeDuration, UnlimitedChallengeDuration);
}

void UGregrGameUserSettings::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.f, 1.f);
}

void UGregrGameUserSettings::ApplyAudio(const UObject* WorldContextObject) const
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World)
	{
		return;
	}

	if (FAudioDeviceHandle AudioDevice = World->GetAudioDevice())
	{
		AudioDevice->SetTransientPrimaryVolume(MasterVolume);
	}
}

void UGregrGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();
	ChallengeDuration = 120.f;
	MasterVolume = 1.f;
}
