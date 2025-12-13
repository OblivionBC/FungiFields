#include "QuestComponent.h"
#include "../Data/Quest.h"
#include "../Data/UCropDataAsset.h"
#include "../Data/USeedDataAsset.h"
#include "../Data/UItemDataAsset.h"
#include "../ENUM/EQuestEventType.h"
#include "InventoryComponent.h"
#include "UFarmingComponent.h"

UQuestComponent::UQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();
	SubscribeToComponentEvents();
}

void UQuestComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnsubscribeFromComponentEvents();
	Super::EndPlay(EndPlayReason);
}

UQuest* UQuestComponent::AddQuest(UQuest* Quest)
{
	if (!Quest)
		return nullptr;

	ActiveQuests.Add(Quest->QuestID, Quest);

	Quest->StartQuest();

	return Quest;
}

TArray<UQuest*> UQuestComponent::GetAllQuests() const
{
	TArray<UQuest*> Values;
	ActiveQuests.GenerateValueArray(Values);
	return Values;
}

bool UQuestComponent::RemoveQuest(FName QuestID)
{
	return ActiveQuests.Remove(QuestID) > 0;
}

void UQuestComponent::SubscribeToComponentEvents()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Subscribe to owner's InventoryComponent
	if (UInventoryComponent* Comp = Owner->FindComponentByClass<UInventoryComponent>())
	{
		Comp->OnItemAdded.AddDynamic(this, &UQuestComponent::OnItemAdded);
	}

	// Subscribe to owner's UFarmingComponent
	if (UFarmingComponent* Comp = Owner->FindComponentByClass<UFarmingComponent>())
	{
		Comp->OnCropHarvested.AddDynamic(this, &UQuestComponent::OnCropHarvested);
		Comp->OnSeedPlanted.AddDynamic(this, &UQuestComponent::OnSeedPlanted);
		Comp->OnSoilTilled.AddDynamic(this, &UQuestComponent::OnSoilTilled);
		Comp->OnSoilWatered.AddDynamic(this, &UQuestComponent::OnSoilWatered);
	}
}

void UQuestComponent::UnsubscribeFromComponentEvents()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Unsubscribe from owner's InventoryComponent
	if (UInventoryComponent* Comp = Owner->FindComponentByClass<UInventoryComponent>())
	{
		Comp->OnItemAdded.RemoveDynamic(this, &UQuestComponent::OnItemAdded);
	}

	// Unsubscribe from owner's UFarmingComponent
	if (UFarmingComponent* Comp = Owner->FindComponentByClass<UFarmingComponent>())
	{
		Comp->OnCropHarvested.RemoveDynamic(this, &UQuestComponent::OnCropHarvested);
		Comp->OnSeedPlanted.RemoveDynamic(this, &UQuestComponent::OnSeedPlanted);
		Comp->OnSoilTilled.RemoveDynamic(this, &UQuestComponent::OnSoilTilled);
		Comp->OnSoilWatered.RemoveDynamic(this, &UQuestComponent::OnSoilWatered);
	}
}

void UQuestComponent::OnCropHarvested(AActor* Harvester, UCropDataAsset* CropData, int32 Quantity)
{
	// No owner filtering needed - we're directly subscribed to our owner's component
	// Iterate through active quests and update progress
	for (auto& QuestPair : ActiveQuests)
	{
		UQuest* Quest = QuestPair.Value;
		if (Quest && Quest->State == EQuestState::InProgress)
		{
			// Check if this quest should respond to this event
			if (Quest->ShouldRespondToCropHarvested(CropData, Quantity))
			{
				Quest->AddProgress(Quantity);
			}
		}
	}
}

void UQuestComponent::OnSeedPlanted(AActor* Planter, USeedDataAsset* SeedData)
{
	// No owner filtering needed - we're directly subscribed to our owner's component
	// Iterate through active quests and update progress
	for (auto& QuestPair : ActiveQuests)
	{
		UQuest* Quest = QuestPair.Value;
		if (Quest && Quest->State == EQuestState::InProgress)
		{
			// Check if this quest should respond to this event
			if (Quest->ShouldRespondToSeedPlanted(SeedData))
			{
				Quest->AddProgress(1);
			}
		}
	}
}

void UQuestComponent::OnSoilTilled(AActor* Tiller)
{
	// No owner filtering needed - we're directly subscribed to our owner's component
	// Iterate through active quests and update progress
	for (auto& QuestPair : ActiveQuests)
	{
		UQuest* Quest = QuestPair.Value;
		if (Quest && Quest->State == EQuestState::InProgress)
		{
			// Check if this quest listens to soil tilled events
			if (Quest->QuestEventType == EQuestEventType::SoilTilled)
			{
				Quest->AddProgress(1);
			}
		}
	}
}

void UQuestComponent::OnSoilWatered(AActor* Waterer, AActor* SoilPlot)
{
	// No owner filtering needed - we're directly subscribed to our owner's component
	// Iterate through active quests and update progress
	for (auto& QuestPair : ActiveQuests)
	{
		UQuest* Quest = QuestPair.Value;
		if (Quest && Quest->State == EQuestState::InProgress)
		{
			// Check if this quest listens to soil watered events
			if (Quest->QuestEventType == EQuestEventType::SoilWatered)
			{
				Quest->AddProgress(1);
			}
		}
	}
}

void UQuestComponent::OnItemAdded(UItemDataAsset* Item, int32 Amount, int32 NewTotal)
{
	// No owner filtering needed - we're directly subscribed to our owner's component
	// Iterate through active quests and update progress
	for (auto& QuestPair : ActiveQuests)
	{
		UQuest* Quest = QuestPair.Value;
		if (Quest && Quest->State == EQuestState::InProgress)
		{
			// Check if this quest should respond to this event
			if (Quest->ShouldRespondToItemAdded(Item, Amount))
			{
				Quest->AddProgress(Amount);
			}
		}
	}
}


