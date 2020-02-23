// Fill out your copyright notice in the Description page of Project Settings.


#include "PortSkMeshComponent.h"
#include "Datas/USB_Macros.h"
#include "UObject/ConstructorHelpers.h"

UPortSkMeshComponent::UPortSkMeshComponent(const FObjectInitializer & objInit)
{
	m_NameWantMovePoint = "PortPoint";
	m_NameParentBonePortPoint = FName(TEXT("PortPoint"));
	m_PortType = E_PinPortType::ENoneType;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundMesh(TEXT("SkeletalMesh'/Game/Meshes/Characters/Port/SK_PortPoint.SK_PortPoint'"));

	if (FoundMesh.Succeeded())
	{
		SetSkeletalMesh(FoundMesh.Object);
	}

	SetCollisionProfileName("Port");
}

void UPortSkMeshComponent::InitPort(UPhysicsConstraintComponent * physicsJoint, USkeletalMeshComponent* parentMesh,E_PinPortType portType, FName namePinBone)
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

FName UPortSkMeshComponent::GetMovePointWant()
{
	return m_NameWantMovePoint;
}

USkeletalMeshComponent * UPortSkMeshComponent::GetParentSkMesh()
{
	return m_MeshParentActor;
}

void UPortSkMeshComponent::Connect(UPinSkMeshComponent * connector)
{
	PRINTF("Connect in Port");
	m_ConnectedPin = connector;
	AdjustPinActorTransform();
	GetParentSkMesh()->ResetAllBodiesSimulatePhysics();
	ResetAllBodiesSimulatePhysics();
	ConstraintPinPort();
}

void UPortSkMeshComponent::AdjustPinActorTransform()
{
	FRotator ConnectRot = m_ConnectedPin->GetComponentRotation();
	GetParentSkMesh()->SetWorldLocation(m_ConnectedPin->GetBoneLocation("PinPoint", EBoneSpaces::WorldSpace), false, nullptr, ETeleportType::TeleportPhysics);
	GetParentSkMesh()->SetWorldRotation(ConnectRot, false, nullptr, ETeleportType::TeleportPhysics);
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
	PRINTF("Dis");
	m_ParentPhysicsConst->BreakConstraint();
	m_ConnectedPin = nullptr;

	return true;
}

bool UPortSkMeshComponent::IsConnected()
{
	return m_ConnectedPin;
}

void UPortSkMeshComponent::DisableCollider()
{
	SetSimulatePhysics(false);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UPortSkMeshComponent::EnableCollider()
{
	SetSimulatePhysics(true);
	SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
}

E_PinPortType UPortSkMeshComponent::GetPortType() const
{
	return _inline_GetPortType();
}

