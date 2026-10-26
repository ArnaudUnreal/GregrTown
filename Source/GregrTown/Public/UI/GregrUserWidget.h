// GregrTown - UI

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GregrUserWidget.generated.h"

class APlayerController;
class UButton;
class UTextBlock;
class UVerticalBox;
class UWidget;

/**
 * Base of GregrTown widgets. Widgets declare their parts as BindWidgetOptional:
 * a Widget Blueprint child can provide its own layout, otherwise BuildDefaultLayout()
 * creates a simple one in C++, so the game works without any UI asset.
 * Menus (widgets with a desired focus widget) keep the gamepad focus on their buttons and sliders
 * and highlight the focused one.
 */
UCLASS(Abstract)
class GREGRTOWN_API UGregrUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	/** Tests: the button or slider showing the focus highlight (null if none). */
	UFUNCTION(BlueprintPure, Category = "Menu")
	UWidget* GetHighlightedWidget() const { return HighlightedWidget.Get(); }

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** False while another menu is on top of this one (it keeps the focus then). */
	virtual bool WantsMenuFocus() const { return true; }

	/** Tint of the focused button / slider handle. */
	UPROPERTY(EditDefaultsOnly, Category = "Menu")
	FLinearColor FocusColor = FLinearColor(1.f, 0.6f, 0.05f);

	/** Creates the default widget tree when the class has no designer layout. */
	virtual void BuildDefaultLayout() {}

	UTextBlock* MakeText(FName Name, const FText& Text, int32 FontSize, bool bBold = true);
	UButton* MakeButton(FName Name, const FText& Label, int32 FontSize = 32);

	/** Sets the root to a dimmed full-screen window with a centered panel, returns the panel content box. */
	UVerticalBox* MakeModalPanel(float PanelWidth = 760.f);

	/** Adds a child to a vertical box, centered horizontally, with some space below. */
	static void AddRow(UVerticalBox* Box, UWidget* Child, float BottomPadding = 24.f, bool bFill = false);

private:
	void UpdateFocusHighlight(APlayerController* PC);
	void SetHighlighted(UWidget* Widget, bool bHighlighted);

	TWeakObjectPtr<UWidget> HighlightedWidget;
	/** Color of the highlighted widget before the focus tint. */
	FLinearColor HighlightedColor = FLinearColor::White;
};
