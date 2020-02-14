// Fill out your copyright notice in the Description page of Project Settings.


#include "PortSkMeshComponent.h"
#include "Datas/USB_Macros.h"

UPortSkMeshComponent::UPortSkMeshComponent(const FObjectInitializer & objInit)
{
	m_NamePinConnectBone = FName(TEXT("PortPoint"));
	m_PortType = E_PinPortType::ENoneType;
	m_bIsConnected = false;
	SetCollisionProfileName("ConnectableObject");
	CreatePhysicsConst();

}

void UPortSkMeshComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPortSkMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
}


void UPortSkMeshComponent::CreatePhysicsConst()
{
	m_PhysicsConst = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("Physics00"));

	//m_PhysicsConst->SetupAttachment(this->GetAttachmentRoot());
	m_PhysicsConst->SetMobility(EComponentMobility::Movable);
	m_PhysicsConst->SetRelativeLocation(this->GetComponentLocation());
	//
	m_PhysicsConst->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhysicsConst->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhysicsConst->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhysicsConst->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhysicsConst->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhysicsConst->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
}

void UPortSkMeshComponent::Connect(UPinSkMeshComponent * connector)
{
	PRINTF("Con");
	m_ConnectedPin = connector;

	DisableCollider();
	FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
	EnableCollider();

	FName ASD= m_ConnectedPin->GetSocketBoneName(m_ConnectedPin->GetNameConnectPoint());
	FVector DDD = GetBoneLocation(m_NamePinConnectBone);
	PRINTF("VV : %s", *DDD.ToString());
	m_PhysicsConst->SetConstrainedComponents(this, NAME_None, m_ConnectedPin, ASD);//Mesh

	m_bIsConnected = true;
}

void UPortSkMeshComponent::Disconnect()
{
	PRINTF("Dis");
	m_PhysicsConst->BreakConstraint();
	m_ConnectedPin = nullptr;
	m_bIsConnected = false;
	SetCollisionProfileName("ConnectableObject");
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
	//SetCollisionProfileName("ConnectedObject");
}

void UPortSkMeshComponent::BindConstraintConnector(USkeletalMeshComponent * connectorMesh)
{
}
E_PinPortType UPortSkMeshComponent::GetPortType() const
{
	return _inline_GetPortType();
}

