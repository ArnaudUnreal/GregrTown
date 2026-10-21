// GregrTown - UI

#include "UI/GregrUserWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

bool UGregrUserWidget::Initialize()
{
	const bool bSuperInitialized = Super::Initialize();
	if (bSuperInitialized && WidgetTree && !WidgetTree->RootWidget)
	{
		BuildDefaultLayout();
	}
	return bSuperInitialized;
}

UTextBlock* UGregrUserWidget::MakeText(FName Name, const FText& Text, int32 FontSize, bool bBold)
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	TextBlock->SetText(Text);
	TextBlock->SetFont(FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", FontSize));
	TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	TextBlock->SetShadowOffset(FVector2D(2.f, 2.f));
	TextBlock->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.75f));
	TextBlock->SetJustification(ETextJustify::Center);
	return TextBlock;
}

UVerticalBox* UGregrUserWidget::MakeModalPanel(float PanelWidth)
{
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	UBorder* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Dim"));
	Dim->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.55f));
	if (UOverlaySlot* DimSlot = Root->AddChildToOverlay(Dim))
	{
		DimSlot->SetHorizontalAlignment(HAlign_Fill);
		DimSlot->SetVerticalAlignment(VAlign_Fill);
	}

	USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("PanelSize"));
	Size->SetWidthOverride(PanelWidth);
	if (UOverlaySlot* SizeSlot = Root->AddChildToOverlay(Size))
	{
		SizeSlot->SetHorizontalAlignment(HAlign_Center);
		SizeSlot->SetVerticalAlignment(VAlign_Center);
	}

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Panel"));
	Panel->SetBrushColor(FLinearColor(0.08f, 0.12f, 0.25f, 0.92f));
	Panel->SetPadding(FMargin(48.f, 36.f));
	Size->AddChild(Panel);

	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Content"));
	Panel->AddChild(Content);
	return Content;
}

void UGregrUserWidget::AddRow(UVerticalBox* Box, UWidget* Child, float BottomPadding, bool bFill)
{
	if (UVerticalBoxSlot* RowSlot = Box->AddChildToVerticalBox(Child))
	{
		RowSlot->SetHorizontalAlignment(bFill ? HAlign_Fill : HAlign_Center);
		RowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, BottomPadding));
	}
}

UButton* UGregrUserWidget::MakeButton(FName Name, const FText& Label, int32 FontSize)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	UTextBlock* LabelText = MakeText(*(Name.ToString() + TEXT("Label")), Label, FontSize);
	LabelText->SetColorAndOpacity(FSlateColor(FLinearColor(0.05f, 0.05f, 0.05f)));
	LabelText->SetShadowOffset(FVector2D::ZeroVector);
	if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(Button->AddChild(LabelText)))
	{
		ButtonSlot->SetPadding(FMargin(32.f, 12.f));
	}
	return Button;
}
