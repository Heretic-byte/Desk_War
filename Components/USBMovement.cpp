


#include "USBMovement.h"

void UUSBMovement::SetUSBUpdateComponent(UPhysicsSkMeshComponent * head, UPhysicsSkMeshComponent * tail)
{
	SetUpdatedComponent(head);
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

void UUSBMovement::BeginPlay()
{
	Super::BeginPlay();
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
