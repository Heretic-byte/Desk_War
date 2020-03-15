// Fill out your copyright notice in the Description page of Project Settings.


#include "PortPawn.h"
#include "ConstructorHelpers.h"

FName APortPawn::MeshComponentName(TEXT("MeshPawn00"));
FName APortPawn::MeshPortComponentName(TEXT("MeshPort00"));
// Sets default values
APortPawn::APortPawn(const FObjectInitializer& objInit):Super(objInit)
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root00"));

	CreateMesh();
	CreatePhyCon();
	CreatePort();
}

void APortPawn::CreateMesh()
{
	m_Mesh = CreateDefaultSubobject<UPhysicsSkMeshComponent>(MeshComponentName);
	m_Mesh->SetupAttachment(RootComponent);
	//RootComponent = m_Mesh;
	m_Mesh->SetCollisionProfileName("PhysicsActor");
	m_Mesh->SetSimulatePhysics(true);
	m_Mesh->SetMeshRadiusMultiple(1.2f);
}

void APortPawn::CreatePhyCon()
{
	m_PhyConPort = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhyCon00"));
	m_PhyConPort->SetupAttachment(m_Mesh);
	m_PhyConPort->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhyConPort->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhyConPort->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhyConPort->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhyConPort->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhyConPort->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhyConPort->SetDisableCollision(true);
}

void APortPawn::PortConnected(UPinSkMeshComponent* pinConnect)
{

}

void APortPawn::PortDisConnected(UPinSkMeshComponent* pinConnect)
{

}

void APortPawn::CreatePort()
{
	m_MeshPort = CreateDefaultSubobject<UPortSkMeshComponent>(MeshPortComponentName);
	m_MeshPort->SetupAttachment(m_Mesh);
	m_MeshPort->InitPort(m_PhyConPort, m_Mesh);
}

void APortPawn::BeginPlay()
{
	Super::BeginPlay();

}
