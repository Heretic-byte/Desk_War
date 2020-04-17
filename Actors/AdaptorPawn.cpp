


#include "AdaptorPawn.h"
#include "Datas/USB_Macros.h"

AAdaptorPawn::AAdaptorPawn(const FObjectInitializer& objInit):
	Super(objInit.SetDefaultSubobjectClass<UPinSkMeshComponent>(APortPawn::MeshComponentName))
{
	m_MeshPort->m_NameInitCollProfile = "USBActor";
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
}