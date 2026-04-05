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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	const ULevelAttributeSet* LevelAttributeSet;

	UPROPERTY()
	UAbilitySystemComponent* ASC;

private:
	void BindDelegates();
	void CheckLevelUp(const FOnAttributeChangeData& Data);
	void LevelUp(int Levels);
	void LowerByMaxXP(int MaxXP);
};
