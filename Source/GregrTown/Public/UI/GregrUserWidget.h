// GregrTown - UI

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GregrUserWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UWidget;

/**
 * Base of GregrTown widgets. Widgets declare their parts as BindWidgetOptional:
 * a Widget Blueprint child can provide its own layout, otherwise BuildDefaultLayout()
 * creates a simple one in C++, so the game works without any UI asset.
 */
UCLASS(Abstract)
class GREGRTOWN_API UGregrUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

protected:
	/** Creates the default widget tree when the class has no designer layout. */
	virtual void BuildDefaultLayout() {}

	UTextBlock* MakeText(FName Name, const FText& Text, int32 FontSize, bool bBold = true);
	UButton* MakeButton(FName Name, const FText& Label, int32 FontSize = 32);

	/** Sets the root to a dimmed full-screen window with a centered panel, returns the panel content box. */
	UVerticalBox* MakeModalPanel(float PanelWidth = 760.f);

	/** Adds a child to a vertical box, centered horizontally, with some space below. */
	static void AddRow(UVerticalBox* Box, UWidget* Child, float BottomPadding = 24.f, bool bFill = false);
};
