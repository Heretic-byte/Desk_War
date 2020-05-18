


#include "IdleBehavior.h"
#include "Datas/USB_Macros.h"



void UIdleBehavior::Execute(AConnectablePawn* connectPawn, float deltaTime)
{
	if (connectPawn->m_FoundPlayerPawn)
	{
		connectPawn->GetMovementComponent()->StopMovementImmediately();
		connectPawn->SetFSM(AConnectablePawn::EFSM::Detect);
		PRINTF("Detect Player !!");
	}
}