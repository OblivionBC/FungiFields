#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UItemTooltipWidget.generated.h"

class UTextBlock;
class UVerticalBox;

/**
 * Custom tooltip widget for inventory items. Displays item name and description
 * with configurable font sizes so tooltips are more readable than the default.
 */
UCLASS()
class FUNGIFIELDS_API UItemTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set the tooltip content. Description can be empty. */
	UFUNCTION(BlueprintCallable, Category = "Item Tooltip")
	void SetContent(const FText& Name, const FText& Description);

	/** Font size for the item name (default 24). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Tooltip", meta = (ClampMin = "8", ClampMax = "48"))
	int32 NameFontSize = 24;

	/** Font size for the item description (default 20). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Tooltip", meta = (ClampMin = "8", ClampMax = "48"))
	int32 DescriptionFontSize = 20;

protected:
	virtual void NativeConstruct() override;

	void BuildWidgetStructure();

	UPROPERTY()
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY()
	TObjectPtr<UTextBlock> DescriptionText;
};
