#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UBT_DialogueComponent.generated.h"

class UBT_DialogueWidget;
class UShopComponent;
class UQuestOfferComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueOptionManage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueOptionQuest);

/**
 * Drop-in component for any NPC or interactable actor to gain a conditional dialogue menu.
 *
 * On InitiateDialogue():
 *  1. Stops the owner's movement and clears the AI path.
 *  2. Rotates the owner to face the interactor (yaw only).
 *  3. Opens UBT_DialogueWidget with buttons shown based on sibling components:
 *       Talk   — always shown; cycles through TalkLines.
 *       Shop   — shown when a UShopComponent is on the same actor.
 *       Quest  — shown when UQuestOfferComponent has quests available to the player.
 *       Manage — shown when bIsManager is true.
 *       Close  — always shown.
 *  4. Sets FInputModeGameAndUI on the PlayerController.
 *
 * Shop and Quest are handled entirely in C++ (no per-instance Blueprint wiring needed).
 * Manage fires OnManageOptionSelected for the owning actor to handle.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UBT_DialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBT_DialogueComponent();

	/** Call from the owner's Interact_Implementation to start the dialogue sequence. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void InitiateDialogue(AActor* Interactor);

	/** Closes the active widget, restores input mode to GameOnly, and re-enables NPC movement. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void CloseDialogue();

	/** Returns the next line from TalkLines, cycling. Falls back to a generic greeting if empty. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FText GetNextTalkLine();

	/** Returns the PlayerController that most recently called InitiateDialogue. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	APlayerController* GetCachedController() const;

	// ── Sibling-component accessors used by UBT_DialogueWidget ─────────────────

	/** Returns the UShopComponent on the owner, or nullptr if none. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	UShopComponent* GetOwnerShopComponent() const;

	/** Returns the UQuestOfferComponent on the owner, or nullptr if none. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	UQuestOfferComponent* GetOwnerQuestOfferComponent() const;

	// ── Option conditions ──────────────────────────────────────────────────────

	/** True → Manage button shown. Bind OnManageOptionSelected to respond. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Options")
	bool bIsManager = false;

	// ── NPC content ────────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Content")
	FText NpcName = FText::FromString(TEXT("NPC"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Content")
	TArray<FText> TalkLines;

	// ── Widget class ───────────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, Category = "Dialogue|UI")
	TSubclassOf<UBT_DialogueWidget> DialogueWidgetClass;

	// ── Delegates ─────────────────────────────────────────────────────────────

	/**
	 * Fired when the player selects Manage.
	 * Bind on the owning actor to open a management widget.
	 * Call GetCachedController() to get the PlayerController.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueOptionManage OnManageOptionSelected;

	/**
	 * Fired when the player selects Quest (after quests have already been awarded).
	 * Use for custom Blueprint hooks; quests are given automatically by UQuestOfferComponent.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueOptionQuest OnQuestOptionSelected;

private:
	void FaceInteractor(const AActor* Interactor) const;
	void StopOwnerMovement();
	void RestoreOwnerMovement();
	void ApplyInputMode(bool bOpen);

	UPROPERTY()
	TObjectPtr<UBT_DialogueWidget> ActiveWidget;

	TWeakObjectPtr<APlayerController> CachedPC;
	int32 TalkLineIndex = 0;
};
