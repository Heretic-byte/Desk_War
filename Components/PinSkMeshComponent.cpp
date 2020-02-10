// Fill out your copyright notice in the Description page of Project Settings.


#include "PinSkMeshComponent.h"

#include "Components/Port.h"
#include "UObject/ConstructorHelpers.h"


UPinSkMeshComponent::UPinSkMeshComponent(const FObjectInitializer& objInit) :Super(objInit)
{
	m_PortConnected = nullptr;
	m_PortOwner = nullptr;
	m_NameNeckBone = FName(TEXT("joint9"));
	m_NameConnectSocket = FName(TEXT("ConnectPoint"));
	m_NameConnectStartSocket = FName(TEXT("ConnectStart"));
	m_NameConnectPushPointSocket = FName(TEXT("PushPoint"));
	m_Pintype = E_PinPortType::ENoneType;

	for (auto& b : m_AryTypeMatch)
	{
		b = false;
	}
}

FVector UPinSkMeshComponent::GetNeckLoc() const
{
	return GetSocketLocation(m_NameNeckBone);
}

void UPinSkMeshComponent::Connect(UPort * port)
{
	if (!CheckTypeMatch(port->GetPortType()))
	{
		return;
	}

	m_PortConnected = port;
	m_PortOwner = m_PortConnected->GetOwner();
	m_PortConnected->Connect(this);

	m_OnConnectedPortOwner.Broadcast(m_PortOwner);
	m_OnConnectedPort.Broadcast(m_PortConnected);
}

void UPinSkMeshComponent::BeginPlay()
{
	Super::BeginPlay();
	SetTypeMatch();

}

void UPinSkMeshComponent::SetTypeMatch()
{
	m_AryTypeMatch[(int)m_Pintype] = true;
}

bool UPinSkMeshComponent::CheckTypeMatch(E_PinPortType portsType)
{
	return m_AryTypeMatch[(int)portsType];
}

void UPinSkMeshComponent::SetNeckName(FName nameWant)
{
	m_NameNeckBone = FName(nameWant);
}



