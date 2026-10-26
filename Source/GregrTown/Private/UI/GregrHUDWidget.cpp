// GregrTown - UI

#include "UI/GregrHUDWidget.h"

#include "AI/MonsterSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Combat/HealthComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Game/GregrGameState.h"
#include "Scoring/ScoreSubsystem.h"
#include "UI/GregrFloatingScoreWidget.h"

#define LOCTEXT_NAMESPACE "GregrHUD"

FText UGregrHUDWidget::FormatTime(int32 Seconds)
{
	Seconds = FMath::Max(Seconds, 0);
	return FText::FromString(FString::Printf(TEXT("%d:%02d"), Seconds / 60, Seconds % 60));
}

void UGregrHUDWidget::BuildDefaultLayout()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	ScoreText = MakeText(TEXT("ScoreText"), FText::GetEmpty(), 44);
	ScoreText->SetJustification(ETextJustify::Left);
	if (UCanvasPanelSlot* ScoreSlot = Root->AddChildToCanvas(ScoreText))
	{
		ScoreSlot->SetAnchors(FAnchors(0.f, 0.f));
		ScoreSlot->SetPosition(FVector2D(40.f, 30.f));
		ScoreSlot->SetAutoSize(true);
	}

	// Kill counters under the score.
	SmallKillsText = MakeText(TEXT("SmallKillsText"), FText::GetEmpty(), 30);
	BigKillsText = MakeText(TEXT("BigKillsText"), FText::GetEmpty(), 30);
	float KillsY = 92.f;
	for (UTextBlock* KillsText : { SmallKillsText.Get(), BigKillsText.Get() })
	{
		KillsText->SetJustification(ETextJustify::Left);
		if (UCanvasPanelSlot* KillsSlot = Root->AddChildToCanvas(KillsText))
		{
			KillsSlot->SetAnchors(FAnchors(0.f, 0.f));
			KillsSlot->SetPosition(FVector2D(40.f, KillsY));
			KillsSlot->SetAutoSize(true);
		}
		KillsY += 40.f;
	}
	SetKills(EMonsterKind::Small, 0);
	SetKills(EMonsterKind::Big, 0);

	// Health bar, bottom left.
	UHorizontalBox* HealthRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HealthRow"));
	if (UHorizontalBoxSlot* LabelSlot = HealthRow->AddChildToHorizontalBox(MakeText(TEXT("HealthLabel"), LOCTEXT("Health", "Vie"), 32)))
	{
		LabelSlot->SetVerticalAlignment(VAlign_Center);
		LabelSlot->SetPadding(FMargin(0.f, 0.f, 16.f, 0.f));
	}
	HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
	HealthBar->SetPercent(1.f);
	HealthBar->SetFillColorAndOpacity(FLinearColor(0.15f, 0.85f, 0.2f));
	if (UHorizontalBoxSlot* BarSlot = HealthRow->AddChildToHorizontalBox(HealthBar))
	{
		BarSlot->SetVerticalAlignment(VAlign_Fill);
		BarSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	if (UCanvasPanelSlot* HealthSlot = Root->AddChildToCanvas(HealthRow))
	{
		HealthSlot->SetAnchors(FAnchors(0.f, 1.f));
		HealthSlot->SetAlignment(FVector2D(0.f, 1.f));
		HealthSlot->SetPosition(FVector2D(40.f, -40.f));
		HealthSlot->SetSize(FVector2D(480.f, 36.f));
	}

	TimerText = MakeText(TEXT("TimerText"), FText::GetEmpty(), 56);
	if (UCanvasPanelSlot* TimerSlot = Root->AddChildToCanvas(TimerText))
	{
		TimerSlot->SetAnchors(FAnchors(0.5f, 0.f));
		TimerSlot->SetAlignment(FVector2D(0.5f, 0.f));
		TimerSlot->SetPosition(FVector2D(0.f, 24.f));
		TimerSlot->SetAutoSize(true);
	}

	WaveText = MakeText(TEXT("WaveText"), FText::GetEmpty(), 32);
	if (UCanvasPanelSlot* WaveSlot = Root->AddChildToCanvas(WaveText))
	{
		WaveSlot->SetAnchors(FAnchors(0.5f, 0.f));
		WaveSlot->SetAlignment(FVector2D(0.5f, 0.f));
		WaveSlot->SetPosition(FVector2D(0.f, 100.f));
		WaveSlot->SetAutoSize(true);
	}

	ScoreToBeatText = MakeText(TEXT("ScoreToBeatText"), FText::GetEmpty(), 40);
	ScoreToBeatText->SetJustification(ETextJustify::Right);
	if (UCanvasPanelSlot* BestSlot = Root->AddChildToCanvas(ScoreToBeatText))
	{
		BestSlot->SetAnchors(FAnchors(1.f, 0.f));
		BestSlot->SetAlignment(FVector2D(1.f, 0.f));
		BestSlot->SetPosition(FVector2D(-40.f, 34.f));
		BestSlot->SetAutoSize(true);
	}
}

