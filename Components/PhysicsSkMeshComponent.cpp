


#include "PhysicsSkMeshComponent.h"

UPhysicsSkMeshComponent::UPhysicsSkMeshComponent(const FObjectInitializer& objInit)
	:Super(objInit)
{
	m_BoundingCapsule = CreateDefaultSubobject<UCapsuleComponent>("BoundingCapsule");
	m_BoundingCapsule->SetupAttachment(this);
	m_BoundingCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	m_BoundingCapsule->SetEnableGravity(false);
	m_BoundingCapsule->SetHiddenInGame(true);

	m_fMeshRadiusMultiple = 1.f;
}

float UPhysicsSkMeshComponent::GetMeshRadiusMultiple()
{
	return m_fMeshRadiusMultiple;
}

void UPhysicsSkMeshComponent::SetMeshRadiusMultiple(float vM)
{
	m_fMeshRadiusMultiple = vM;
}
