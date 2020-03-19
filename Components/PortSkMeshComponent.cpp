// Fill out your copyright notice in the Description page of Project Settings.


#include "PortSkMeshComponent.h"
#include "Datas/USB_Macros.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

UPortSkMeshComponent::UPortSkMeshComponent(const FObjectInitializer & objInit)
{
	m_fFailImpulsePower = 10000.f;
	m_ConnectableRotation.Yaw = 30.f;
	m_ConnectableRotation.Roll = 5.f;
	m_ConnectableRotation.Pitch = 5.f;
	m_bBlockMoveOnConnected = false;
	m_NameWantMovePoint = "PortPoint";
	m_NameParentBonePortPoint = "PortPoint";
	m_NamePortConnectSocket = "PortPoint";
	m_NamePortConnectStartSocket = "ConnectStart";
	m_NamePortConnectPushPointSocket = "PushPoint";
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

void UPortSkMeshComponent::BeginPlay()
{
	Super::BeginPlay();
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

bool UPortSkMeshComponent::CheckConnectTransform(UPinSkMeshComponent * connector)
{
	FRotator PortRot = GetComponentRotation();
	FRotator PinRot = connector->GetComponentRotation();

	float PitchDiff = FMath::Abs(PortRot.Pitch - PinRot.Pitch);
	bool PitchCheck = PitchDiff <= m_ConnectableRotation.Pitch;

	float RollDiff = FMath::Abs(PortRot.Roll - PinRot.Roll);
	bool RollCheck = RollDiff <= m_ConnectableRotation.Roll;

	float YawDiff = FMath::Abs(PortRot.Yaw - PinRot.Yaw);
	bool YawCheck = YawDiff <= m_ConnectableRotation.Yaw;

	return PitchCheck && RollCheck&&YawCheck;
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

void UPortSkMeshComponent::Connect(UPinSkMeshComponent * connector)//should call last
{
	m_ConnectedPin = connector;
	
	ConstraintPinPort();
	m_OnConnected.Broadcast(m_ConnectedPin);
}

void UPortSkMeshComponent::ConstraintPinPort()
{
	m_ParentPhysicsConst->SetConstrainedComponents(m_MeshParentActor, NAME_None, m_ConnectedPin, NAME_None);
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

	m_MeshParentActor->AddImpulse(GetForwardVector()*m_fEjectPower);

	return true;
}

bool UPortSkMeshComponent::IsConnected()
{
	return m_ConnectedPin;
}


void UPortSkMeshComponent::EnablePhysicsCollision()
{
	m_MeshParentActor->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);
}

void UPortSkMeshComponent::DisblePhysicsCollision()
{
	m_MeshParentActor->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
}

E_PinPortType UPortSkMeshComponent::GetPortType() const
{
	return _inline_GetPortType();
}

void UPortSkMeshComponent::FailConnection(const FHitResult & hitResult)
{
	EnablePhysicsCollision();
	
	m_MeshParentActor->AddImpulseAtLocation((GetUpVector()+GetForwardVector())*m_fFailImpulsePower, hitResult.ImpactPoint);
}