void UGregrHUDWidget::UpdateScoreToBeat(int32 Score)
{
	const AGregrGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AGregrGameState>() : nullptr;
	if (!ScoreToBeatText || !GameState)
	{
		return;
	}

	const int32 ToBeat = GameState->GetScoreToBeat();
	const bool bBeaten = ToBeat > 0 && Score > ToBeat;
	ScoreToBeatText->SetText(bBeaten
		? LOCTEXT("RecordBeaten", "Record battu !")
		: FText::Format(LOCTEXT("ScoreToBeat", "À battre : {0}"), FText::AsNumber(ToBeat)));
	ScoreToBeatText->SetColorAndOpacity(FSlateColor(bBeaten ? RecordColor : FLinearColor::White));
}

FText UGregrHUDWidget::GetScoreToBeatDisplay() const
{
	return ScoreToBeatText ? ScoreToBeatText->GetText() : FText::GetEmpty();
}

void UGregrHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UScoreSubsystem* Score = UWorld::GetSubsystem<UScoreSubsystem>(GetWorld()))
	{
		Score->OnScoreChanged.AddUniqueDynamic(this, &UGregrHUDWidget::HandleScoreChanged);
		SetScore(Score->GetScore());
	}
	else
	{
		SetScore(0);
	}

	if (AGregrGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AGregrGameState>() : nullptr)
	{
		GameState->OnChallengeTimeChanged.AddUniqueDynamic(this, &UGregrHUDWidget::HandleTimeChanged);
		HandleTimeChanged(FMath::CeilToInt32(GameState->GetRemainingTime()));
	}

	if (UMonsterSubsystem* Monsters = UWorld::GetSubsystem<UMonsterSubsystem>(GetWorld()))
	{
		Monsters->OnMonsterKilled.AddUniqueDynamic(this, &UGregrHUDWidget::HandleMonsterKilled);
		SetKills(EMonsterKind::Small, Monsters->GetKillCount(EMonsterKind::Small));
		SetKills(EMonsterKind::Big, Monsters->GetKillCount(EMonsterKind::Big));
	}
}

void UGregrHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Read every frame: the pawn can change (restart) and the bar must follow it.
	const APawn* Pawn = GetOwningPlayerPawn();
	const UHealthComponent* Health = Pawn ? Pawn->FindComponentByClass<UHealthComponent>() : nullptr;
	if (HealthBar && Health)
	{
		const float Percent = Health->GetHealthPercent();
		HealthBar->SetPercent(Percent);
		HealthBar->SetFillColorAndOpacity(Percent > 0.3f ? FLinearColor(0.15f, 0.85f, 0.2f) : FLinearColor(1.f, 0.2f, 0.1f));
	}
	UpdateWave();
}

void UGregrHUDWidget::UpdateWave()
{
	const UMonsterSubsystem* Monsters = UWorld::GetSubsystem<UMonsterSubsystem>(GetWorld());
	if (!WaveText || !Monsters)
	{
		return;
	}

	FText Text;
	if (Monsters->IsWaitingForNextWave())
	{
		Text = FText::Format(LOCTEXT("NextWave", "Vague {0} dans {1} s"),
			FText::AsNumber(Monsters->GetCurrentWave() + 1), FText::AsNumber(FMath::CeilToInt32(Monsters->GetTimeUntilNextWave())));
	}
	else if (Monsters->GetCurrentWave() > 0)
	{
		const int32 Left = Monsters->GetAliveMonsterCount();
		Text = FText::Format(LOCTEXT("Wave", "Vague {0} : {1} {1}|plural(one=monstre,other=monstres)"), Monsters->GetCurrentWave(), Left);
	}
	if (!Text.EqualTo(WaveText->GetText()))
	{
		WaveText->SetText(Text);
	}
}

FText UGregrHUDWidget::GetWaveDisplay() const
{
	return WaveText ? WaveText->GetText() : FText::GetEmpty();
}

void UGregrHUDWidget::HandleMonsterKilled(EMonsterKind Kind, int32 KillCount)
{
	SetKills(Kind, KillCount);
}

