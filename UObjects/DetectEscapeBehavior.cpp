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

	//�߰��� �÷��̾ ���͹����� ���ö����� ó�ٺ�
	if (!connectPawn->IsPlayerInInterRadius())
	{
		FRotator LookAtRot= UKismetMathLibrary::FindLookAtRotation(connectPawn->GetActorLocation(), connectPawn->m_FoundPlayerPawn->GetActorLocation());
		FRotator Rot = FMath::RInterpTo(connectPawn->GetActorRotation(),LookAtRot,deltaTime,5.f);
		connectPawn->SetActorRotation(Rot);
	}

	//�־����� �׸�ó�ٺ�



	/*if (connectPawn->GetAICon()->GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Type::Moving)
	{
		return;
	}*/

	if (connectPawn->IsOutFromStartArea())//������������ �ʹ� �־����
	{
		connectPawn->MoveToLocation(connectPawn->m_StartLocation);
		return;
	}

	auto* Player = Cast<AUSB_PlayerPawn>(connectPawn->m_FoundPlayerPawn);
	//��������

	float Radius = connectPawn->GetRadius();//���ͷ��� ����Ʈ

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

	

	//�ʹ� �־����� ����?

	//�� �������� ����ų�

	//�÷��̾� ���̻� �Ѿƿ��� ������?

	//
}