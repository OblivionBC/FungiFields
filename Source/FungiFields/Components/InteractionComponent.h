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

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact(const FInputActionValue& Value);

	/** Should be called from the owner's BeginPlay after components are initialized. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetCamera(UCameraComponent* Camera);

protected:
	virtual void BeginPlay() override;

	void TraceForInteractable();
	void ClearInteractable();
	void ShowInteractionWidget(AActor* Interactable, const FText& Prompt);
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

