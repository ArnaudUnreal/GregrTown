// GregrTown - game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "GregrGameUserSettings.generated.h"

/**
 * Player settings saved in GameUserSettings.ini (challenge duration, volume...).
 * Registered in DefaultEngine.ini as the engine GameUserSettings class.
 */
UCLASS(Config = GameUserSettings, ConfigDoNotCheckDefaults)
class GREGRTOWN_API UGregrGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	static constexpr float MinChallengeDuration = 30.f;
	static constexpr float MaxChallengeDuration = 600.f;

	UFUNCTION(BlueprintPure, Category = "Settings", meta = (DisplayName = "Get Gregr User Settings"))
	static UGregrGameUserSettings* GetGregrUserSettings();

	/** Challenge length in seconds. */
	UFUNCTION(BlueprintPure, Category = "Settings")
	float GetChallengeDuration() const { return ChallengeDuration; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetChallengeDuration(float Seconds);

	/** Master volume, 0..1. */
	UFUNCTION(BlueprintPure, Category = "Settings")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetMasterVolume(float Volume);

	/** Pushes the volume to the audio device of this world. */
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (WorldContext = "WorldContextObject"))
	void ApplyAudio(const UObject* WorldContextObject) const;

	virtual void SetToDefaults() override;

protected:
	UPROPERTY(Config)
	float ChallengeDuration = 120.f;

	UPROPERTY(Config)
	float MasterVolume = 1.f;
};
