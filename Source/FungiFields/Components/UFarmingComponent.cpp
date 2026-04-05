#include "UFarmingComponent.h"
#include "Camera/CameraComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "../Data/UToolDataAsset.h"
#include "../Data/USeedDataAsset.h"
#include "../Data/UCropDataAsset.h"
#include "../Data/UItemDataAsset.h"
#include "../Interfaces/IFarmableInterface.h"
#include "../Interfaces/IHarvestableInterface.h"
#include "../Data/FHarvestResult.h"
#include "../Attributes/CharacterAttributeSet.h"
#include "../Interfaces/ITooltipProvider.h"
#include "../Widgets/InteractionWidget.h"
#include "AbilitySystemInterface.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

UFarmingComponent::UFarmingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFarmingComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateEquippedTool();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TooltipPollingTimer,
			this,
			&UFarmingComponent::TraceForFarmable,
			0.0667f,
			true
		);
	}
}

void UFarmingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TooltipPollingTimer);
		World->GetTimerManager().ClearTimer(FarmableResetTimer);
	}
	ClearFarmable();
	Super::EndPlay(EndPlayReason);
}

void UFarmingComponent::SetCamera(UCameraComponent* Camera)
{
	CameraComponent = Camera;
}

void UFarmingComponent::UseEquippedTool(const FInputActionValue& Value)
{
	if (!CameraComponent)
		return;

	FHitResult HitResult;
	if (!PerformToolTrace(HitResult))
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		return;
	}

	FVector ActionLocation = HitResult.Location;
	USeedDataAsset* SeedData = bHasSeedEquipped ? EquippedSeedData : nullptr;
	EToolType ToolType = bHasValidTool ? CurrentToolType : EToolType::None;
	float ToolPower = bHasValidTool ? CurrentToolPower : 1.0f;

	ExecuteFarmingAction(HitActor, ActionLocation, ToolType, ToolPower, SeedData);
}

bool UFarmingComponent::ExecuteFarmingAction(AActor* TargetActor, const FVector& ActionLocation, EToolType ToolType, float ToolPower, USeedDataAsset* SeedData)
{
	if (!TargetActor)
	{
		return false;
	}

	if (UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>())
	{
		const int32 EquippedSlotIndex = InventoryComp->GetEquippedSlot();
		if (EquippedSlotIndex != INDEX_NONE)
		{
			const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
			if (Slots.IsValidIndex(EquippedSlotIndex))
			{
				const FInventorySlot& Slot = Slots[EquippedSlotIndex];
				if (const UItemDataAsset* ItemData = Cast<const UItemDataAsset>(Slot.ItemDefinition))
				{
					if (ItemData->bIsSoilBag)
					{
						if (TargetActor->Implements<UFarmableInterface>() &&
							IFarmableInterface::Execute_CanAcceptSoilBag(TargetActor, const_cast<UItemDataAsset*>(ItemData)) &&
							IFarmableInterface::Execute_AddSoilFromBag(TargetActor, const_cast<UItemDataAsset*>(ItemData)))
						{
							InventoryComp->ConsumeFromSlot(EquippedSlotIndex, 1);
							UpdateEquippedTool();
							OnFarmingActionPerformed.Broadcast(GetOwner());
							return true;
						}
						return false;
					}
				}
			}
		}
	}

	if (SeedData && SeedData->CropToPlant && TargetActor->Implements<UFarmableInterface>())
	{
		if (IFarmableInterface::Execute_CanAcceptSeed(TargetActor))
		{
			if (IFarmableInterface::Execute_PlantSeed(TargetActor, SeedData->CropToPlant.Get(), GetOwner()))
			{
				if (UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>())
				{
					InventoryComp->ConsumeFromSlot(EquippedSlotIndexCached, 1);
					UpdateEquippedTool();
				}

				OnSeedPlanted.Broadcast(GetOwner(), SeedData);
				return true;
			}
		}
		return false;
	}

	if (ToolType == EToolType::None)
	{
		return false;
	}

	if (TargetActor->Implements<UFarmableInterface>())
	{
		bool bSuccess = IFarmableInterface::Execute_InteractTool(TargetActor, ToolType, GetOwner(), ToolPower);
		
		if (bSuccess)
		{
			ConsumeStamina(ResolveStaminaCost());

			if (ToolType == EToolType::Hoe)
			{
				OnSoilTilled.Broadcast(GetOwner());
				OnFarmingActionPerformed.Broadcast(GetOwner());
			}
			else if (ToolType == EToolType::WateringCan)
			{
				OnSoilWatered.Broadcast(GetOwner(), TargetActor);
				OnFarmingActionPerformed.Broadcast(GetOwner());
			}
		}
		
		return bSuccess;
	}
	else if (TargetActor->Implements<UHarvestableInterface>())
	{
		if (IHarvestableInterface::Execute_CanHarvest(TargetActor))
		{
			FHarvestResult HarvestResult = IHarvestableInterface::Execute_Harvest(TargetActor, GetOwner(), ToolPower);
			
			if (HarvestResult.bSuccess)
			{
				ConsumeStamina(ResolveStaminaCost());

				UCropDataAsset* CropData = IHarvestableInterface::Execute_GetCropData(TargetActor);

				OnCropHarvested.Broadcast(GetOwner(), CropData, HarvestResult.Quantity);
			}
			
			return HarvestResult.bSuccess;
		}
	}

	return false;
}