void UGregrHUDWidget::SetKills(EMonsterKind Kind, int32 KillCount)
{
	if (Kind == EMonsterKind::Small && SmallKillsText)
	{
		SmallKillsText->SetText(FText::Format(LOCTEXT("SmallKills", "Petits monstres : {0}"), FText::AsNumber(KillCount)));
	}
	else if (Kind == EMonsterKind::Big && BigKillsText)
	{
		BigKillsText->SetText(FText::Format(LOCTEXT("BigKills", "Gros monstres : {0}"), FText::AsNumber(KillCount)));
	}
}

FText UGregrHUDWidget::GetKillsDisplay(EMonsterKind Kind) const
{
	const UTextBlock* KillsText = Kind == EMonsterKind::Small ? SmallKillsText.Get() : BigKillsText.Get();
	return KillsText ? KillsText->GetText() : FText::GetEmpty();
}

float UGregrHUDWidget::GetHealthDisplay() const
{
	return HealthBar ? HealthBar->GetPercent() : 0.f;
}

void UGregrHUDWidget::NativeDestruct()
{
	if (UScoreSubsystem* Score = UWorld::GetSubsystem<UScoreSubsystem>(GetWorld()))
	{
		Score->OnScoreChanged.RemoveDynamic(this, &UGregrHUDWidget::HandleScoreChanged);
	}
	if (AGregrGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AGregrGameState>() : nullptr)
	{
		GameState->OnChallengeTimeChanged.RemoveDynamic(this, &UGregrHUDWidget::HandleTimeChanged);
	}
	if (UMonsterSubsystem* Monsters = UWorld::GetSubsystem<UMonsterSubsystem>(GetWorld()))
	{
		Monsters->OnMonsterKilled.RemoveDynamic(this, &UGregrHUDWidget::HandleMonsterKilled);
	}
	Super::NativeDestruct();
}

UGregrHUDWidget::UGregrHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FloatingScoreClass = UGregrFloatingScoreWidget::StaticClass();
}

void UGregrHUDWidget::HandleScoreChanged(int32 NewScore, int32 Delta, AActor* Source, AActor* Instigator, FVector Location)
{
	SetScore(NewScore);

	if (Delta > 0 && FloatingScoreClass && !Location.IsZero())
	{
		// Pops of the same blow arrive in the same frame: stack them.
		const uint64 Frame = GFrameCounter;
		StackIndex = Frame == LastPopFrame ? StackIndex + 1 : 0;
		LastPopFrame = Frame;

		if (UGregrFloatingScoreWidget* Floating = CreateWidget<UGregrFloatingScoreWidget>(GetOwningPlayer(), FloatingScoreClass))
		{
			Floating->Setup(Delta, Location, StackIndex);
			Floating->AddToViewport(5);
			FloatingScores.Add(Floating);
		}
	}
	FloatingScores.RemoveAll([](const TObjectPtr<UGregrFloatingScoreWidget>& Widget) { return !Widget || !Widget->IsInViewport(); });
}

int32 UGregrHUDWidget::GetActiveFloatingScoreCount() const
{
	int32 Count = 0;
	for (const TObjectPtr<UGregrFloatingScoreWidget>& Widget : FloatingScores)
	{
		Count += Widget && Widget->IsInViewport() ? 1 : 0;
	}
	return Count;
}

void UGregrHUDWidget::SetScore(int32 Score)
{
	CurrentScore = Score;
	if (ScoreText)
	{
		ScoreText->SetText(FText::Format(LOCTEXT("Score", "Score : {0}"), FText::AsNumber(Score)));
	}
	UpdateScoreToBeat(Score);
}

void UGregrHUDWidget::HandleTimeChanged(int32 SecondsLeft)
{
	UpdateScoreToBeat(CurrentScore);
	if (TimerText)
	{
		const AGregrGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AGregrGameState>() : nullptr;
		const bool bUnlimited = GameState && GameState->IsChallengeUnlimited();
		TimerText->SetText(bUnlimited ? LOCTEXT("Unlimited", "∞") : FormatTime(SecondsLeft));
		TimerText->SetColorAndOpacity(FSlateColor(!bUnlimited && SecondsLeft <= HurrySeconds ? HurryColor : FLinearColor::White));
	}
}

FText UGregrHUDWidget::GetScoreDisplay() const
{
	return ScoreText ? ScoreText->GetText() : FText::GetEmpty();
}

FText UGregrHUDWidget::GetTimerDisplay() const
{
	return TimerText ? TimerText->GetText() : FText::GetEmpty();
}

#undef LOCTEXT_NAMESPACE
