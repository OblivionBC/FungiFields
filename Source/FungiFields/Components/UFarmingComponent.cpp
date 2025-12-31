#include "UFarmingComponent.h"
#include "Camera/CameraComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Inventory/FInventorySlot.h"
#include "../Data/UToolDataAsset.h"
#include "../Data/USeedDataAsset.h"
#include "../Data/UCropDataAsset.h"
#include "../Data/UItemDataAsset.h"
#include "../Actors/ACropBase.h"
#include "../Interfaces/IFarmableInterface.h"
#include "../Interfaces/IHarvestableInterface.h"
#include "../Data/FHarvestResult.h"
#include "../Attributes/CharacterAttributeSet.h"
#include "../Interfaces/ITooltipProvider.h"
#include "../Widgets/InteractionWidget.h"
#include "AbilitySystemInterface.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Blueprint/UserWidget.h"

UFarmingComponent::UFarmingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	CurrentToolType = EToolType::Hoe;
	CurrentToolPower = 1.0f;
	bHasValidTool = false;
	bHasSeedEquipped = false;
	EquippedSeedData = nullptr;
	EquippedSlotIndexCached = INDEX_NONE;
	LastFarmableTarget = nullptr;
	LastTooltipText = FText::GetEmpty();
	FarmingTooltipWidget = nullptr;
	TooltipTraceDistance = 800.0f;
	TooltipClearDelay = 3.0f;
}

void UFarmingComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdateEquippedTool();
}

void UFarmingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TraceForFarmable();
}

void UFarmingComponent::SetCamera(UCameraComponent* Camera)
{
	CameraComponent = Camera;
}

