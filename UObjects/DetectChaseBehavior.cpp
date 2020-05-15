#include "DetectChaseBehavior.h"
#include "Actors/USB_PlayerPawn.h"


void UDetectChaseBehavior::Execute(AConnectablePawn* connectPawn, float deltaTime)
{
	connectPawn->MoveToLocation(Cast<AUSB_PlayerPawn>( connectPawn->m_FoundPlayerPawn)->GetHead()->GetComponentLocation());



}