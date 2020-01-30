// Fill out your copyright notice in the Description page of Project Settings.


#include "Connector.h"
#include "Components/Port.h"
// Sets default values for this component's properties
UConnector::UConnector()
{


}


// Called when the game starts
void UConnector::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UConnector::Connect(UPort * port)
{
	m_PortConnected = port;
	m_PortOwner = m_PortConnected->GetOwner();
	m_PortConnected->Connect(this);
}


