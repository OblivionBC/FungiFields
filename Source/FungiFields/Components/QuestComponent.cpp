#include "QuestComponent.h"
#include "../Data/UCropDataAsset.h"
#include "../Data/USeedDataAsset.h"
#include "../Data/UItemDataAsset.h"
#include "../ENUM/EQuestEventType.h"
#include "InventoryComponent.h"
#include "UFarmingComponent.h"
#include "../Subsystems/UCropManagerSubsystem.h"
#include "../Interfaces/IHarvestableInterface.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "FungiFields/Attributes/EconomyAttributeSet.h"
#include "FungiFields/Attributes/LevelAttributeSet.h"

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

	if (QuestDefinitions.Contains(Quest->QuestID))
		return QuestDefinitions[Quest->QuestID].Get();

	QuestDefinitions.Add(Quest->QuestID, Quest);

	FQuestProgress& Progress = QuestProgress.Add(Quest->QuestID, FQuestProgress{Quest->QuestID});
	Quest->StartQuest(Progress);

	OnQuestsUpdated.Broadcast();
	return Quest;
}

TArray<UQuest*> UQuestComponent::GetAllQuestDefinitions() const
{
	TArray<UQuest*> Out;
	Out.Reserve(QuestDefinitions.Num());
	for (const auto& Pair : QuestDefinitions)
	{
		if (Pair.Value)
			Out.Add(Pair.Value.Get());
	}
	return Out;
}

void UQuestComponent::ForEachQuestDefinition(TFunctionRef<void(const UQuest*)> Callback) const
{
	for (const auto& Pair : QuestDefinitions)
	{
		if (Pair.Value)
			Callback(Pair.Value.Get());
	}
}

bool UQuestComponent::GetQuestProgressForID(FName QuestID, FQuestProgress& OutProgress) const
{
	if (const FQuestProgress* Found = QuestProgress.Find(QuestID))
	{
		OutProgress = *Found;
		return true;
	}
	return false;
}

bool UQuestComponent::RemoveQuest(FName QuestID)
{
	if (QuestDefinitions.Remove(QuestID) > 0)
	{
		QuestProgress.Remove(QuestID);
		OnQuestsUpdated.Broadcast();
		return true;
	}
	return false;
}

void UQuestComponent::SubscribeToComponentEvents()
{
	AActor* Owner = GetOwner();
	if (!Owner)
		return;

	if (UInventoryComponent* Comp = Owner->FindComponentByClass<UInventoryComponent>())
	{
		Comp->OnItemAdded.AddDynamic(this, &UQuestComponent::OnItemAdded);
		Comp->OnItemRemoved.AddDynamic(this, &UQuestComponent::OnItemRemoved);
	}

	if (UFarmingComponent* Comp = Owner->FindComponentByClass<UFarmingComponent>())
	{
		Comp->OnCropHarvested.AddDynamic(this, &UQuestComponent::OnCropHarvested);
		Comp->OnSeedPlanted.AddDynamic(this, &UQuestComponent::OnSeedPlanted);
		Comp->OnSoilTilled.AddDynamic(this, &UQuestComponent::OnSoilTilled);
		Comp->OnSoilWatered.AddDynamic(this, &UQuestComponent::OnSoilWatered);
		Comp->OnFarmingActionPerformed.AddDynamic(this, &UQuestComponent::OnFarmingActionPerformed);
	}

	if (UWorld* World = GetWorld())
	{
		if (UCropManagerSubsystem* CropManager = World->GetSubsystem<UCropManagerSubsystem>())
		{
			CropManager->OnCropFullyGrown.AddDynamic(this, &UQuestComponent::OnCropFullyGrown);
			CropManager->OnCropWithered.AddDynamic(this, &UQuestComponent::OnCropWithered);
		}
	}
}

void UQuestComponent::UnsubscribeFromComponentEvents()
{
	AActor* Owner = GetOwner();
	if (!Owner)
		return;

	if (UInventoryComponent* Comp = Owner->FindComponentByClass<UInventoryComponent>())
	{
		Comp->OnItemAdded.RemoveDynamic(this, &UQuestComponent::OnItemAdded);
		Comp->OnItemRemoved.RemoveDynamic(this, &UQuestComponent::OnItemRemoved);
	}

	if (UFarmingComponent* Comp = Owner->FindComponentByClass<UFarmingComponent>())
	{
		Comp->OnCropHarvested.RemoveDynamic(this, &UQuestComponent::OnCropHarvested);
		Comp->OnSeedPlanted.RemoveDynamic(this, &UQuestComponent::OnSeedPlanted);
		Comp->OnSoilTilled.RemoveDynamic(this, &UQuestComponent::OnSoilTilled);
		Comp->OnSoilWatered.RemoveDynamic(this, &UQuestComponent::OnSoilWatered);
		Comp->OnFarmingActionPerformed.RemoveDynamic(this, &UQuestComponent::OnFarmingActionPerformed);
	}

	if (UWorld* World = GetWorld())
	{
		if (UCropManagerSubsystem* CropManager = World->GetSubsystem<UCropManagerSubsystem>())
		{
			CropManager->OnCropFullyGrown.RemoveDynamic(this, &UQuestComponent::OnCropFullyGrown);
			CropManager->OnCropWithered.RemoveDynamic(this, &UQuestComponent::OnCropWithered);
		}
	}
}

