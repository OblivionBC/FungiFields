#include "FungiFieldsCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "../Components/InteractionComponent.h"
#include "../Components/InventoryComponent.h"
#include "AbilitySystemComponent.h"
#include "../Attributes/CharacterAttributeSet.h"
#include "../Attributes/EconomyAttributeSet.h"
#include "../Attributes/LevelAttributeSet.h"
#include "../Widgets/PlayerHUDWidget.h"
#include "../Widgets/UBackpackWidget.h"
#include "Blueprint/UserWidget.h"
#include "FungiFields/Components/LevelComponent.h"
#include "FungiFields/Components/QuestComponent.h"
#include "FungiFields/Components/UFarmingComponent.h"
#include "FungiFields/Components/UPlacementComponent.h"
#include "FungiFields/Data/USoilDataAsset.h"
#include "FungiFields/Data/USoilContainerDataAsset.h"
#include "FungiFields/Data/UItemDataAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "Misc/CoreMiscDefines.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AFungiFieldsCharacter

AFungiFieldsCharacter::AFungiFieldsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Components
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	QuestComponent = CreateDefaultSubobject<UQuestComponent>(TEXT("QuestComponent"));
	LevelComponent = CreateDefaultSubobject<ULevelComponent>(TEXT("LevelComponent"));
	FarmingComponent = CreateDefaultSubobject<UFarmingComponent>(TEXT("FarmingComponent"));
	PlacementComponent = CreateDefaultSubobject<UPlacementComponent>(TEXT("PlacementComponent"));

	// Attribute Sets
	CharacterAttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
	EconomyAttributeSet   = CreateDefaultSubobject<UEconomyAttributeSet>(TEXT("EconomyAttributeSet"));
	LevelAttributeSet = CreateDefaultSubobject<ULevelAttributeSet>(TEXT("LevelAttributeSet"));
}

void AFungiFieldsCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		if (InitialCharacterStatsGE)
		{
			FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitialCharacterStatsGE, 1, Context);

			if (SpecHandle.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Health")), 100.0f);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.MaxHealth")), 100.0f);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Stamina")), 100.0f);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.MaxStamina")), 100.0f);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.MaxMagic")), 50.0f);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Magic")), 50.0f);
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}

		if (InitialEconomyStatsGE)
		{
			FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitialEconomyStatsGE, 1, Context);

			if (SpecHandle.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Gold")), 0.0f);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.MaxGold")), 9999999.0f);

				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}

		if (InitialLevelStatsGE)
		{
			FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitialLevelStatsGE, 1, Context);

			if (SpecHandle.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.XP")), 0.0f);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.MaxXP")), 1000);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Level")), 1);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.MaxLevel")), 100);

				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (HUDWidgetClass && IsPlayerControlled())
		{

			HUDWidget = CreateWidget<UPlayerHUDWidget>(GetWorld(), HUDWidgetClass);
			if (HUDWidget)
			{
				HUDWidget->SetOwningPlayer(PC);
				HUDWidget->AddToViewport();
			}
		}

		if (QuestMenuClass)
		{

			QuestMenuWidget = CreateWidget<UQuestMenu>(GetWorld(), QuestMenuClass);
			if (QuestMenuWidget)
			{
				QuestMenuWidget->SetOwningPlayer(PC);
				QuestMenuWidget->AddToViewport();
				QuestMenuWidget->SetVisibility(ESlateVisibility::Hidden);
				QuestMenuWidget->OnQuestMenuClosed.AddDynamic(this, &AFungiFieldsCharacter::OnQuestMenuClosed);
			}
		}

		if (BackpackWidgetClass)
		{
			BackpackWidget = CreateWidget<UBackpackWidget>(GetWorld(), BackpackWidgetClass);
			if (BackpackWidget)
			{
				BackpackWidget->SetOwningPlayer(PC);
				BackpackWidget->AddToViewport();
				BackpackWidget->SetVisibility(ESlateVisibility::Hidden);
				// Bind to close delegate
				BackpackWidget->OnBackpackClosed.AddDynamic(this, &AFungiFieldsCharacter::OnBackpackClosed);
			}
		}
	}

	if (FollowCamera)
	{
		if (InteractionComponent)
		{
			InteractionComponent->SetCamera(FollowCamera);
		}

		if (FarmingComponent)
		{
			FarmingComponent->SetCamera(FollowCamera);
			if (InventoryComponent)
			{
				InventoryComponent->OnInventoryChanged.AddDynamic(FarmingComponent, &UFarmingComponent::UpdateEquippedTool);
			}
		}

		if (PlacementComponent)
		{
			PlacementComponent->SetCamera(FollowCamera);
			if (InventoryComponent)
			{
				// Subscribe to item equipped events to detect placeable items
				InventoryComponent->OnItemEquipped.AddDynamic(this, &AFungiFieldsCharacter::OnItemEquipped);
				// Subscribe to placeable picked up events to add item to inventory
				PlacementComponent->OnPlaceablePickedUp.AddDynamic(this, &AFungiFieldsCharacter::OnSoilPlotPickedUp);
				PlacementComponent->OnContainerPickedUp.AddDynamic(this, &AFungiFieldsCharacter::OnContainerPickedUp);
			}
		}
	}
}