bool UFarmingComponent::CanPerformFarmingAction(AActor* TargetActor, EToolType ToolType, USeedDataAsset* SeedData) const
{
	if (!TargetActor)
	{
		return false;
	}

	if (SeedData && SeedData->CropToPlant && TargetActor->Implements<UFarmableInterface>())
	{
		return IFarmableInterface::Execute_CanAcceptSeed(TargetActor);
	}

	if (ToolType == EToolType::None)
	{
		return false;
	}

	if (TargetActor->Implements<UFarmableInterface>())
	{
		return true;
	}
	else if (TargetActor->Implements<UHarvestableInterface>())
	{
		return IHarvestableInterface::Execute_CanHarvest(TargetActor);
	}

	return false;
}

void UFarmingComponent::SetEquippedTool(EToolType ToolType, float ToolPower)
{
	CurrentToolType = ToolType;
	CurrentToolPower = ToolPower;
}

void UFarmingComponent::UpdateEquippedTool()
{
	EToolType PreviousToolType = CurrentToolType;
	bool bPreviousHasTool = bHasValidTool;
	bool bPreviousHasSeed = bHasSeedEquipped;
	
	bHasValidTool = false;
	bHasSeedEquipped = false;
	EquippedSeedData = nullptr;
	EquippedSlotIndexCached = INDEX_NONE;

	if (!GetOwner())
	{
		return;
	}

	UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComp)
	{
		return;
	}

	const int32 EquippedSlotIndex = InventoryComp->GetEquippedSlot();
	if (EquippedSlotIndex == INDEX_NONE)
	{
		return;
	}

	const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
	if (!Slots.IsValidIndex(EquippedSlotIndex))
	{
		return;
	}

	const FInventorySlot& EquippedSlot = Slots[EquippedSlotIndex];
	if (EquippedSlot.IsEmpty())
	{
		return;
	}

	EquippedSlotIndexCached = EquippedSlotIndex;

	if (const UToolDataAsset* ToolData = Cast<const UToolDataAsset>(EquippedSlot.ItemDefinition))
	{
		bHasValidTool = true;
		EToolType NewToolType = ToolData->ToolType;
		CurrentToolPower = ToolData->ToolPower;
		ToolTraceDistance = ToolData->ToolRange;
		
		if (PreviousToolType != NewToolType || bPreviousHasTool != bHasValidTool || bPreviousHasSeed != bHasSeedEquipped)
		{
			LastTooltipText = FText::GetEmpty();
		}
		
		CurrentToolType = NewToolType;
		return;
	}

	if (USeedDataAsset* SeedData = const_cast<USeedDataAsset*>(Cast<const USeedDataAsset>(EquippedSlot.ItemDefinition)))
	{
		if (SeedData->CropToPlant)
		{
			bHasSeedEquipped = true;
			EquippedSeedData = SeedData;
			
			if (bPreviousHasTool != bHasValidTool || bPreviousHasSeed != bHasSeedEquipped)
			{
				LastTooltipText = FText::GetEmpty();
			}
		}
	}
}

bool UFarmingComponent::PerformToolTrace(FHitResult& OutHit) const
{
	AActor* Owner = GetOwner();
	if (!CameraComponent || !GetWorld() || !Owner)
		return false;

	FVector Start = CameraComponent->GetComponentLocation();
	FVector ForwardVector = CameraComponent->GetForwardVector();
	FVector End = Start + (ForwardVector * ToolTraceDistance);

	FCollisionQueryParams TraceParams(FName(TEXT("ToolTrace")), true, Owner);
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;

	return GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_Visibility,
		TraceParams
	);
}

