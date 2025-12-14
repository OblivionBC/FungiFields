// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "FungiFields/Widgets/QuestMenu.h"
#include "Logging/LogMacros.h"
#include "FungiFieldsCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UInteractionComponent;
class UInventoryComponent;
class UAbilitySystemComponent;
class UCharacterAttributeSet;
class UEconomyAttributeSet;
class UPlayerHUDWidget;
class ULevelComponent;
class UGameplayEffect;
class UQuestComponent;
class ULevelAttributeSet;
class UFarmingComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AFungiFieldsCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** Interaction component for handling interactions with interactable actors */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	UInteractionComponent* InteractionComponent;

	/** Interaction component for handling interactions with interactable actors */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	UQuestComponent* QuestComponent;
	
	/** Inventory component for managing player inventory */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent;

	/** Level component for handling XP changes and Level Events */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	ULevelComponent* LevelComponent;

	/** Farming component for handling tool usage */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming", meta = (AllowPrivateAccess = "true"))
	UFarmingComponent* FarmingComponent;

	/** Placement component for handling placeable items */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placement", meta = (AllowPrivateAccess = "true"))
	class UPlacementComponent* PlacementComponent;
	
	/** Ability System Component. Required to use Gameplay Attributes and Gameplay Abilities. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	/** Character Attribute Set containing Health, Stamina, and Magic attributes. */
	UPROPERTY()
	UCharacterAttributeSet* CharacterAttributeSet;

	/** Economy Attribute Set containing Gold attributes. */
	UPROPERTY()
	UEconomyAttributeSet* EconomyAttributeSet;

	UPROPERTY()
	ULevelAttributeSet* LevelAttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> InitialCharacterStatsGE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> InitialEconomyStatsGE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> InitialLevelStatsGE;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Interact Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	/** Toggle Quest Menu Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleQuestAction;

	/** Use Tool Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* UseToolAction;
	
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	
	/** Equip Item Actions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipSlot1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipSlot2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipSlot3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipSlot4Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipSlot5Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipSlot6Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipSlot7Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipSlot8Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipSlot9Action;

	/** Place Item Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PlaceAction;

	/** Pickup Item Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PickupAction;

	/** Adjust Placement Rotation Input Action (Scroll Wheel) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AdjustPlacementRotationAction;
	
	/** HUD Widget class to create and display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPlayerHUDWidget> HUDWidgetClass;
	
	/** Quest Menu Widget class to create and display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UQuestMenu> QuestMenuClass;
	
	/** Instance of the HUD widget */
	UPROPERTY()
	TObjectPtr<UPlayerHUDWidget> HUDWidget;

	/** Instance of the Quest Menu widget */
	UPROPERTY()
	TObjectPtr<UQuestMenu> QuestMenuWidget;

	bool bQuestMenuVisible = false;

public:
	AFungiFieldsCharacter();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UFUNCTION()
	void ToggleQuestMenu(const FInputActionValue& Value);

	UFUNCTION()
	void OnQuestMenuClosed();

	UFUNCTION()
	void OnItemEquipped(UItemDataAsset* Item, int32 SlotIndex);

	UFUNCTION()
	void OnSoilPlotPickedUp(AActor* Picker, class USoilDataAsset* SoilData);

private:
	/**
	 * Find an ItemDataAsset that matches the given SoilDataAsset.
	 * Searches through asset registry for placeable items.
	 * @param SoilData The soil data asset to match
	 * @return Matching ItemDataAsset, or nullptr if not found
	 */
	class UItemDataAsset* FindItemDataAssetBySoilData(class USoilDataAsset* SoilData) const;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

