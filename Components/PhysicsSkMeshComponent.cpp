#include "PhysicsSkMeshComponent.h"
#include "Datas/USB_Macros.h"


UPhysicsSkMeshComponent::UPhysicsSkMeshComponent(const FObjectInitializer& objInit)
	:Super(objInit)
{

	OnComponentHit.AddDynamic(this, &UPhysicsSkMeshComponent::OnBlockHit);

}

void UPhysicsSkMeshComponent::OnBlockHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{


}