float UFarmingComponent::ResolveStaminaCost() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
		return 10.0f;

	if (UInventoryComponent* InventoryComp = Owner->FindComponentByClass<UInventoryComponent>())
	{
		const int32 SlotIndex = InventoryComp->GetEquippedSlot();
		if (SlotIndex != INDEX_NONE)
		{
			const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
			if (Slots.IsValidIndex(SlotIndex))
			{
				if (const UToolDataAsset* ToolData = Cast<const UToolDataAsset>(Slots[SlotIndex].ItemDefinition))
					return ToolData->StaminaCost;
			}
		}
	}
	return 10.0f;
}

bool UFarmingComponent::ConsumeStamina(float StaminaCost)
{
	if (!GetOwner())
	{
		return false;
	}

	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
		if (!ASC)
		{
			return false;
		}

		const UCharacterAttributeSet* AttributeSet = ASC->GetSet<UCharacterAttributeSet>();
		if (!AttributeSet)
		{
			return false;
		}

		float CurrentStamina = AttributeSet->GetStamina();
		if (CurrentStamina < StaminaCost)
		{
			return false;
		}

		ASC->ApplyModToAttribute(
			UCharacterAttributeSet::GetStaminaAttribute(),
			EGameplayModOp::Additive,
			-StaminaCost
		);

		return true;
	}

	return false;
}

void UFarmingComponent::TraceForFarmable()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!CameraComponent || !World || !Owner)
		return;

	bool bHasSoilBagEquipped = false;
	if (UInventoryComponent* InventoryComp = Owner->FindComponentByClass<UInventoryComponent>())
	{
		const int32 EquippedSlotIndex = InventoryComp->GetEquippedSlot();
		if (EquippedSlotIndex != INDEX_NONE)
		{
			const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
			if (Slots.IsValidIndex(EquippedSlotIndex))
			{
				const FInventorySlot& Slot = Slots[EquippedSlotIndex];
				if (const UItemDataAsset* ItemData = Cast<const UItemDataAsset>(Slot.ItemDefinition))
				{
					bHasSoilBagEquipped = ItemData->bIsSoilBag;
				}
			}
		}
	}

	if (!bHasValidTool && !bHasSeedEquipped && !bHasSoilBagEquipped)
	{
		HideFarmingTooltip();
		if (LastFarmableTarget && !IsValid(LastFarmableTarget))
		{
			ClearFarmable();
		}
		else if (LastFarmableTarget)
		{
			World->GetTimerManager().SetTimer(
				FarmableResetTimer,
				this,
				&UFarmingComponent::ClearFarmable,
				TooltipClearDelay,
				false
			);
		}
		return;
	}

	FVector Start = CameraComponent->GetComponentLocation();
	FVector End = Start + (CameraComponent->GetForwardVector() * TooltipTraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params(FName(TEXT("FarmingTooltipTrace")), true, Owner);

	World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
	
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		HideFarmingTooltip();
		if (LastFarmableTarget && !IsValid(LastFarmableTarget))
		{
			ClearFarmable();
		}
		else if (LastFarmableTarget)
		{
			World->GetTimerManager().SetTimer(
				FarmableResetTimer,
				this,
				&UFarmingComponent::ClearFarmable,
				TooltipClearDelay,
				false
			);
		}
		return;
	}

	FText TooltipText;
	bool bShouldShowTooltip = false;

	if (UInventoryComponent* InventoryComp = Owner->FindComponentByClass<UInventoryComponent>())
	{
		const int32 EquippedSlotIndex = InventoryComp->GetEquippedSlot();
		if (EquippedSlotIndex != INDEX_NONE)
		{
			const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
			if (Slots.IsValidIndex(EquippedSlotIndex))
			{
				const FInventorySlot& Slot = Slots[EquippedSlotIndex];
				if (const UItemDataAsset* ItemData = Slot.ItemDefinition.Get())
				{
					if (ItemData->bIsSoilBag && HitActor->Implements<UFarmableInterface>())
					{
						if (IFarmableInterface::Execute_CanAcceptSoilBag(HitActor, const_cast<UItemDataAsset*>(ItemData)))
						{
							TooltipText = FText::FromString(TEXT("Left Click to Place Soil"));
							bShouldShowTooltip = true;
						}
						else
						{
							FText BlockedReason = IFarmableInterface::Execute_GetCannotAcceptSoilBagReason(HitActor);
							if (!BlockedReason.IsEmpty())
							{
								TooltipText = BlockedReason;
								bShouldShowTooltip = true;
							}
						}
					}
				}
			}
		}
	}

	if (!bShouldShowTooltip && bHasSeedEquipped && EquippedSeedData && EquippedSeedData->CropToPlant && HitActor->Implements<UFarmableInterface>())
	{
		if (IFarmableInterface::Execute_CanAcceptSeed(HitActor))
		{
			FString SeedName = EquippedSeedData->CropToPlant->CropName.ToString();
			TooltipText = FText::FromString(FString::Printf(TEXT("Left Click to Plant %s"), *SeedName));
			bShouldShowTooltip = true;
		}
		else
		{
			FText BlockedReason = IFarmableInterface::Execute_GetCannotPlantReason(HitActor);
			if (!BlockedReason.IsEmpty())
			{
				TooltipText = BlockedReason;
				bShouldShowTooltip = true;
			}
		}
	}
	else if (bHasValidTool && HitActor->Implements<UFarmableInterface>())
	{
		if (IFarmableInterface::Execute_CanInteractWithTool(HitActor, CurrentToolType, Owner) && CurrentToolType != EToolType::Scythe)
		{
			FString ActionText;
			switch (CurrentToolType)
			{
			case EToolType::Hoe:
				ActionText = "Till Soil";
				break;
			case EToolType::WateringCan:
				ActionText = "Water Soil";
				break;
			default:
				break;
			}

			if (!ActionText.IsEmpty())
			{
				TooltipText = FText::FromString(FString::Printf(TEXT("Left Click to %s"), *ActionText));
				bShouldShowTooltip = true;
			}
		}
		else if (CurrentToolType != EToolType::Scythe)
		{
			FText BlockedReason = IFarmableInterface::Execute_GetCannotUseToolReason(HitActor, CurrentToolType);
			if (!BlockedReason.IsEmpty())
			{
				TooltipText = BlockedReason;
				bShouldShowTooltip = true;
			}
		}
	}
	else if (bHasValidTool && HitActor->Implements<UHarvestableInterface>() && CurrentToolType == EToolType::Scythe)
	{
		if (HitActor->Implements<UTooltipProvider>())
		{
			TooltipText = ITooltipProvider::Execute_GetTooltipText(HitActor);
		}
		else
		{
			TooltipText = IHarvestableInterface::Execute_GetHarvestText(HitActor);
		}
		if (!TooltipText.IsEmpty())
		{
			TooltipText = FText::FromString(FString::Printf(TEXT("Left Click to %s"), *TooltipText.ToString()));
			bShouldShowTooltip = true;
		}
	}

	bool bShouldUpdateTooltip = bShouldShowTooltip && (HitActor != LastFarmableTarget || !TooltipText.EqualTo(LastTooltipText));
	
	if (bShouldUpdateTooltip)
	{
		ShowFarmingTooltip(HitActor, TooltipText);
		LastFarmableTarget = HitActor;
		LastTooltipText = TooltipText;
		World->GetTimerManager().ClearTimer(FarmableResetTimer);
	}
	else if (!bShouldShowTooltip)
	{
		HideFarmingTooltip();
		if (LastFarmableTarget && !IsValid(LastFarmableTarget))
		{
			ClearFarmable();
		}
		else if (LastFarmableTarget)
		{
			World->GetTimerManager().SetTimer(
				FarmableResetTimer,
				this,
				&UFarmingComponent::ClearFarmable,
				TooltipClearDelay,
				false
			);
		}
	}
}

