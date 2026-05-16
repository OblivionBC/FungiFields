#pragma once

#include "CoreMinimal.h"
#include "UBackpackWidget.h"
#include "UPlayerInventoryWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerInventoryClosed);

/**
 * Player-facing inventory widget. Extends UBackpackWidget with a close button,
 * Escape/Tab key handling, and a closed delegate.
 * Re-parent WBP_BackPack to this class and add a Button named "CloseButton".
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UPlayerInventoryWidget : public UBackpackWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseInventory();

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnPlayerInventoryClosed OnInventoryClosed;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION()
	void OnCloseButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CloseButton;
};
