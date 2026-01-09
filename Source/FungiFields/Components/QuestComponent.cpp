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

	if (UInventoryComponent* Comp = Owner->FindComponentByClass<UInventoryComponent>())
	{
		Comp->OnItemAdded.AddDynamic(this, &UQuestComponent::OnItemAdded);
	}

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

	if (UInventoryComponent* Comp = Owner->FindComponentByClass<UInventoryComponent>())
	{
		Comp->OnItemAdded.RemoveDynamic(this, &UQuestComponent::OnItemAdded);
	}

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
	for (auto& QuestPair : ActiveQuests)
	{
		UQuest* Quest = QuestPair.Value;
		if (Quest && Quest->State == EQuestState::InProgress)
		{
			if (Quest->ShouldRespondToCropHarvested(CropData, Quantity))
			{
				Quest->AddProgress(Quantity);
			}
		}
	}
}

void UQuestComponent::OnSeedPlanted(AActor* Planter, USeedDataAsset* SeedData)
{
	for (auto& QuestPair : ActiveQuests)
	{
		UQuest* Quest = QuestPair.Value;
		if (Quest && Quest->State == EQuestState::InProgress)
		{
			if (Quest->ShouldRespondToSeedPlanted(SeedData))
			{
				Quest->AddProgress(1);
			}
		}
	}
}

void UQuestComponent::OnSoilTilled(AActor* Tiller)
{
	for (auto& QuestPair : ActiveQuests)
	{
		UQuest* Quest = QuestPair.Value;
		if (Quest && Quest->State == EQuestState::InProgress)
		{
			if (Quest->QuestEventType == EQuestEventType::SoilTilled)
			{
				Quest->AddProgress(1);
			}
		}
	}
}

void UQuestComponent::OnSoilWatered(AActor* Waterer, AActor* SoilPlot)
{
	for (auto& QuestPair : ActiveQuests)
	{
		UQuest* Quest = QuestPair.Value;
		if (Quest && Quest->State == EQuestState::InProgress)
		{
			if (Quest->QuestEventType == EQuestEventType::SoilWatered)
			{
				Quest->AddProgress(1);
			}
		}
	}
}

void UQuestComponent::OnItemAdded(UItemDataAsset* Item, int32 Amount, int32 NewTotal)
{
	for (auto& QuestPair : ActiveQuests)
	{
		UQuest* Quest = QuestPair.Value;
		if (Quest && Quest->State == EQuestState::InProgress)
		{
			if (Quest->ShouldRespondToItemAdded(Item, Amount))
			{
				Quest->AddProgress(Amount);
			}
		}
	}
}


