// GregrTown - UI

#include "UI/GregrEndScreenWidget.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

#define LOCTEXT_NAMESPACE "GregrEndScreen"

void UGregrEndScreenWidget::BuildDefaultLayout()
{
	UVerticalBox* Content = MakeModalPanel();

	AddRow(Content, MakeText(TEXT("Title"), LOCTEXT("Title", "Temps écoulé !"), 64), 24.f);
	FinalScoreText = MakeText(TEXT("FinalScoreText"), FText::GetEmpty(), 52);
	AddRow(Content, FinalScoreText, 16.f);

	NewRecordText = MakeText(TEXT("NewRecordText"), LOCTEXT("NewRecord", "Nouveau record !"), 44);
	NewRecordText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.78f, 0.1f)));
	NewRecordText->SetVisibility(ESlateVisibility::Collapsed);
	AddRow(Content, NewRecordText, 24.f);

	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Buttons"));
	RestartButton = MakeButton(TEXT("RestartButton"), LOCTEXT("Restart", "Rejouer"));
	SettingsButton = MakeButton(TEXT("SettingsButton"), LOCTEXT("Settings", "Réglages"));
	if (UHorizontalBoxSlot* RestartSlot = Buttons->AddChildToHorizontalBox(RestartButton))
	{
		RestartSlot->SetPadding(FMargin(0.f, 0.f, 24.f, 0.f));
	}
	Buttons->AddChildToHorizontalBox(SettingsButton);
	AddRow(Content, Buttons, 0.f);
}

void UGregrEndScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (RestartButton)
	{
		RestartButton->OnClicked.AddUniqueDynamic(this, &UGregrEndScreenWidget::HandleRestartClicked);
	}
	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddUniqueDynamic(this, &UGregrEndScreenWidget::HandleSettingsClicked);
	}
}

void UGregrEndScreenWidget::SetFinalScore(int32 Score)
{
	if (FinalScoreText)
	{
		FinalScoreText->SetText(FText::Format(LOCTEXT("Score", "Score : {0}"), FText::AsNumber(Score)));
	}
}

void UGregrEndScreenWidget::SetNewRecord(bool bNewRecord)
{
	if (NewRecordText)
	{
		NewRecordText->SetVisibility(bNewRecord ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

bool UGregrEndScreenWidget::IsNewRecordShown() const
{
	return NewRecordText && NewRecordText->GetVisibility() != ESlateVisibility::Collapsed;
}

FText UGregrEndScreenWidget::GetScoreDisplay() const
{
	return FinalScoreText ? FinalScoreText->GetText() : FText::GetEmpty();
}

void UGregrEndScreenWidget::HandleRestartClicked()
{
	OnRestartRequested.Broadcast();
}

void UGregrEndScreenWidget::HandleSettingsClicked()
{
	OnSettingsRequested.Broadcast();
}

#undef LOCTEXT_NAMESPACE
