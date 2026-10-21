// GregrTown - UI

#pragma once

#include "CoreMinimal.h"
#include "UI/GregrUserWidget.h"
#include "GregrMonsterHealthWidget.generated.h"

class UProgressBar;

/** Small health bar drawn above a monster's head. */
UCLASS()
class GREGRTOWN_API UGregrMonsterHealthWidget : public UGregrUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Health Bar")
	void SetPercent(float Percent);

	UFUNCTION(BlueprintPure, Category = "Health Bar")
	float GetPercent() const;

protected:
	virtual void BuildDefaultLayout() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> Bar;
};
