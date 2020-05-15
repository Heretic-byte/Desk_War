#include "DetectEscapeBehavior.h"
#include "Actors/USB_PlayerPawn.h"


void UDetectEscapeBehavior::Execute(AConnectablePawn* connectPawn, float deltaTime)
{
	if (m_fMoveTimer > 0.f)
	{
		m_fMoveTimer -= deltaTime;

		return;
	}

	/*if (connectPawn->GetAICon()->GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Type::Moving)
	{
		return;
	}*/

	if (connectPawn->IsOutFromStartArea())
	{
		connectPawn->MoveToLocation(connectPawn->m_StartLocation);
		return;
	}

	auto* Player = Cast<AUSB_PlayerPawn>(connectPawn->m_FoundPlayerPawn);

	float Radius = connectPawn->GetRadius();

	FVector EscapeDir = connectPawn->GetActorLocation() - Player->GetHead()->GetComponentLocation();
	EscapeDir.Normalize();

	FVector TargetPoint = EscapeDir * (Radius*2.f) + connectPawn->GetActorLocation();

	FNavLocation Result;

	if (connectPawn->GetNav()->GetRandomPointInNavigableRadius(TargetPoint,Radius, Result))
	{
		connectPawn->MoveToLocation(Result);
		//connectPawn->TestPlayAnimation();
		m_fMoveTimer = 1.f;
	}

	

	//너무 멀어지면 리턴?

	//내 영역에서 벗어나거나

	//플레이어 더이상 쫓아오지 않으면?

	//
}