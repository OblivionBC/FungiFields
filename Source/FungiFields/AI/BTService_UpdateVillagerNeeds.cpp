#include "BTService_UpdateVillagerNeeds.h"
#include "FarmerBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../Characters/FarmerVillagerCharacter.h"
#include "../Components/UVillagerNeedsComponent.h"

UBTService_UpdateVillagerNeeds::UBTService_UpdateVillagerNeeds()
{
	NodeName = TEXT("Update Villager Needs");
	// Run every 2 seconds — hunger changes on game-hour boundaries so frequent ticking is wasteful.
	Interval = 2.f;
	RandomDeviation = 0.5f;
	bNotifyTick = true;
}

void UBTService_UpdateVillagerNeeds::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController*        AIC = OwnerComp.GetAIOwner();
	AFarmerVillagerCharacter* Villager = AIC ? Cast<AFarmerVillagerCharacter>(AIC->GetPawn()) : nullptr;

	if (!BB || !Villager) return;

	// Sync home location once (doesn't change, but keeps BB fresh after respawns).
	BB->SetValueAsVector(FarmerBBKeys::HomeLocation, Villager->GetHomeLocation());

	UVillagerNeedsComponent* Needs = Villager->GetNeedsComponent();
	if (!Needs) return;

	BB->SetValueAsBool  (FarmerBBKeys::bIsStarving, Needs->IsStarving());
	BB->SetValueAsBool  (FarmerBBKeys::bIsHungry,   Needs->IsHungry());
	BB->SetValueAsFloat (FarmerBBKeys::WorkSpeed,    Needs->GetWorkSpeedMultiplier());
}
