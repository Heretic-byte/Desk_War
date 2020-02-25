// Fill out your copyright notice in the Description page of Project Settings.


#include "PortSkMeshComponent.h"
#include "Datas/USB_Macros.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"

UPortSkMeshComponent::UPortSkMeshComponent(const FObjectInitializer & objInit)
{
	m_bBlockMoveOnConnected = false;
	m_NameWantMovePoint = "PortPoint";
	m_NameParentBonePortPoint = FName(TEXT("PortPoint"));
	m_PortType = E_PinPortType::ENoneType;
	m_fEjectPower = 999.f;
	m_fConnectableDistSqr = 25.f;
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundMesh(TEXT("SkeletalMesh'/Game/Meshes/Characters/Port/SK_PortPoint.SK_PortPoint'"));

	if (FoundMesh.Succeeded())
	{
		SetSkeletalMesh(FoundMesh.Object);
	}

	SetCollisionProfileName("Port");
}

void UPortSkMeshComponent::InitPort(UPhysicsConstraintComponent * physicsJoint, UPhysicsSkMeshComponent* parentMesh,E_PinPortType portType, FName namePinBone)
{
	m_ParentPhysicsConst = physicsJoint;
	m_MeshParentActor = parentMesh;

	if (portType != E_PinPortType::ENoneType)
	{
		m_PortType = portType;
	}

	if (namePinBone != NAME_None)
	{
		m_NameParentBonePortPoint = namePinBone;
	}
}

void UPortSkMeshComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UPortSkMeshComponent::SetAimTracePoint(FVector & tracedImpactPoint)
{
	FVector PortLoc = GetComponentLocation();
	FVector TracedImpactLoc= tracedImpactPoint;
	float DistSqr = FVector::DistSquared(PortLoc, TracedImpactLoc);
	PortLoc.X = 0.f;
	TracedImpactLoc.X = 0.f;

	float DistSqrForBlink = FVector::DistSquared(PortLoc, TracedImpactLoc);

	return DistSqr < m_fConnectableDistSqr;
}

bool UPortSkMeshComponent::GetBlockMoveOnConnnect()
{
	return m_bBlockMoveOnConnected;
}

FName UPortSkMeshComponent::GetMovePointWant()
{
	return m_NameWantMovePoint;
}

UPhysicsSkMeshComponent * UPortSkMeshComponent::GetParentSkMesh()
{
	return m_MeshParentActor;
}

void UPortSkMeshComponent::Connect(UPinSkMeshComponent * connector)
{
	m_ConnectedPin = connector;
	ConstraintPinPort();
	m_OnConnected.Broadcast(m_ConnectedPin);
}

void UPortSkMeshComponent::ConstraintPinPort()
{
	m_ParentPhysicsConst->SetConstrainedComponents(m_ConnectedPin, m_ConnectedPin->GetNameConnectPoint(), m_MeshParentActor, m_NameParentBonePortPoint);//Mesh
}


bool UPortSkMeshComponent::Disconnect()
{
	if (!m_ConnectedPin)
	{
		return false;
	}
	m_OnDisconnected.Broadcast(m_ConnectedPin);
	m_ParentPhysicsConst->BreakConstraint();
	m_ConnectedPin = nullptr;

	m_MeshParentActor->AddImpulse(m_MeshParentActor->GetForwardVector()*m_fEjectPower);
	return true;
}

bool UPortSkMeshComponent::IsConnected()
{
	return m_ConnectedPin;
}

void UPortSkMeshComponent::DisablePhysics()
{
	m_MeshParentActor->SetSimulatePhysics(false);
}

void UPortSkMeshComponent::EnablePhysics()
{
	m_MeshParentActor->SetSimulatePhysics(true);
}

E_PinPortType UPortSkMeshComponent::GetPortType() const
{
	return _inline_GetPortType();
}

