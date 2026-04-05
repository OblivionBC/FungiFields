#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class AActor;
class UCameraComponent;
class UInteractionWidget;
class UInputAction;
struct FInputActionValue;

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
};

