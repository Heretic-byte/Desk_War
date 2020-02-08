// Fill out your copyright notice in the Description page of Project Settings.


#include "Connector.h"
#include "Components/Port.h"
#include "UObject/ConstructorHelpers.h"


UConnector::UConnector(const FObjectInitializer& objInit):Super(objInit)
{
	m_PortConnected = nullptr;
	m_PortOwner = nullptr;
	m_NameNeckBone = FName(TEXT("joint9"));
	m_NameConnectSocket = FName(TEXT("ConnectPoint"));
	m_NameConnectStartSocket = FName(TEXT("ConnectStart"));
	m_NameConnectPushPointSocket = FName(TEXT("PushPoint"));
	CreateHeadMesh();
}


void UConnector::CreateHeadMesh()
{
	m_MeshPin=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh01"));
	m_MeshPin->SetupAttachment(this);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundMeshPortUSB(TEXT("SkeletalMesh'/Game/01_FinalUSB/Mesh/Head/NewHead0120/USB_Head_Mesh_06.USB_Head_Mesh_06'"));
	check(FoundMeshPortUSB.Object);

	m_MeshPin->SetSkeletalMesh( FoundMeshPortUSB.Object);
	m_MeshPin->SetCollisionProfileName(TEXT("USBMovement"));
}

FVector UConnector::GetNeckLoc() const
{
	return m_MeshPin->GetSocketLocation(m_NameNeckBone);
}

void UConnector::SetSimulatePhysics(bool v)
{
	m_MeshPin->SetSimulatePhysics(v);
}

void UConnector::Connect(UPort * port)
{
	m_PortConnected = port;
	m_PortOwner = m_PortConnected->GetOwner();
	m_PortConnected->Connect(this);

	m_OnConnectedPortOwner.Broadcast(m_PortOwner);
	m_OnConnectedPort.Broadcast(m_PortConnected);
}


