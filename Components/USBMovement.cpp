#include "USBMovement.h"
#include "Actors/USB_PlayerPawn.h"

UUSBMovement::UUSBMovement(const FObjectInitializer& objInit):Super(objInit)
{
	m_fAutoMoveTimeWant = -1.f;
	m_fAutoMoveTimer = -1.f;
	m_fInitHeadMass = 2.5f;
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
}
bool UUSBMovement::DoJump()
{
	if (CanJump())
	{
		PRINTF("DidJump");
		m_nJumpCurrentCount++;
		PRINTF("JumpCount:%d", m_nJumpCurrentCount);

		FVector CurrentV = m_MovingTarget->GetPhysicsLinearVelocity();
		CurrentV.Z = FMath::Max(m_fJumpZVelocity, CurrentV.Z);
		float CurrentHeadMass = m_MovingTarget->GetBodyInstance()->GetBodyMass();
		float CurrentTailMass = m_MovingTargetTail->GetBodyInstance()->GetBodyMass();
		float HeadMassRate = CurrentHeadMass / m_fInitHeadMass;
		float TailMassRate = CurrentTailMass / m_fInitHeadMass;

		m_MovingTarget->SetPhysicsLinearVelocity(CurrentV*TailMassRate);
		m_MovingTargetTail->SetPhysicsLinearVelocity(CurrentV *HeadMassRate);

		return true;
	}
	return false;
}