void UFarmingComponent::UseEquippedTool(const FInputActionValue& Value)
{
	if (!CameraComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFarmingComponent::UseEquippedTool: CameraComponent is null!"));
		return;
	}

	// Perform line trace
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

	// Use the location-based execution method
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

	// Handle soil bag placement
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
						UE_LOG(LogTemp, Log, TEXT("UFarmingComponent: Soil bag detected in ExecuteFarmingAction"));
						if (TargetActor->Implements<UFarmableInterface>())
						{
							UE_LOG(LogTemp, Log, TEXT("UFarmingComponent: Target implements IFarmableInterface"));
							if (IFarmableInterface::Execute_CanAcceptSoilBag(TargetActor, const_cast<UItemDataAsset*>(ItemData)))
							{
								UE_LOG(LogTemp, Log, TEXT("UFarmingComponent: Can accept soil bag"));
								if (IFarmableInterface::Execute_AddSoilFromBag(TargetActor, const_cast<UItemDataAsset*>(ItemData)))
								{
									UE_LOG(LogTemp, Log, TEXT("UFarmingComponent: Soil bag placed successfully"));
									// Consume soil bag from inventory
									InventoryComp->ConsumeFromSlot(EquippedSlotIndex, 1);
									UpdateEquippedTool();
									return true;
								}
								else
								{
									UE_LOG(LogTemp, Warning, TEXT("UFarmingComponent: AddSoilFromBag returned false"));
								}
							}
							else
							{
								UE_LOG(LogTemp, Warning, TEXT("UFarmingComponent: CanAcceptSoilBag returned false"));
							}
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("UFarmingComponent: Target does not implement IFarmableInterface"));
						}
						return false;
					}
				}
			}
		}
	}

	// Handle seed planting
	if (SeedData && SeedData->CropToPlant && TargetActor->Implements<UFarmableInterface>())
	{
		if (IFarmableInterface::Execute_CanAcceptSeed(TargetActor))
		{
			if (IFarmableInterface::Execute_PlantSeed(TargetActor, SeedData->CropToPlant.Get(), GetOwner()))
			{
				// Consume seed from inventory if available (only for players with inventory)
				if (UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>())
				{
					InventoryComp->ConsumeFromSlot(EquippedSlotIndexCached, 1);
					UpdateEquippedTool();
				}
				
				// Broadcast seed planted event (works for both players and AI workers)
				OnSeedPlanted.Broadcast(GetOwner(), SeedData);
				
				return true;
			}
		}
		return false;
	}

	// Handle tool interactions
	if (ToolType == EToolType::None)
	{
		return false;
	}

	// Check if actor implements IFarmableInterface
	if (TargetActor->Implements<UFarmableInterface>())
	{
		bool bSuccess = IFarmableInterface::Execute_InteractTool(TargetActor, ToolType, GetOwner(), ToolPower);
		
		if (bSuccess)
		{
			// Consume stamina only if ability system exists (players have it, AI workers might not)
			if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
			{
				float StaminaCost = 10.0f; // Default
				
				// Try to get actual stamina cost from equipped tool
				UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>();
				if (InventoryComp)
				{
					int32 EquippedSlotIndex = InventoryComp->GetEquippedSlot();
					if (EquippedSlotIndex != INDEX_NONE)
					{
						const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
						if (Slots.IsValidIndex(EquippedSlotIndex))
						{
							const FInventorySlot& Slot = Slots[EquippedSlotIndex];
							if (const UToolDataAsset* ToolData = Cast<const UToolDataAsset>(Slot.ItemDefinition))
							{
								StaminaCost = ToolData->StaminaCost;
							}
						}
					}
				}
				
				ConsumeStamina(StaminaCost);
			}
			
			// Broadcast events based on tool type (works for both players and AI workers)
			if (ToolType == EToolType::Hoe)
			{
				OnSoilTilled.Broadcast(GetOwner());
			}
			else if (ToolType == EToolType::WateringCan)
			{
				OnSoilWatered.Broadcast(GetOwner(), TargetActor);
			}
		}
		
		return bSuccess;
	}
	// Check if actor implements IHarvestableInterface
	else if (TargetActor->Implements<UHarvestableInterface>())
	{
		if (IHarvestableInterface::Execute_CanHarvest(TargetActor))
		{
			FHarvestResult HarvestResult = IHarvestableInterface::Execute_Harvest(TargetActor, GetOwner(), ToolPower);
			
			if (HarvestResult.bSuccess)
			{
				// Consume stamina only if ability system exists (players have it, AI workers might not)
				if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
				{
					float StaminaCost = 10.0f;
					
					// Try to get actual stamina cost from equipped tool
					UInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>();
					if (InventoryComp)
					{
						int32 EquippedSlotIndex = InventoryComp->GetEquippedSlot();
						if (EquippedSlotIndex != INDEX_NONE)
						{
							const TArray<FInventorySlot>& Slots = InventoryComp->GetInventorySlots();
							if (Slots.IsValidIndex(EquippedSlotIndex))
							{
								const FInventorySlot& Slot = Slots[EquippedSlotIndex];
								if (const UToolDataAsset* ToolData = Cast<const UToolDataAsset>(Slot.ItemDefinition))
								{
									StaminaCost = ToolData->StaminaCost;
								}
							}
						}
					}
					
					ConsumeStamina(StaminaCost);
				}
				
				// Get crop data for event broadcasting
				UCropDataAsset* CropData = nullptr;
				if (ACropBase* Crop = Cast<ACropBase>(TargetActor))
				{
					CropData = Crop->GetCropData();
				}
				
				// Broadcast crop harvested event (works for both players and AI workers)
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

	// Check seed planting
	if (SeedData && SeedData->CropToPlant && TargetActor->Implements<UFarmableInterface>())
	{
		return IFarmableInterface::Execute_CanAcceptSeed(TargetActor);
	}

	// Check tool interactions
	if (ToolType == EToolType::None)
	{
		return false;
	}

	if (TargetActor->Implements<UFarmableInterface>())
	{
		// For farmable, we can always try to interact (the interface will determine if it's valid)
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

	// Check if equipped item is a tool
	if (const UToolDataAsset* ToolData = Cast<const UToolDataAsset>(EquippedSlot.ItemDefinition))
	{
		// Valid tool equipped
		bHasValidTool = true;
		EToolType NewToolType = ToolData->ToolType;
		CurrentToolPower = ToolData->ToolPower;
		ToolTraceDistance = ToolData->ToolRange;
		
		// Clear tooltip text if tool type changed or switched from seed to tool
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
			
			// Clear tooltip text if switched from tool to seed or seed changed
			if (bPreviousHasTool != bHasValidTool || bPreviousHasSeed != bHasSeedEquipped)
			{
				LastTooltipText = FText::GetEmpty();
			}
		}
	}
}

bool UFarmingComponent::PerformToolTrace(FHitResult& OutHit) const
{
	if (!CameraComponent || !GetWorld())
	{
		return false;
	}

	FVector Start = CameraComponent->GetComponentLocation();
	FVector ForwardVector = CameraComponent->GetForwardVector();
	FVector End = Start + (ForwardVector * ToolTraceDistance);

	FCollisionQueryParams TraceParams(FName(TEXT("ToolTrace")), true, GetOwner());
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_Visibility,
		TraceParams
	);

	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 2.0f);

	return bHit;
}

