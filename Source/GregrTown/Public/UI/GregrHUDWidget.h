// GregrTown - UI

#pragma once

#include "CoreMinimal.h"
#include "AI/GregrMonster.h"
#include "UI/GregrUserWidget.h"
#include "GregrHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

/** In-game overlay: score, monsters killed, player health and challenge countdown. */
UCLASS()
class GREGRTOWN_API UGregrHUDWidget : public UGregrUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetScoreDisplay() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetTimerDisplay() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetScoreToBeatDisplay() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetKillsDisplay(EMonsterKind Kind) const;

	/** "Vague 3 : 12 monstres" during a wave, "Vague 4 dans 15 s" during the countdown. */
	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetWaveDisplay() const;

	/** Fill of the player health bar, in [0, 1]. */
	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetHealthDisplay() const;

	/** "1:05" style text for a number of seconds. */
	static FText FormatTime(int32 Seconds);

protected:
	virtual void BuildDefaultLayout() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void HandleMonsterKilled(EMonsterKind Kind, int32 KillCount);

	void SetKills(EMonsterKind Kind, int32 KillCount);
	void UpdateWave();

	UFUNCTION()
	void HandleScoreChanged(int32 NewScore, int32 Delta, AActor* Source, AActor* Instigator, FVector Location);

	/** Class of the "+points" popping where points are earned. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UGregrFloatingScoreWidget> FloatingScoreClass;

	UPROPERTY(Transient)
	TArray<TObjectPtr<class UGregrFloatingScoreWidget>> FloatingScores;

	uint64 LastPopFrame = 0;
	int32 StackIndex = 0;

public:
	UGregrHUDWidget(const FObjectInitializer& ObjectInitializer);

	/** Floating "+points" currently on screen. */
	UFUNCTION(BlueprintPure, Category = "HUD")
	int32 GetActiveFloatingScoreCount() const;

protected:

	UFUNCTION()
	void HandleTimeChanged(int32 SecondsLeft);

	void SetScore(int32 Score);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TimerText;

	/** Small monsters killed, under the score. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SmallKillsText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BigKillsText;

	/** Wave number and monsters left, or countdown to the next wave, under the timer. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WaveText;

	/** Player health, bottom left. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	/** Best score for this challenge duration, top right. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreToBeatText;

	/** Color of the score to beat once beaten. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	FLinearColor RecordColor = FLinearColor(1.f, 0.78f, 0.1f);

	void UpdateScoreToBeat(int32 Score);

	int32 CurrentScore = 0;

	/** Countdown turns to this color for the last seconds. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	FLinearColor HurryColor = FLinearColor(1.f, 0.25f, 0.15f);

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	int32 HurrySeconds = 10;
};
