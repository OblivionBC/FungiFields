// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LevelComponent.generated.h"

class ULevelAttributeSet;
class UAbilitySystemComponent;
class UGameplayEffect;
struct FOnAttributeChangeData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API ULevelComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULevelComponent();
	
	/** Level Attribute Set containing Level and XP attributes. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	ULevelAttributeSet* LevelAttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> InitialStatsGE;
	float GetXP();
	float GetMaxXP();
protected:
	virtual void BeginPlay() override;
	
private:
	/** Bind the level up delegates to functions */
	void BindDelegates(UAbilitySystemComponent* ASC);
	void CheckLevelUp(const FOnAttributeChangeData& Data);
};
