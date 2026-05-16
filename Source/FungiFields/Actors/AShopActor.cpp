#include "AShopActor.h"
#include "Components/StaticMeshComponent.h"
#include "../Components/UShopComponent.h"

AShopActor::AShopActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	ShopComponent = CreateDefaultSubobject<UShopComponent>(TEXT("ShopComponent"));
}

void AShopActor::Interact_Implementation(AActor* Interactor)
{
	if (ShopComponent)
		ShopComponent->OpenShopWidget(Interactor);
}

FText AShopActor::GetInteractionText_Implementation()
{
	return FText::Format(NSLOCTEXT("Shop", "InteractText", "Open {0}"),
		ShopComponent ? ShopComponent->GetShopName() : FText::GetEmpty());
}

FText AShopActor::GetTooltipText_Implementation() const
{
	return ShopComponent ? ShopComponent->GetShopName() : FText::GetEmpty();
}
