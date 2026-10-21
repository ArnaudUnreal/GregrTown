// GregrTown - UI

#pragma once

#include "CoreMinimal.h"
#include "UI/GregrUserWidget.h"
#include "GregrFloatingScoreWidget.generated.h"

class UTextBlock;

/** "+49" popping where points were earned, rising and fading out, then removing itself. */
UCLASS()
class GREGRTOWN_API UGregrFloatingScoreWidget : public UGregrUserWidget
{
	GENERATED_BODY()

public:
	/** StackIndex lifts the text when several pop at once, so they don't overlap. */
	void Setup(int32 InPoints, const FVector& InWorldLocation, int32 StackIndex = 0);

	/** Vertical pixels between stacked pops. */
	UPROPERTY(EditDefaultsOnly, Category = "Floating Score")
	float StackSpacing = 52.f;

	UFUNCTION(BlueprintPure, Category = "Floating Score")
	int32 GetPoints() const { return Points; }

	/** Seconds on screen. */
	UPROPERTY(EditDefaultsOnly, Category = "Floating Score")
	float Lifetime = 1.2f;

	/** Pixels travelled upwards during the lifetime. */
	UPROPERTY(EditDefaultsOnly, Category = "Floating Score")
	float RiseDistance = 140.f;

	/** Color of big rewards (>= BigPoints). */
	UPROPERTY(EditDefaultsOnly, Category = "Floating Score")
	FLinearColor BigColor = FLinearColor(1.f, 0.78f, 0.1f);

	UPROPERTY(EditDefaultsOnly, Category = "Floating Score")
	int32 BigPoints = 70;

protected:
	virtual void BuildDefaultLayout() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PointsText;

	FVector WorldLocation = FVector::ZeroVector;
	FVector2D RandomOffset = FVector2D::ZeroVector;
	int32 Points = 0;
	float Age = 0.f;
};
