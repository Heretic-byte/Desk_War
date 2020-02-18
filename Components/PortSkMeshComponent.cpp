// Fill out your copyright notice in the Description page of Project Settings.


#include "PortSkMeshComponent.h"
#include "Datas/USB_Macros.h"

UPortSkMeshComponent::UPortSkMeshComponent(const FObjectInitializer & objInit)
{
	m_NamePinConnectBone = FName(TEXT("PortPoint"));
	m_PortType = E_PinPortType::ENoneType;
	m_bIsConnected = false;
}
void UPortSkMeshComponent::InitPort(UPhysicsConstraintComponent * physicsJoint, E_PinPortType portType, FName namePinBone)
{
	m_PhysicsConst = physicsJoint;

	if (portType != E_PinPortType::ENoneType)
	{
		m_PortType = portType;
	}

	if (namePinBone != NAME_None)
	{
		m_NamePinConnectBone = namePinBone;
	}
}

void UPortSkMeshComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPortSkMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
}

void UPortSkMeshComponent::Connect(UPinSkMeshComponent * connector)
{
	PRINTF("Connect in Port");

	m_ConnectedPin = connector;

	AdjustPinActorTransform();
	ConstraintPinPort();
	ResetAllBodiesSimulatePhysics();
	m_ConnectedPin->ResetAllBodiesSimulatePhysics();
	m_bIsConnected = true;
}

void UPortSkMeshComponent::AdjustPinActorTransform()
{
	FRotator ConnectRot = m_ConnectedPin->GetComponentRotation();
	SetWorldLocation(m_ConnectedPin->GetBoneLocation("PinPoint", EBoneSpaces::WorldSpace), false, nullptr, ETeleportType::TeleportPhysics);
	SetWorldRotation(ConnectRot, false, nullptr, ETeleportType::TeleportPhysics);
}

void UPortSkMeshComponent::ConstraintPinPort()
{
	m_PhysicsConst->SetConstrainedComponents(m_ConnectedPin, m_ConnectedPin->GetNameConnectPoint(), this, m_NamePinConnectBone);//Mesh
}


void UPortSkMeshComponent::Disconnect()
{
	PRINTF("Dis");
	m_PhysicsConst->BreakConstraint();
	m_ConnectedPin = nullptr;
	m_bIsConnected = false;
}

bool UPortSkMeshComponent::IsConnected()
{
	return m_bIsConnected;
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

