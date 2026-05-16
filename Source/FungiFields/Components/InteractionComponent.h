#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class AActor;
class UCameraComponent;
class UInteractionWidget;
class UInputAction;
class AFarmerVillagerCharacter;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVillagerInteractionStarted, AFarmerVillagerCharacter*, Villager);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent(const FObjectInitializer& ObjectInitializer);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetCamera(UCameraComponent* Camera);

	/** Fired when the player successfully interacts with a villager. Bind HUD sub-widgets here
	 *  (e.g. hunger bar) to subscribe to the returned villager's component delegates. */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnVillagerInteractionStarted OnVillagerInteractionStarted;

	/** Returns the villager from the most recent villager interaction, or nullptr. */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AFarmerVillagerCharacter* GetCurrentInteractedVillager() const { return CurrentInteractedVillager.Get(); }

protected:
	virtual void BeginPlay() override;

	void TraceForInteractable();
	void ClearInteractable();
	void ShowInteractionWidget(AActor* Interactable, const FText& Prompt);
	void HideInteractionWidget();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TSubclassOf<UUserWidget> InteractionWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0"))
	float TraceDistance = 575.0f;

	/** Half-width of the sphere used for the interaction sweep. Larger values make it easier to hit narrow targets like villagers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0"))
	float TraceRadius = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0"))
	float ClearDelay = 3.0f;

private:
	UPROPERTY()
	UCameraComponent* CameraComponent = nullptr;

	UPROPERTY()
	AActor* LastInteractable = nullptr;

	FTimerHandle InteractableResetTimer;
	FTimerHandle TracePollingTimer;

	UPROPERTY()
	UInteractionWidget* InteractionWidget = nullptr;

	TWeakObjectPtr<AFarmerVillagerCharacter> CurrentInteractedVillager;
};

