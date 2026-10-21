// GregrTown - UI

#include "UI/GregrSettingsWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Game/GregrGameUserSettings.h"
#include "UI/GregrHUDWidget.h"

#define LOCTEXT_NAMESPACE "GregrSettings"

void UGregrSettingsWidget::BuildDefaultLayout()
{
	UVerticalBox* Content = MakeModalPanel();

	AddRow(Content, MakeText(TEXT("Title"), LOCTEXT("Title", "Réglages"), 56), 36.f);

	DurationLabel = MakeText(TEXT("DurationLabel"), FText::GetEmpty(), 30);
	AddRow(Content, DurationLabel, 8.f);
	DurationSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("DurationSlider"));
	AddRow(Content, DurationSlider, 8.f, true);
	AddRow(Content, MakeText(TEXT("DurationHint"), LOCTEXT("DurationHint", "(pour la prochaine partie)"), 18, false), 28.f);

	VolumeLabel = MakeText(TEXT("VolumeLabel"), FText::GetEmpty(), 30);
	AddRow(Content, VolumeLabel, 8.f);
	VolumeSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("VolumeSlider"));
	AddRow(Content, VolumeSlider, 40.f, true);

	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Buttons"));
	ResumeButton = MakeButton(TEXT("ResumeButton"), LOCTEXT("Resume", "Continuer"));
	RestartButton = MakeButton(TEXT("RestartButton"), LOCTEXT("Restart", "Rejouer"));
	if (UHorizontalBoxSlot* ResumeSlot = Buttons->AddChildToHorizontalBox(ResumeButton))
	{
		ResumeSlot->SetPadding(FMargin(0.f, 0.f, 24.f, 0.f));
	}
	Buttons->AddChildToHorizontalBox(RestartButton);
	AddRow(Content, Buttons, 0.f);
}

void UGregrSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DurationSlider)
	{
		DurationSlider->SetMinValue(UGregrGameUserSettings::MinChallengeDuration);
		DurationSlider->SetMaxValue(UGregrGameUserSettings::MaxChallengeDuration);
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
	}
	if (RestartButton)
	{
		RestartButton->OnClicked.AddUniqueDynamic(this, &UGregrSettingsWidget::HandleRestartClicked);
	}

	RefreshFromSettings();
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
		DurationLabel->SetText(FText::Format(LOCTEXT("Duration", "Durée du défi : {0}"),
			UGregrHUDWidget::FormatTime(FMath::RoundToInt32(Settings->GetChallengeDuration()))));
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

#undef LOCTEXT_NAMESPACE
