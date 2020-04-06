#include "USBMovement.h"
#include "Actors/USB_PlayerPawn.h"

UUSBMovement::UUSBMovement(const FObjectInitializer& objInit):Super(objInit)
{
	m_fAutoMoveTimeWant = -1.f;
	m_fAutoMoveTimer = -1.f;
}

void UUSBMovement::BeginPlay()
{
	Super::BeginPlay();
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
			m_PlayerPawn->FailConnection(nullptr, nullptr, EFailConnectionReason::PortNotFoundTimeEnd);
		}
	}
}

void UUSBMovement::PhysSceneStep(FPhysScene * PhysScene, float DeltaTime)
{
	Super::PhysSceneStep(PhysScene, DeltaTime);
}

void UUSBMovement::SetUSBUpdateComponent(AUSB_PlayerPawn* playerPawn, UPhysicsSkMeshComponent * head, UPhysicsSkMeshComponent * tail)
{
	SetUpdatedComponent(head);
	m_MovingTargetTail = tail;
	m_PlayerPawn = playerPawn;
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
	m_TargetRot = portRot;
	RequestConnectChargeMove(normalHorizon, timeWant);
	PRINTF("Requested:%s", *m_TargetRot.ToString());
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
	if (Super::DoJump())
	{
		FVector CurrentV = m_MovingTarget->GetPhysicsLinearVelocity();
		CurrentV.Z = FMath::Max(m_fJumpZVelocity, CurrentV.Z);
		m_MovingTargetTail->SetPhysicsLinearVelocity(CurrentV);

		return true;
	}
	return false;
}
