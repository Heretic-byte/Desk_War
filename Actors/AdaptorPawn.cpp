


#include "AdaptorPawn.h"

AAdaptorPawn::AAdaptorPawn(const FObjectInitializer& objInit) :Super(objInit)
{
	m_MeshPin = CreateDefaultSubobject<UPinSkMeshComponent>(TEXT("MeshPin00"));
	m_MeshPin->SetupAttachment(m_MeshPort);
}