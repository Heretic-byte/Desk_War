#include "RoamGroundBehavior.h"
#include "AIController.h"
#include "Datas/USB_Macros.h"
#include "Components/PhysicsMovement.h"



void URoamGroundBehavior::Execute(AConnectablePawn* connectPawn, float deltaTime)
{

	if (m_fRoamCooldown > 0.f)
	{
		m_fRoamCooldown -= deltaTime;

		return;
	}

	if (connectPawn->GetAICon()->GetPathFollowingComponent()->GetStatus() ==EPathFollowingStatus::Type::Moving)
	{
		return;
	}



	FNavLocation ResultLoc;
	if (connectPawn->GetNav()->GetRandomReachablePointInRadius(connectPawn->m_StartLocation, connectPawn->GetRadius(), ResultLoc))
	{
		connectPawn->MoveToLocation(ResultLoc);

		m_fRoamCooldown = 1.f;//FMath::FRandRange(3.f,8.f);
		Cast<UPhysicsMovement>( connectPawn->GetMovementComponent())->Jump();
		PRINTF("SetDest");
	}


	if (connectPawn->m_FoundPlayerPawn)
	{
		connectPawn->GetMovementComponent()->StopMovementImmediately();
		connectPawn->SetFSM(AConnectablePawn::EFSM::Detect);
	}

	
}