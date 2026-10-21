// GregrTown - UI

#include "UI/GregrFloatingScoreWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UGregrFloatingScoreWidget::BuildDefaultLayout()
{
	PointsText = MakeText(TEXT("PointsText"), FText::GetEmpty(), 40);
	WidgetTree->RootWidget = PointsText;
}

void UGregrFloatingScoreWidget::Setup(int32 InPoints, const FVector& InWorldLocation, int32 StackIndex)
{
	Points = InPoints;
	WorldLocation = InWorldLocation;
	// Stack simultaneous pops and spread them a little sideways.
	RandomOffset = FVector2D(FMath::FRandRange(-30.f, 30.f), -StackSpacing * StackIndex);

	if (PointsText)
	{
		const bool bBig = Points >= BigPoints;
		PointsText->SetText(FText::FromString(FString::Printf(TEXT("+%d"), Points)));
		PointsText->SetColorAndOpacity(FSlateColor(bBig ? BigColor : FLinearColor::White));
		// Bigger rewards, bigger text.
		const int32 FontSize = FMath::RoundToInt32(FMath::GetMappedRangeValueClamped(FVector2f(10.f, 100.f), FVector2f(30.f, 64.f), static_cast<float>(Points)));
		FSlateFontInfo Font = PointsText->GetFont();
		Font.Size = FontSize;
		PointsText->SetFont(Font);
	}
	SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
}

void UGregrFloatingScoreWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	Age += InDeltaTime;
	const float T = FMath::Clamp(Age / FMath::Max(Lifetime, 0.01f), 0.f, 1.f);
	if (T >= 1.f)
	{
		RemoveFromParent();
		return;
	}

	FVector2D ScreenPosition;
	const bool bOnScreen = GetOwningPlayer() && UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), WorldLocation, ScreenPosition, true);
	SetVisibility(bOnScreen ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	if (bOnScreen)
	{
		const float Rise = RiseDistance * FMath::InterpEaseOut(0.f, 1.f, T, 2.f);
		SetPositionInViewport(ScreenPosition + RandomOffset - FVector2D(0.f, Rise), true);
	}

	// Quick pop, then fade during the last 40%.
	const float Pop = T < 0.12f ? FMath::Lerp(0.6f, 1.25f, T / 0.12f) : FMath::Lerp(1.25f, 1.f, FMath::Min((T - 0.12f) / 0.2f, 1.f));
	SetRenderScale(FVector2D(Pop));
	SetRenderOpacity(T < 0.6f ? 1.f : 1.f - (T - 0.6f) / 0.4f);
}
