// Fill out your copyright notice in the Description page of Project Settings.


#include "Port.h"
#include "Components/Connector.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values for this component's properties
UPort::UPort()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPort::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPort::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPort::SetCollider(UShapeComponent * coll)
{
	m_CollConnector = coll;
}

void UPort::SetPrmitiveMesh(UPrimitiveComponent * primi)
{
	m_PrmiMeshConnector = primi;
}

void UPort::SetConstraint(UPhysicsConstraintComponent * constraint)
{
	m_Constraint = constraint;
}

void UPort::Connect(UConnector * connector)
{
	m_ConnectedConnector = connector;
	m_OnConnected.Broadcast(m_ConnectedConnector);
	EnableCollider();
	BindConstraintConnector(connector->GetMesh());
	m_PrmiMeshConnector->SetMassOverrideInKg(NAME_None, 50);
}

void UPort::Disconnect( )
{
	m_OnDisconnected.Broadcast(m_ConnectedConnector);
	m_ConnectedConnector = nullptr;
}

void UPort::DisableCollider()
{
	m_PrmiMeshConnector->SetSimulatePhysics(false);
	m_PrmiMeshConnector->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UPort::EnableCollider()
{
	m_PrmiMeshConnector->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	m_PrmiMeshConnector->SetSimulatePhysics(true);
}

bool UPort::IsConnected()
{
	return m_ConnectedConnector;
}

void UPort::OverlapBlock(USceneComponent * collider, USceneComponent * portMesh)
{
	FVector NormalWant = collider->GetComponentLocation() - portMesh->GetComponentLocation();
	NormalWant = NormalWant.GetSafeNormal2D();

	//UKismetMathLibrary::InverseTransformDirection()
}

void UPort::BindConstraintConnector(USkeletalMeshComponent * meshWant)
{
	m_Constraint->SetConstrainedComponents(m_PrmiMeshConnector, NAME_None,meshWant,NAME_None);
}
