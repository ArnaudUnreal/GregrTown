// GregrTown - UI

#include "UI/GregrSettingsWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Game/GregrGameUserSettings.h"
#include "UI/GregrHUDWidget.h"

#define LOCTEXT_NAMESPACE "GregrSettings"

void UGregrSettingsWidget::BuildDefaultLayout()
{
	UVerticalBox* Content = MakeModalPanel(880.f);

	AddRow(Content, MakeText(TEXT("Title"), LOCTEXT("Title", "Réglages"), 56), 36.f);

	DurationLabel = MakeText(TEXT("DurationLabel"), FText::GetEmpty(), 30);
	AddRow(Content, DurationLabel, 8.f);
	DurationSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("DurationSlider"));
	// Gamepad: left / right change the value right away (no "press A to edit" step).
	DurationSlider->RequiresControllerLock = false;
	AddRow(Content, DurationSlider, 8.f, true);
	DurationHint = MakeText(TEXT("DurationHint"), LOCTEXT("DurationHint", "(pour la prochaine partie)"), 18, false);
	AddRow(Content, DurationHint, 28.f);

	VolumeLabel = MakeText(TEXT("VolumeLabel"), FText::GetEmpty(), 30);
	AddRow(Content, VolumeLabel, 8.f);
	VolumeSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("VolumeSlider"));
	VolumeSlider->RequiresControllerLock = false;
	AddRow(Content, VolumeSlider, 40.f, true);

	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Buttons"));
	ResumeButton = MakeButton(TEXT("ResumeButton"), LOCTEXT("Resume", "Continuer"));
	ResumeButtonLabel = Cast<UTextBlock>(ResumeButton->GetChildAt(0));
	RestartButton = MakeButton(TEXT("RestartButton"), LOCTEXT("Restart", "Rejouer"));
	QuitButton = MakeButton(TEXT("QuitButton"), LOCTEXT("Quit", "Quitter"));
	for (UButton* Button : { ResumeButton.Get(), RestartButton.Get(), QuitButton.Get() })
	{
		if (UHorizontalBoxSlot* ButtonSlot = Buttons->AddChildToHorizontalBox(Button))
		{
			ButtonSlot->SetPadding(FMargin(12.f, 0.f));
		}
	}
	AddRow(Content, Buttons, 0.f);
}

void UGregrSettingsWidget::SetStartMenu(bool bStartMenu)
{
	bIsStartMenu = bStartMenu;
	if (ResumeButtonLabel)
	{
		ResumeButtonLabel->SetText(bStartMenu ? LOCTEXT("Play", "Jouer") : LOCTEXT("Resume", "Continuer"));
	}
	// Collapsed rather than hidden: the remaining buttons stay centered.
	if (RestartButton)
	{
		RestartButton->SetVisibility(bStartMenu ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	if (DurationHint)
	{
		DurationHint->SetVisibility(bStartMenu ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}

FText UGregrSettingsWidget::GetDurationDisplay() const
{
	return DurationLabel ? DurationLabel->GetText() : FText::GetEmpty();
}

FText UGregrSettingsWidget::GetResumeLabel() const
{
	return ResumeButtonLabel ? ResumeButtonLabel->GetText() : FText::GetEmpty();
}

void UGregrSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DurationSlider)
	{
		DurationSlider->SetMinValue(UGregrGameUserSettings::MinChallengeDuration);
		// The last notch (after the max duration) is "unlimited".
		DurationSlider->SetMaxValue(UGregrGameUserSettings::UnlimitedChallengeDuration);
		DurationSlider->SetStepSize(30.f);
		DurationSlider->MouseUsesStep = true;
		DurationSlider->OnValueChanged.AddUniqueDynamic(this, &UGregrSettingsWidget::HandleDurationChanged);
	}
	if (VolumeSlider)
	{
		VolumeSlider->SetMinValue(0.f);
		VolumeSlider->SetMaxValue(1.f);
		VolumeSlider->SetStepSize(0.05f);
		VolumeSlider->MouseUsesStep = true;
		VolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UGregrSettingsWidget::HandleVolumeChanged);
	}
	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddUniqueDynamic(this, &UGregrSettingsWidget::HandleResumeClicked);
		if (!GetDesiredFocusWidget())
		{
			SetDesiredFocusWidget(ResumeButton);
		}
	}
	if (RestartButton)
	{
		RestartButton->OnClicked.AddUniqueDynamic(this, &UGregrSettingsWidget::HandleRestartClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddUniqueDynamic(this, &UGregrSettingsWidget::HandleQuitClicked);
	}

	RefreshFromSettings();
}

FReply UGregrSettingsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Gamepad B / Escape closes the pause menu like "Resume" (the start menu waits for "Play").
	if (!bIsStartMenu && FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Back)
	{
		HandleResumeClicked();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UGregrSettingsWidget::RefreshFromSettings()
{
	if (const UGregrGameUserSettings* Settings = UGregrGameUserSettings::GetGregrUserSettings())
	{
		if (DurationSlider)
		{
			DurationSlider->SetValue(Settings->GetChallengeDuration());
		}
		if (VolumeSlider)
		{
			VolumeSlider->SetValue(Settings->GetMasterVolume());
		}
	}
	UpdateLabels();
}

void UGregrSettingsWidget::SetDurationFromUI(float Seconds)
{
	if (DurationSlider)
	{
		DurationSlider->SetValue(Seconds);
	}
	HandleDurationChanged(Seconds);
}

void UGregrSettingsWidget::SetVolumeFromUI(float Volume)
{
	if (VolumeSlider)
	{
		VolumeSlider->SetValue(Volume);
	}
	HandleVolumeChanged(Volume);
}

void UGregrSettingsWidget::HandleDurationChanged(float Value)
{
	if (UGregrGameUserSettings* Settings = UGregrGameUserSettings::GetGregrUserSettings())
	{
		// Snap to the slider steps so keyboard/gamepad and mouse give the same values.
		Settings->SetChallengeDuration(FMath::GridSnap(Value, 30.f));
		Settings->SaveSettings();
	}
	UpdateLabels();
}

void UGregrSettingsWidget::HandleVolumeChanged(float Value)
{
	if (UGregrGameUserSettings* Settings = UGregrGameUserSettings::GetGregrUserSettings())
	{
		Settings->SetMasterVolume(Value);
		Settings->ApplyAudio(this);
		Settings->SaveSettings();
	}
	UpdateLabels();
}

void UGregrSettingsWidget::UpdateLabels()
{
	const UGregrGameUserSettings* Settings = UGregrGameUserSettings::GetGregrUserSettings();
	if (!Settings)
	{
		return;
	}
	if (DurationLabel)
	{
		DurationLabel->SetText(FText::Format(LOCTEXT("Duration", "Durée du défi : {0}"), Settings->IsChallengeUnlimited()
			? LOCTEXT("Unlimited", "illimitée")
			: UGregrHUDWidget::FormatTime(FMath::RoundToInt32(Settings->GetChallengeDuration()))));
	}
	if (VolumeLabel)
	{
		VolumeLabel->SetText(FText::Format(LOCTEXT("Volume", "Volume : {0} %"),
			FText::AsNumber(FMath::RoundToInt32(Settings->GetMasterVolume() * 100.f))));
	}
}

void UGregrSettingsWidget::HandleResumeClicked()
{
	OnResumeRequested.Broadcast();
}

void UGregrSettingsWidget::HandleRestartClicked()
{
	OnRestartRequested.Broadcast();
}

void UGregrSettingsWidget::HandleQuitClicked()
{
	OnQuitRequested.Broadcast();
}

#undef LOCTEXT_NAMESPACE
