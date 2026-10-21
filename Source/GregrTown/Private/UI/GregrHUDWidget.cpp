// GregrTown - UI

#include "UI/GregrHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
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

	TimerText = MakeText(TEXT("TimerText"), FText::GetEmpty(), 56);
	if (UCanvasPanelSlot* TimerSlot = Root->AddChildToCanvas(TimerText))
	{
		TimerSlot->SetAnchors(FAnchors(0.5f, 0.f));
		TimerSlot->SetAlignment(FVector2D(0.5f, 0.f));
		TimerSlot->SetPosition(FVector2D(0.f, 24.f));
		TimerSlot->SetAutoSize(true);
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
		TimerText->SetText(FormatTime(SecondsLeft));
		TimerText->SetColorAndOpacity(FSlateColor(SecondsLeft <= HurrySeconds ? HurryColor : FLinearColor::White));
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
