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

AFungiFieldsCharacter::AFungiFieldsCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	QuestComponent = CreateDefaultSubobject<UQuestComponent>(TEXT("QuestComponent"));
	LevelComponent = CreateDefaultSubobject<ULevelComponent>(TEXT("LevelComponent"));
	FarmingComponent = CreateDefaultSubobject<UFarmingComponent>(TEXT("FarmingComponent"));
	PlacementComponent = CreateDefaultSubobject<UPlacementComponent>(TEXT("PlacementComponent"));

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
				InventoryComponent->OnItemEquipped.AddDynamic(this, &AFungiFieldsCharacter::OnItemEquipped);
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

void AFungiFieldsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFungiFieldsCharacter::Move);

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
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AFungiFieldsCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
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

	// If we're already in placement mode, check if we need to update or exit
	if (PlacementComponent->IsInPlacementMode())
	{
		if (Item && Item->bIsPlaceable)
		{
			// Check if it's a different placeable item - if so, update placement mode
			// EnterPlacementMode will handle updating the preview actor
			PlacementComponent->EnterPlacementMode(Item);
		}
		else
		{
			// New item is not placeable, exit placement mode
			PlacementComponent->ExitPlacementMode();
		}
	}
	else
	{
		// Not in placement mode - enter it if the new item is placeable
		if (Item && Item->bIsPlaceable)
		{
			PlacementComponent->EnterPlacementMode(Item);
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
		
		if (!BackpackWidget->IsInViewport())
		{
			BackpackWidget->AddToViewport();
		}
		
		BackpackWidget->RefreshInventory();
		
		BackpackWidget->SetVisibility(ESlateVisibility::Visible);
		bBackpackVisible = true;

		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(BackpackWidget->TakeWidget());
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = true;
	}
	else
	{
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

	const TArray<FInventorySlot>& Slots = InventoryComponent->GetInventorySlots();
	UItemDataAsset* MatchingItem = nullptr;

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

	if (MatchingItem)
	{
		InventoryComponent->TryAddItem(MatchingItem, 1);
	}
	else
	{
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

	const TArray<FInventorySlot>& Slots = InventoryComponent->GetInventorySlots();
	UItemDataAsset* MatchingItem = nullptr;

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

	if (MatchingItem)
	{
		InventoryComponent->TryAddItem(MatchingItem, 1);
	}
	else
	{
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

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UItemDataAsset::StaticClass()->GetClassPathName(), AssetDataList, true);

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

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UItemDataAsset::StaticClass()->GetClassPathName(), AssetDataList, true);

	for (const FAssetData& AssetData : AssetDataList)
	{
		UItemDataAsset* ItemAsset = Cast<UItemDataAsset>(AssetData.GetAsset());
		
		if (ItemAsset && ItemAsset->bIsPlaceable && ItemAsset->PlaceableSoilDataAsset == SoilData)
		{
			return ItemAsset;
		}
	}

	return nullptr;
}