bool UQuestComponent::AdvanceMatchingQuests(TFunction<bool(const UQuest*)> ShouldAdvance, int32 Amount)
{
	bool bAnyChanged = false;

	for (auto& ProgPair : QuestProgress)
	{
		FQuestProgress& Progress = ProgPair.Value;
		if (Progress.State != EQuestState::InProgress)
			continue;

		UQuest* Def = QuestDefinitions.FindRef(ProgPair.Key).Get();
		if (!Def || !ShouldAdvance(Def))
			continue;

		const EQuestState OldState = Progress.State;
		const int32 OldProg = Progress.CurrentProgress;
		Def->AddProgress(Progress, Amount);

		if (Progress.State != OldState || Progress.CurrentProgress != OldProg)
		{
			bAnyChanged = true;
			if (Progress.State == EQuestState::Completed)
			{
				DispatchQuestRewards(Def, Progress);
				OnQuestCompleted.Broadcast(Def, Progress);
			}
		}
	}

	return bAnyChanged;
}

void UQuestComponent::TryAdvanceAndNotify(TFunction<bool(const UQuest*)> ShouldAdvance, int32 Amount)
{
	if (AdvanceMatchingQuests(ShouldAdvance, Amount))
		OnQuestsUpdated.Broadcast();
}

void UQuestComponent::DispatchQuestRewards(const UQuest* QuestDef, const FQuestProgress& Progress)
{
	if (!QuestDef || !GetOwner())
		return;

	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
		{
			if (QuestDef->XPReward > 0.0f)
			{
				ASC->ApplyModToAttribute(
					ULevelAttributeSet::GetXPAttribute(),
					EGameplayModOp::Additive,
					QuestDef->XPReward
				);
			}
			if (QuestDef->GoldReward > 0.0f)
			{
				ASC->ApplyModToAttribute(
					UEconomyAttributeSet::GetGoldAttribute(),
					EGameplayModOp::Additive,
					QuestDef->GoldReward
				);
			}
		}
	}

	if (QuestDef->RewardItem && QuestDef->RewardItemQuantity > 0)
	{
		if (UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>())
		{
			Inventory->TryAddItem(QuestDef->RewardItem, QuestDef->RewardItemQuantity);
		}
	}
}

void UQuestComponent::OnCropHarvested(AActor* Harvester, UCropDataAsset* CropData, int32 Quantity)
{
	TryAdvanceAndNotify([CropData, Quantity](const UQuest* Q) {
		return Q->ShouldRespondToCropHarvested(CropData, Quantity);
	}, Quantity);
}

void UQuestComponent::OnCropFullyGrown(AActor* Crop)
{
	UCropDataAsset* CropData = Crop && Crop->Implements<UHarvestableInterface>()
		? IHarvestableInterface::Execute_GetCropData(Crop) : nullptr;

	TryAdvanceAndNotify([CropData](const UQuest* Q) {
		return Q->ShouldRespondToCropFullyGrown(CropData);
	}, 1);
}

void UQuestComponent::OnCropWithered(AActor* Crop)
{
	UCropDataAsset* CropData = Crop && Crop->Implements<UHarvestableInterface>()
		? IHarvestableInterface::Execute_GetCropData(Crop) : nullptr;

	TryAdvanceAndNotify([CropData](const UQuest* Q) {
		return Q->ShouldRespondToCropWithered(CropData);
	}, 1);
}

void UQuestComponent::OnSeedPlanted(AActor* Planter, USeedDataAsset* SeedData)
{
	TryAdvanceAndNotify([SeedData](const UQuest* Q) {
		return Q->ShouldRespondToSeedPlanted(SeedData);
	}, 1);
}

void UQuestComponent::OnSoilTilled(AActor* Tiller)
{
	TryAdvanceAndNotify([](const UQuest* Q) {
		return Q->QuestEventType == EQuestEventType::SoilTilled;
	}, 1);
}

void UQuestComponent::OnSoilWatered(AActor* Waterer, AActor* SoilPlot)
{
	TryAdvanceAndNotify([](const UQuest* Q) {
		return Q->QuestEventType == EQuestEventType::SoilWatered;
	}, 1);
}

void UQuestComponent::OnItemAdded(UItemDataAsset* Item, int32 Amount, int32 NewTotal)
{
	TryAdvanceAndNotify([Item, Amount](const UQuest* Q) {
		return Q->ShouldRespondToItemAdded(Item, Amount);
	}, Amount);
}

void UQuestComponent::OnItemRemoved(UItemDataAsset* Item, int32 Amount, int32 NewTotal)
{
	TryAdvanceAndNotify([Item, Amount](const UQuest* Q) {
		return Q->ShouldRespondToItemRemoved(Item, Amount);
	}, Amount);
}

void UQuestComponent::OnFarmingActionPerformed(AActor* Performer)
{
	TryAdvanceAndNotify([](const UQuest* Q) {
		return Q->QuestEventType == EQuestEventType::FarmingActionPerformed;
	}, 1);
}