UAbilitySystemComponent* AFungiFieldsCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFungiFieldsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFungiFieldsCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFungiFieldsCharacter::Look);

		if (InteractionComponent)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, InteractionComponent, &UInteractionComponent::Interact);
		}
		
		if (FarmingComponent)
		{
			EnhancedInputComponent->BindAction(UseToolAction, ETriggerEvent::Started, FarmingComponent, &UFarmingComponent::UseEquippedTool);
		}
		
		EnhancedInputComponent->BindAction(ToggleQuestAction, ETriggerEvent::Started, this, &AFungiFieldsCharacter::ToggleQuestMenu);

		EnhancedInputComponent->BindAction(ToggleBackpackAction, ETriggerEvent::Started, this, &AFungiFieldsCharacter::ToggleBackpack);

		if (InventoryComponent)
		{
			EnhancedInputComponent->BindAction(EquipSlot1Action, ETriggerEvent::Started, 
				InventoryComponent, &UInventoryComponent::EquipSlot, 0);
			EnhancedInputComponent->BindAction(EquipSlot2Action, ETriggerEvent::Started, 
				InventoryComponent, &UInventoryComponent::EquipSlot, 1);
			EnhancedInputComponent->BindAction(EquipSlot3Action, ETriggerEvent::Started, 
				InventoryComponent, &UInventoryComponent::EquipSlot, 2);
			EnhancedInputComponent->BindAction(EquipSlot4Action, ETriggerEvent::Started, 
				InventoryComponent, &UInventoryComponent::EquipSlot, 3);
			EnhancedInputComponent->BindAction(EquipSlot5Action, ETriggerEvent::Started, 
				InventoryComponent, &UInventoryComponent::EquipSlot, 4);
			EnhancedInputComponent->BindAction(EquipSlot6Action, ETriggerEvent::Started, 
				InventoryComponent, &UInventoryComponent::EquipSlot, 5);
			EnhancedInputComponent->BindAction(EquipSlot7Action, ETriggerEvent::Started, 
				InventoryComponent, &UInventoryComponent::EquipSlot, 6);
			EnhancedInputComponent->BindAction(EquipSlot8Action, ETriggerEvent::Started, 
				InventoryComponent, &UInventoryComponent::EquipSlot, 7);
			EnhancedInputComponent->BindAction(EquipSlot9Action, ETriggerEvent::Started, 
				InventoryComponent, &UInventoryComponent::EquipSlot, 8);
		}

		if (PlacementComponent)
		{
			if (PlaceAction)
			{
				EnhancedInputComponent->BindAction(PlaceAction, ETriggerEvent::Started, PlacementComponent, &UPlacementComponent::PlaceItem);
			}
			if (PickupAction)
			{
				EnhancedInputComponent->BindAction(PickupAction, ETriggerEvent::Started, PlacementComponent, &UPlacementComponent::PickupItem);
			}
			if (AdjustPlacementRotationAction)
			{
				EnhancedInputComponent->BindAction(AdjustPlacementRotationAction, ETriggerEvent::Triggered, PlacementComponent, &UPlacementComponent::AdjustPlacementRotation);
			}
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AFungiFieldsCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AFungiFieldsCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFungiFieldsCharacter::ToggleQuestMenu(const FInputActionValue& Value)
{
	if (!QuestMenuWidget) 
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestMenuWidget not set!"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return;

	if (!bQuestMenuVisible)
	{
		this->GetCharacterMovement()->StopMovementImmediately();
		QuestMenuWidget->RefreshQuests();
		QuestMenuWidget->SetVisibility(ESlateVisibility::Visible);
		bQuestMenuVisible = true;

		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(QuestMenuWidget->TakeWidget());
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		QuestMenuWidget->SetVisibility(ESlateVisibility::Hidden);
		bQuestMenuVisible = false;
		FInputModeGameOnly Mode;
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = false;
	}
}

void AFungiFieldsCharacter::OnItemEquipped(UItemDataAsset* Item, int32 SlotIndex)
{
	if (!PlacementComponent)
	{
		return;
	}

	// Check if the equipped item is placeable
	if (Item && Item->bIsPlaceable)
	{
		PlacementComponent->EnterPlacementMode(Item);
	}
	else
	{
		// Exit placement mode if item is not placeable
		if (PlacementComponent->IsInPlacementMode())
		{
			PlacementComponent->ExitPlacementMode();
		}
	}
}

void AFungiFieldsCharacter::OnQuestMenuClosed()
{
	if (!QuestMenuWidget)
		return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return;

	QuestMenuWidget->SetVisibility(ESlateVisibility::Hidden);
	bQuestMenuVisible = false;

	FInputModeGameOnly Mode;
	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = false;
}

void AFungiFieldsCharacter::ToggleBackpack(const FInputActionValue& Value)
{
	if (!BackpackWidget) 
	{
		UE_LOG(LogTemp, Warning, TEXT("BackpackWidget not set!"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return;

	if (!bBackpackVisible)
	{
		this->GetCharacterMovement()->StopMovementImmediately();
		
		// Refresh inventory before showing to ensure all items are displayed
		BackpackWidget->RefreshInventory();
		
		BackpackWidget->SetVisibility(ESlateVisibility::Visible);
		bBackpackVisible = true;

		FInputModeUIOnly Mode;
		Mode.SetLockMouseToViewport(true);
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		// Close via widget method (will broadcast delegate)
		BackpackWidget->CloseBackpack();
	}
}

void AFungiFieldsCharacter::OnBackpackClosed()
{
	if (!BackpackWidget)
		return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return;

	BackpackWidget->SetVisibility(ESlateVisibility::Hidden);
	bBackpackVisible = false;
	FInputModeGameOnly Mode;
	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = false;
}

void AFungiFieldsCharacter::OnSoilPlotPickedUp(AActor* Picker, USoilDataAsset* SoilData)
{
	if (!InventoryComponent || !SoilData)
	{
		return;
	}

	// Search inventory for an item that matches this soil data asset
	const TArray<FInventorySlot>& Slots = InventoryComponent->GetInventorySlots();
	UItemDataAsset* MatchingItem = nullptr;

	// First, try to find an existing item in inventory with matching PlaceableSoilDataAsset
	for (const FInventorySlot& Slot : Slots)
	{
		if (!Slot.IsEmpty() && Slot.ItemDefinition && Slot.ItemDefinition->bIsPlaceable)
		{
			if (Slot.ItemDefinition->PlaceableSoilDataAsset == SoilData)
			{
				MatchingItem = const_cast<UItemDataAsset*>(Slot.ItemDefinition.Get());
				break;
			}
		}
	}

	// If we found a matching item, add it to inventory
	if (MatchingItem)
	{
		InventoryComponent->TryAddItem(MatchingItem, 1);
	}
	else
	{
		// Try to find the item using Asset Registry
		// This searches all loaded ItemDataAssets for one matching the soil data
		UItemDataAsset* FoundItem = FindItemDataAssetBySoilData(SoilData);
		if (FoundItem)
		{
			InventoryComponent->TryAddItem(FoundItem, 1);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AFungiFieldsCharacter::OnSoilPlotPickedUp: No matching placeable item found for soil data asset '%s'. Item not added to inventory."), *SoilData->GetName());
		}
	}
}

void AFungiFieldsCharacter::OnContainerPickedUp(AActor* Picker, USoilContainerDataAsset* ContainerData)
{
	if (!InventoryComponent || !ContainerData)
	{
		return;
	}

	// Search inventory for an item that matches this container data asset
	const TArray<FInventorySlot>& Slots = InventoryComponent->GetInventorySlots();
	UItemDataAsset* MatchingItem = nullptr;

	// First, try to find an existing item in inventory with matching PlaceableContainerDataAsset
	for (const FInventorySlot& Slot : Slots)
	{
		if (!Slot.IsEmpty() && Slot.ItemDefinition && Slot.ItemDefinition->bIsPlaceable)
		{
			if (Slot.ItemDefinition->PlaceableContainerDataAsset == ContainerData)
			{
				MatchingItem = const_cast<UItemDataAsset*>(Slot.ItemDefinition.Get());
				break;
			}
		}
	}

	// If we found a matching item, add it to inventory
	if (MatchingItem)
	{
		InventoryComponent->TryAddItem(MatchingItem, 1);
	}
	else
	{
		// Try to find the item using Asset Registry
		// This searches all loaded ItemDataAssets for one matching the container data
		UItemDataAsset* FoundItem = FindItemDataAssetByContainerData(ContainerData);
		if (FoundItem)
		{
			InventoryComponent->TryAddItem(FoundItem, 1);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AFungiFieldsCharacter::OnContainerPickedUp: No matching placeable item found for container data asset '%s'. Item not added to inventory."), *ContainerData->GetName());
		}
	}
}

UItemDataAsset* AFungiFieldsCharacter::FindItemDataAssetByContainerData(USoilContainerDataAsset* ContainerData) const
{
	if (!ContainerData)
	{
		return nullptr;
	}

	// Use Asset Registry to find all ItemDataAssets
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UItemDataAsset::StaticClass()->GetClassPathName(), AssetDataList, true);

	// Search for an item that matches this container data
	for (const FAssetData& AssetData : AssetDataList)
	{
		if (UItemDataAsset* ItemAsset = Cast<UItemDataAsset>(AssetData.GetAsset()))
		{
			if (ItemAsset && ItemAsset->bIsPlaceable && ItemAsset->PlaceableContainerDataAsset == ContainerData)
			{
				return ItemAsset;
			}
		}
	}

	return nullptr;
}

UItemDataAsset* AFungiFieldsCharacter::FindItemDataAssetBySoilData(USoilDataAsset* SoilData) const
{
	if (!SoilData)
	{
		return nullptr;
	}

	// Use Asset Registry to find all ItemDataAsset instances
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UItemDataAsset::StaticClass()->GetClassPathName(), AssetDataList, true);

	// Search through all ItemDataAssets for one matching the soil data
	for (const FAssetData& AssetData : AssetDataList)
	{
		// Load the asset - GetAsset() will load it if not already loaded
		UItemDataAsset* ItemAsset = Cast<UItemDataAsset>(AssetData.GetAsset());
		
		if (ItemAsset && ItemAsset->bIsPlaceable && ItemAsset->PlaceableSoilDataAsset == SoilData)
		{
			return ItemAsset;
		}
	}

	return nullptr;
}