bool UFarmingComponent::ConsumeStamina(float StaminaCost)
{
	if (!GetOwner())
	{
		return false;
	}

	// Get ability system component
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
		if (!ASC)
		{
			return false;
		}

		// Get character attribute set
		const UCharacterAttributeSet* AttributeSet = ASC->GetSet<UCharacterAttributeSet>();
		if (!AttributeSet)
		{
			return false;
		}

		// Check if enough stamina
		float CurrentStamina = AttributeSet->GetStamina();
		if (CurrentStamina < StaminaCost)
		{
			return false;
		}

		// Consume stamina
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
	// Only trace for tooltips if camera is set (player mode)
	// NPCs don't need tooltips, so skip if no camera
	if (!CameraComponent)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Only show tooltip if we have a tool, seed, or soil bag equipped
	// Check for soil bag first
	bool bHasSoilBagEquipped = false;
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
	FCollisionQueryParams Params(FName(TEXT("FarmingTooltipTrace")), true, GetOwner());

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

	// Check for soil bag placement first (highest priority)
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
						UE_LOG(LogTemp, VeryVerbose, TEXT("UFarmingComponent::TraceForFarmable: Soil bag detected in tooltip trace"));
						if (HitActor->Implements<UFarmableInterface>())
						{
							UE_LOG(LogTemp, VeryVerbose, TEXT("UFarmingComponent::TraceForFarmable: HitActor implements IFarmableInterface"));
							if (IFarmableInterface::Execute_CanAcceptSoilBag(HitActor, const_cast<UItemDataAsset*>(ItemData)))
							{
								UE_LOG(LogTemp, VeryVerbose, TEXT("UFarmingComponent::TraceForFarmable: Can accept soil bag - showing tooltip"));
								TooltipText = FText::FromString(TEXT("Left Click to Place Soil"));
								bShouldShowTooltip = true;
							}
							else
							{
								UE_LOG(LogTemp, VeryVerbose, TEXT("UFarmingComponent::TraceForFarmable: Cannot accept soil bag"));
							}
						}
						else
						{
							UE_LOG(LogTemp, VeryVerbose, TEXT("UFarmingComponent::TraceForFarmable: HitActor does not implement IFarmableInterface"));
						}
					}
				}
			}
		}
	}

	// Check for seed planting (second priority)
	if (!bShouldShowTooltip && bHasSeedEquipped && EquippedSeedData && EquippedSeedData->CropToPlant && HitActor->Implements<UFarmableInterface>())
	{
		if (IFarmableInterface::Execute_CanAcceptSeed(HitActor))
		{
			FString SeedName = EquippedSeedData->CropToPlant->CropName.ToString();
			TooltipText = FText::FromString(FString::Printf(TEXT("Left Click to Plant %s"), *SeedName));
			bShouldShowTooltip = true;
		}
	}
	// Check for tool interaction with farmable
	else if (bHasValidTool && HitActor->Implements<UFarmableInterface>())
	{
		// Validate that the tool can actually interact with this farmable
		// Note: Scythe is excluded here - harvest should only show on crops, not soil
		if (IFarmableInterface::Execute_CanInteractWithTool(HitActor, CurrentToolType, GetOwner()) && CurrentToolType != EToolType::Scythe)
		{
			// Get context-sensitive text based on tool type
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
	}
	// Check for harvestable (crops)
	else if (bHasValidTool && HitActor->Implements<UHarvestableInterface>())
	{
		// Only show harvest tooltip if holding a scythe
		if (CurrentToolType == EToolType::Scythe && IHarvestableInterface::Execute_CanHarvest(HitActor))
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
	}

	// Update tooltip if actor changed OR if tooltip text would be different (tool changed)
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
	UWorld* World = GetWorld();
	if (!FarmingTooltipWidget && FarmingTooltipWidgetClass && World)
	{
		FarmingTooltipWidget = CreateWidget<UUserWidget>(World, FarmingTooltipWidgetClass);
		if (FarmingTooltipWidget)
		{
			FarmingTooltipWidget->AddToViewport();
		}
	}

	if (FarmingTooltipWidget)
	{
		// Try to cast to InteractionWidget to use its SetPromptText method
		if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(FarmingTooltipWidget))
		{
			InteractionWidget->SetPromptText(Prompt);
			InteractionWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			// If not InteractionWidget, just make it visible
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