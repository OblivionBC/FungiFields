#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LevelComponent.generated.h"

class ULevelAttributeSet;
class UAbilitySystemComponent;
class UGameplayEffect;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLevelUp, int32, NewLevel);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FUNGIFIELDS_API ULevelComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULevelComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> InitialStatsGE;

	/** Fired whenever the player gains a level. USkillTreeComponent binds this automatically. */
	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnPlayerLevelUp OnLevelUp;

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
