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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> InitialStatsGE;
	float GetXP() const;
	float GetMaxXP() const;
protected:
	virtual void BeginPlay() override;

	/** Cached Level Attribute Set containing Level and XP attributes. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	const ULevelAttributeSet* LevelAttributeSet;

	/** Cached Ability System Component */
	UPROPERTY()
	UAbilitySystemComponent* ASC;
	
private:
	/** Bind the level up delegates to functions */
	void BindDelegates();
	void CheckLevelUp(const FOnAttributeChangeData& Data) const;
	void LevelUp(int Levels) const;
	void LowerByMaxXP(int MaxXP) const;
};
