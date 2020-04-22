#include "USBMovement.h"
#include "Actors/USB_PlayerPawn.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"



UUSBMovement::UUSBMovement(const FObjectInitializer& objInit):Super(objInit)
{
	m_fJumpHeight = 20.f;
	m_fAutoMoveTimeWant = -1.f;
	m_fAutoMoveTimer = -1.f;
	m_fInitHeadMass = 2.5f;
	m_PlayerPawn = nullptr;
	m_NameSweepProfileName = "USBActor";
}

void UUSBMovement::BeginPlay()
{
	Super::BeginPlay();
	m_fInitHeadMass = m_MovingTarget->GetBodyInstance()->GetBodyMass();
}

void UUSBMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (m_fAutoMoveTimer < m_fAutoMoveTimeWant)
	{
		m_fAutoMoveTimer += DeltaTime;
		RequestPathMove(m_fAutoMoveInput);

		if (m_fAutoMoveTimer >= m_fAutoMoveTimeWant)
		{
			m_PlayerPawn->FailConnection(nullptr,nullptr,EFailConnectionReason::PortNotFoundTimeEnd); 
		}
	}
}

void UUSBMovement::PhysSceneStep(FPhysScene * PhysScene, float DeltaTime)
{
	Super::PhysSceneStep(PhysScene, DeltaTime);
}

void UUSBMovement::InitUSBUpdateComponent(AUSB_PlayerPawn* playerPawn, UPhysicsSkMeshComponent * head, UPhysicsSkMeshComponent * tail)
{
	SetUSBUpdateComponent(head, tail,false);
	m_PlayerPawn = playerPawn;
}

void UUSBMovement::SetUSBUpdateComponent(UPhysicsSkMeshComponent * head, UPhysicsSkMeshComponent * tail, bool bRemoveTraceOld)
{
	SetMovingComponent(head,bRemoveTraceOld);
	m_MovingTargetTail = tail;
}

void UUSBMovement::AddForce(FVector forceWant)
{
	Super::AddForce(forceWant);
	m_MovingTargetTail->AddForce(forceWant);
}

void UUSBMovement::AddImpulse(FVector impulseWant)
{
	Super::AddImpulse(impulseWant);
	m_MovingTargetTail->AddImpulse(impulseWant);
}

void UUSBMovement::RequestConnectChargeMove(const FVector & normalHorizon, float timeWant)
{
	m_fAutoMoveInput = normalHorizon;
	m_fAutoMoveTimeWant = timeWant;
	m_fAutoMoveTimer = 0;
}

void UUSBMovement::RequestAirConnectChargeMove(FRotator portRot, const FVector & normalHorizon, float timeWant)
{
	m_OnAirTargetRot = portRot;
	RequestConnectChargeMove(normalHorizon, timeWant);
}

void UUSBMovement::StopUSBMove()
{
	m_fAutoMoveTimeWant = 0.f;
	m_fAutoMoveTimer = 0.f;
	m_fAutoMoveInput = FVector::ZeroVector;
	PRINTF("StopAutoMove");

	StopMovementImmediately();
	ClearAirRotation();
}
void UUSBMovement::ClearAirRotation()
{
	m_OnAirTargetRot = FRotator::ZeroRotator;
}
bool UUSBMovement::DoJump()
{
 	if (CanJump())
	{
		m_nJumpCurrentCount++;
		PRINTF("JumpCount:%d", m_nJumpCurrentCount);

		if (m_nJumpCurrentCount == 1)
		{
			m_OnJumpBP.Broadcast();
		}
		else
		{
			m_OnJumpSeveralBP.Broadcast(m_nJumpCurrentCount);
		}

		FVector CurrentV = m_MovingTarget->GetPhysicsLinearVelocity();
		float ZVelo= FMath::Sqrt(-2.f *GetGravityZ() * m_fJumpHeight);
		CurrentV.Z = FMath::Max(ZVelo, CurrentV.Z);

		for (auto Phy : m_PlayerPawn->GetPhysicsAry())
		{
			if (Phy == m_PlayerPawn->GetHead())
			{
				float Weight = ((m_PlayerPawn->GetTail()->GetMass() / m_PlayerPawn->GetHead()->GetMass()) / 10.f + 1.f);

				Phy->SetPhysicsLinearVelocity(CurrentV*Weight);
			}
			else
			{
				Phy->SetPhysicsLinearVelocity(CurrentV);
			}
		}

		return true;
	}
	return false;
}
