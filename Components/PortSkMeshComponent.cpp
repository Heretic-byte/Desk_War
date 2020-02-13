// Fill out your copyright notice in the Description page of Project Settings.


#include "PortSkMeshComponent.h"
#include "Datas/USB_Macros.h"

UPortSkMeshComponent::UPortSkMeshComponent(const FObjectInitializer & objInit)
{
	m_NamePinConnectBone = FName(TEXT("ConnectPoint"));
	m_PortType = E_PinPortType::ENoneType;
	SetCollisionProfileName("ConnectableObject");
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
	m_ConnectedPin = connector;

	DisableCollider();
	FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
	EnableCollider();

	this->AttachToComponent(m_ConnectedPin, Rules, m_NamePinConnectBone);//boneName SHould Be USB point not port's point

}

void UPortSkMeshComponent::Disconnect()
{
}

bool UPortSkMeshComponent::IsConnected()
{
	return false;
}


void UPortSkMeshComponent::DisableCollider()
{
	SetSimulatePhysics(false);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}

void UPortSkMeshComponent::EnableCollider()
{
	SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	SetCollisionProfileName("ConnectedObject");
}

void UPortSkMeshComponent::BindConstraintConnector(USkeletalMeshComponent * connectorMesh)
{
}
E_PinPortType UPortSkMeshComponent::GetPortType() const
{
	return _inline_GetPortType();
}

