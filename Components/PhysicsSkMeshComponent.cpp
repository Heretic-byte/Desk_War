


#include "PhysicsSkMeshComponent.h"

UPhysicsSkMeshComponent::UPhysicsSkMeshComponent(const FObjectInitializer& objInit)
	:Super(objInit)
{
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