void UFarmingComponent::ShowFarmingTooltip(AActor* Target, const FText& Prompt)
{
	if (!FarmingTooltipWidget && FarmingTooltipWidgetClass)
	{
		APawn* PawnOwner = Cast<APawn>(GetOwner());
		APlayerController* PC = PawnOwner ? Cast<APlayerController>(PawnOwner->GetController()) : nullptr;
		if (PC)
		{
			FarmingTooltipWidget = CreateWidget<UUserWidget>(PC, FarmingTooltipWidgetClass);
			if (FarmingTooltipWidget)
				FarmingTooltipWidget->AddToViewport();
		}
	}

	if (FarmingTooltipWidget)
	{
		if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(FarmingTooltipWidget))
		{
			InteractionWidget->SetPromptText(Prompt);
			InteractionWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			FarmingTooltipWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}

	LastFarmableTarget = Target;
}

void UFarmingComponent::HideFarmingTooltip()
{
	if (FarmingTooltipWidget)
	{
		if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(FarmingTooltipWidget))
		{
			InteractionWidget->HidePrompt();
		}
		else
		{
			FarmingTooltipWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	LastFarmableTarget = nullptr;
}

void UFarmingComponent::ClearFarmable()
{
	LastFarmableTarget = nullptr;
	LastTooltipText = FText::GetEmpty();
	HideFarmingTooltip();
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FarmableResetTimer);
	}
}