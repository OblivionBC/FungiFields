#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UQuestOfferComponent.generated.h"

class UQuest;
class UQuestComponent;

/**
 * Attach to any actor to give it quests to offer.
 * UBT_DialogueComponent on the same actor automatically surfaces the Quest button when at
 * least one quest is available (prerequisites met and not yet accepted by the player).
 * Quests are awarded directly through dialogue — no separate quest giver UI is needed.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UQuestOfferComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestOfferComponent();

	/**
	 * Returns quests that:
	 *  - Have all PrerequisiteQuests completed by the player.
	 *  - Have not yet been accepted (not in the player's quest log at all).
	 * Pass nullptr to skip filtering — returns every quest in QuestsToOffer.
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Offer")
	TArray<UQuest*> GetAvailableQuests(UQuestComponent* PlayerQuestComp) const;

	/** Returns true if at least one quest is available to the given player. */
	UFUNCTION(BlueprintPure, Category = "Quest Offer")
	bool HasAvailableQuests(UQuestComponent* PlayerQuestComp) const;

	/**
	 * Adds all available quests to the player's UQuestComponent.
	 * @return Number of quests newly added.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Offer")
	int32 GiveAvailableQuests(UQuestComponent* PlayerQuestComp);

	// ── Editor config ──────────────────────────────────────────────────────────

	/** Quests this actor can offer. Each UQuest already has its own PrerequisiteQuests field. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Offer")
	TArray<TObjectPtr<UQuest>> QuestsToOffer;

	/** Dialogue line displayed after the player accepts quests. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Offer")
	FText QuestGivenLine = NSLOCTEXT("QuestOffer", "DefaultAccepted", "I hope you complete it. Good luck!");
};
