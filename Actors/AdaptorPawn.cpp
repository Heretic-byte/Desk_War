


#include "AdaptorPawn.h"

AAdaptorPawn::AAdaptorPawn(const FObjectInitializer& objInit) :Super(objInit
.SetDefaultSubobjectClass<UPinSkMeshComponent>(APortPawn::MeshComponentName))
{

}


void AAdaptorPawn::BeginPlay()
{
	Super::BeginPlay();

	Cast<UPinSkMeshComponent>(m_Mesh)->SetMyPort(m_MeshPort);
}