// GregrTown - UI

#include "UI/GregrMonsterHealthWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"

void UGregrMonsterHealthWidget::BuildDefaultLayout()
{
	Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("Bar"));
	Bar->SetFillColorAndOpacity(FLinearColor(0.9f, 0.1f, 0.05f));
	FProgressBarStyle Style = Bar->GetWidgetStyle();
	Style.BackgroundImage.TintColor = FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.7f));
	Bar->SetWidgetStyle(Style);
	Bar->SetPercent(1.f);
	WidgetTree->RootWidget = Bar;
}

void UGregrMonsterHealthWidget::SetPercent(float Percent)
{
	if (Bar)
	{
		Bar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
}

float UGregrMonsterHealthWidget::GetPercent() const
{
	return Bar ? Bar->GetPercent() : 0.f;
}
