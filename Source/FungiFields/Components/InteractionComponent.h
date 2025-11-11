#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class AActor;
class UCameraComponent;
class UInteractionWidget;
class UInputAction;
struct FInputActionValue;

/**
 * Component responsible for handling player interaction with interactable actors.
 * Performs line traces, manages interaction widgets, and handles interaction input.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent(const FObjectInitializer& ObjectInitializer);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Called when the interact input action is triggered.
	 * Performs a line trace and executes interaction on hit actors.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact(const FInputActionValue& Value);

	/**
	 * Sets the camera component to use for interaction traces.
	 * Should be called from the owner's BeginPlay after components are initialized.
	 * @param Camera The camera component to use for line traces
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetCamera(UCameraComponent* Camera);

protected:
	virtual void BeginPlay() override;

	/**
	 * Performs a line trace from the camera to detect interactable actors.
	 * Called every frame in TickComponent.
	 */
	void TraceForInteractable();

	/**
	 * Clears the current interactable reference and hides the widget.
	 * Called by timer when no interactable is detected.
	 */
	void ClearInteractable();

	/**
	 * Shows the interaction widget with the specified prompt text.
	 * @param Interactable The actor that can be interacted with
	 * @param Prompt The text to display in the interaction widget
	 */
	void ShowInteractionWidget(AActor* Interactable, const FText& Prompt);

	/**
	 * Hides the interaction widget.
	 */
	void HideInteractionWidget();

protected:
	/** Widget class to use for displaying interaction prompts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TSubclassOf<UUserWidget> InteractionWidgetClass;

	/** Maximum distance for interaction line traces */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0"))
	float TraceDistance = 575.0f;

	/** Delay in seconds before clearing the widget when no interactable is detected */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0"))
	float ClearDelay = 3.0f;

private:
	/** Cached reference to the camera component for line traces */
	UPROPERTY()
	UCameraComponent* CameraComponent = nullptr;

	/** The currently focused interactable actor */
	UPROPERTY()
	AActor* LastInteractable = nullptr;

	/** Timer handle for clearing the widget after losing focus */
	FTimerHandle InteractableResetTimer;

	/** Instance of the interaction widget */
	UPROPERTY()
	UInteractionWidget* InteractionWidget = nullptr;
};

