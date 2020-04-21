


#include "AdaptorPawn.h"
#include "Datas/USB_Macros.h"

AAdaptorPawn::AAdaptorPawn(const FObjectInitializer& objInit):
	Super(objInit.SetDefaultSubobjectClass<UPinSkMeshComponent>(APortPawn::MeshComponentName))
{
	m_MeshPort->m_NameInitCollProfile = "USBActor";

	m_Movement = CreateDefaultSubobject<UNavPawnMovement>("Movement00");
	m_Movement->m_nJumpMaxCount = 1;

}


void AAdaptorPawn::BeginPlay()
{
	Super::BeginPlay();

	Cast<UPinSkMeshComponent>(m_Mesh)->SetMyPort(m_MeshPort);
	m_Mesh->SetGenerateOverlapEvents(true);

	if (!m_Mesh->GetSocketByName("PinPoint"))
	{
		PRINTF("!! The Mesh Doesnt Have PinPoint Socket");
	}

	m_Movement->SetMovingComponent(m_Mesh,false);
}

void AAdaptorPawn::PortConnected(UPinSkMeshComponent * pinConnect)
{
	Super::PortConnected(pinConnect);

	m_Movement->StopMovementImmediately();

	PRINTF("Move Stop");
}

void AAdaptorPawn::PortDisConnected(UPinSkMeshComponent * pinConnect)
{
	Super::PortDisConnected(pinConnect);
}
