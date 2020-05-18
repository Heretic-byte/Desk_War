#include "DetectEscapeBehavior.h"
#include "Actors/USB_PlayerPawn.h"
#include "Components/PhysicsMovement.h"
#include "Kismet/KismetMathLibrary.h"


void UDetectEscapeBehavior::Execute(AConnectablePawn* connectPawn, float deltaTime)
{
	if (m_fMoveTimer > 0.f)
	{
		m_fMoveTimer -= deltaTime;

		return;
	}

	//발견한 플레이어를 인터범위에 들어올때까지 처다봄
	if (!connectPawn->IsPlayerInInterRadius())
	{
		FRotator LookAtRot= UKismetMathLibrary::FindLookAtRotation(connectPawn->GetActorLocation(), connectPawn->m_FoundPlayerPawn->GetActorLocation());
		FRotator Rot = FMath::RInterpTo(connectPawn->GetActorRotation(),LookAtRot,deltaTime,5.f);
		connectPawn->SetActorRotation(Rot);
	}

	//멀어지면 그만처다봄



	/*if (connectPawn->GetAICon()->GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Type::Moving)
	{
		return;
	}*/

	if (connectPawn->IsOutFromStartArea())//시작지점에서 너무 멀어젔으면
	{
		connectPawn->MoveToLocation(connectPawn->m_StartLocation);
		return;
	}

	auto* Player = Cast<AUSB_PlayerPawn>(connectPawn->m_FoundPlayerPawn);
	//도망가기

	float Radius = connectPawn->GetRadius();//인터렉션 포인트

	FVector EscapeDir = connectPawn->GetActorLocation() - Player->GetHead()->GetComponentLocation();
	EscapeDir.Normalize();

	FVector TargetPoint = EscapeDir * (Radius*2.f) + connectPawn->GetActorLocation();

	FNavLocation Result;

	if (connectPawn->GetNav()->GetRandomPointInNavigableRadius(TargetPoint,Radius, Result))
	{
		connectPawn->MoveToLocation(Result);
		Cast< UPhysicsMovement >(connectPawn->GetMovementComponent())->Jump();
		m_fMoveTimer = 2.f;
	}

	

	//너무 멀어지면 리턴?

	//내 영역에서 벗어나거나

	//플레이어 더이상 쫓아오지 않으면?

	//
}