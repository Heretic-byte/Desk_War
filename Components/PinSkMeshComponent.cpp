// Fill out your copyright notice in the Description page of Project Settings.


#include "PinSkMeshComponent.h"
#include "GameFramework/Controller.h"
#include "Components/PortSkMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Datas/USB_Macros.h"


UPinSkMeshComponent::UPinSkMeshComponent(const FObjectInitializer& objInit) :Super(objInit)
{
	m_PortConnected = nullptr;
	m_PortOwner = nullptr;
	m_NameNeckBone = FName(TEXT("Root"));
	m_NamePinBone = FName(TEXT("PinPoint"));
	m_NameVelocityPivotBone = FName(TEXT("PinPoint"));
	m_Pintype = E_PinPortType::ENoneType;

	for (auto& b : m_AryTypeMatch)
	{
		b = false;
	}
}

void UPinSkMeshComponent::SetPinType(E_PinPortType pinType)
{
	m_AryTypeMatch[(int)m_Pintype] = false;
	m_Pintype = pinType;
	m_AryTypeMatch[(int)m_Pintype] = true;
}

FVector UPinSkMeshComponent::GetNeckLoc() const
{
	return GetSocketLocation(m_NameNeckBone);
}

bool UPinSkMeshComponent::Connect(UPortSkMeshComponent * port)
{
	if (!CheckTypeMatch(port->GetPortType()))
	{
		return false;
	}
	
	m_PortConnected = port;
	m_PortOwner = m_PortConnected->GetOwner();
	m_PortConnected->Connect(this);

	m_OnConnectedPortOwner.Broadcast(m_PortOwner);
	m_OnConnectedPort.Broadcast(m_PortConnected);

	
	return true;
}

bool UPinSkMeshComponent::Disconnect()
{
	if (!m_PortConnected->Disconnect())
	{
		return false;
	}

	m_PortConnected = nullptr;
	m_PortOwner = nullptr;

	return true;
}

void UPinSkMeshComponent::BeginPlay()
{
	Super::BeginPlay();
	SetPinType(m_Pintype);
}



bool UPinSkMeshComponent::CheckTypeMatch(E_PinPortType portsType)
{
	return m_AryTypeMatch[(int)portsType];
}

void UPinSkMeshComponent::SetNeckName(FName nameWant)
{
	m_NameNeckBone = FName(nameWant);
}

void UPinSkMeshComponent::SetVelocityPivotName(FName nameWant)
{
	m_NameVelocityPivotBone = nameWant;
}




