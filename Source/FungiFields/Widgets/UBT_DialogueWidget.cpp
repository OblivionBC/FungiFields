#include "UBT_DialogueWidget.h"
#include "../Components/UBT_DialogueComponent.h"
#include "../Components/UShopComponent.h"
#include "../Components/UQuestOfferComponent.h"
#include "../Components/QuestComponent.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

void UBT_DialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TalkButton)   TalkButton->OnClicked.AddDynamic(this, &UBT_DialogueWidget::OnTalkButtonClicked);
	if (ShopButton)   ShopButton->OnClicked.AddDynamic(this, &UBT_DialogueWidget::OnShopButtonClicked);
	if (QuestButton)  QuestButton->OnClicked.AddDynamic(this, &UBT_DialogueWidget::OnQuestButtonClicked);
	if (ManageButton) ManageButton->OnClicked.AddDynamic(this, &UBT_DialogueWidget::OnManageButtonClicked);
	if (CloseButton)  CloseButton->OnClicked.AddDynamic(this, &UBT_DialogueWidget::OnCloseButtonClicked);
}

void UBT_DialogueWidget::NativeDestruct()
{
	if (DialogueComp)
	{
		UBT_DialogueComponent* Comp = DialogueComp;
		DialogueComp = nullptr;
		Comp->CloseDialogue();
	}
	Super::NativeDestruct();
}

void UBT_DialogueWidget::SetDialogueComponent(UBT_DialogueComponent* InComponent)
{
	DialogueComp = InComponent;
	if (!InComponent)
		return;

	if (NpcNameText)
		NpcNameText->SetText(InComponent->NpcName);

	if (DialogueText)
		DialogueText->SetText(InComponent->GetNextTalkLine());

	const ESlateVisibility Shown  = ESlateVisibility::Visible;
	const ESlateVisibility Hidden = ESlateVisibility::Collapsed;

	// Shop button — shown when the owner has a UShopComponent.
	if (ShopButton)
		ShopButton->SetVisibility(InComponent->GetOwnerShopComponent() ? Shown : Hidden);

	// Manage button
	if (ManageButton)
		ManageButton->SetVisibility(InComponent->bIsManager ? Shown : Hidden);

	// Quest button — shown when UQuestOfferComponent has quests available to this player.
	if (QuestButton)
	{
		UQuestOfferComponent* QuestOfferComp = InComponent->GetOwnerQuestOfferComponent();
		bool bQuestAvailable = false;
		if (QuestOfferComp)
		{
			UQuestComponent* PlayerQuestComp = nullptr;
			if (APlayerController* PC = InComponent->GetCachedController())
			{
				if (APawn* Pawn = PC->GetPawn())
					PlayerQuestComp = Pawn->FindComponentByClass<UQuestComponent>();
			}
			bQuestAvailable = QuestOfferComp->HasAvailableQuests(PlayerQuestComp);
		}
		QuestButton->SetVisibility(bQuestAvailable ? Shown : Hidden);
	}
}

// ── Button handlers ───────────────────────────────────────────────────────────

void UBT_DialogueWidget::OnTalkButtonClicked()
{
	if (!DialogueComp || !DialogueText) return;
	DialogueText->SetText(DialogueComp->GetNextTalkLine());
}

void UBT_DialogueWidget::OnShopButtonClicked()
{
	if (!DialogueComp) return;

	UShopComponent* ShopComp  = DialogueComp->GetOwnerShopComponent();
	APlayerController* PC     = DialogueComp->GetCachedController();
	APawn* PlayerPawn          = PC ? PC->GetPawn() : nullptr;

	// Cache before CloseDialogue resets CachedPC.
	UBT_DialogueComponent* Comp = DialogueComp;
	DialogueComp = nullptr;
	Comp->CloseDialogue();

	if (PlayerPawn && ShopComp)
		ShopComp->OpenShopWidget(PlayerPawn);
}

void UBT_DialogueWidget::OnQuestButtonClicked()
{
	if (!DialogueComp) return;

	APlayerController* PC = DialogueComp->GetCachedController();
	APawn* PlayerPawn      = PC ? PC->GetPawn() : nullptr;

	UQuestOfferComponent* QuestOfferComp = DialogueComp->GetOwnerQuestOfferComponent();

	if (PlayerPawn && QuestOfferComp)
	{
		UQuestComponent* QuestComp = PlayerPawn->FindComponentByClass<UQuestComponent>();
		if (QuestComp)
		{
			QuestOfferComp->GiveAvailableQuests(QuestComp);

			// Show acceptance confirmation in the dialogue text.
			if (DialogueText)
				DialogueText->SetText(QuestOfferComp->QuestGivenLine);
		}
	}

	// Hide the quest button — there's nothing left to accept.
	if (QuestButton)
		QuestButton->SetVisibility(ESlateVisibility::Collapsed);

	// Fire delegate for any custom Blueprint hooks.
	DialogueComp->OnQuestOptionSelected.Broadcast();
}

void UBT_DialogueWidget::OnManageButtonClicked()
{
	if (!DialogueComp) return;

	UBT_DialogueComponent* Comp = DialogueComp;
	DialogueComp = nullptr;
	Comp->CloseDialogue();
	Comp->OnManageOptionSelected.Broadcast();
}

void UBT_DialogueWidget::OnCloseButtonClicked()
{
	if (DialogueComp)
		DialogueComp->CloseDialogue();
}
