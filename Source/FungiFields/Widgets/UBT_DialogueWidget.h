#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UBT_DialogueWidget.generated.h"

class UBT_DialogueComponent;
class UTextBlock;
class UButton;

/**
 * Dynamic dialogue widget driven by UBT_DialogueComponent.
 *
 * Buttons are conditionally shown or collapsed based on the component's configuration:
 *   TalkButton   — always visible; cycles through the NPC's TalkLines.
 *   ShopButton   — visible only when LinkedShopActor is set on the component.
 *   QuestButton  — visible only when QuestsToOffer is non-empty on the component.
 *   ManageButton — visible only when bIsManager is true on the component.
 *   CloseButton  — always visible; calls CloseDialogue on the component.
 *
 * Abstract — create WBP_BT_Dialogue as a Blueprint child and bind all named widgets.
 */
UCLASS(Abstract)
class FUNGIFIELDS_API UBT_DialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Binds the component and applies conditional button visibility.
	 * Call immediately after CreateWidget, before AddToViewport.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue Widget")
	void SetDialogueComponent(UBT_DialogueComponent* InComponent);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION() void OnTalkButtonClicked();
	UFUNCTION() void OnShopButtonClicked();
	UFUNCTION() void OnQuestButtonClicked();
	UFUNCTION() void OnManageButtonClicked();
	UFUNCTION() void OnCloseButtonClicked();

	/** Displays the NPC's name. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NpcNameText;

	/** Shows the current talk line or a default greeting. Updated each time Talk is clicked. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DialogueText;

	/** Always shown. Each click advances to the next line in TalkLines. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TalkButton;

	/** Collapsed unless LinkedShopActor is set. Opens the shop and closes this widget. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ShopButton;

	/** Collapsed unless QuestsToOffer is non-empty. Fires OnQuestOptionSelected. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuestButton;

	/** Collapsed unless bIsManager is true. Fires OnManageOptionSelected. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ManageButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

private:
	UPROPERTY()
	TObjectPtr<UBT_DialogueComponent> DialogueComp;
};
