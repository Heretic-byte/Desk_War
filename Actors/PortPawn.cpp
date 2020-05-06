// Fill out your copyright notice in the Description page of Project Settings.


#include "PortPawn.h"
#include "ConstructorHelpers.h"
#include "AIController.h"


FName APortPawn::MeshComponentName(TEXT("MeshPawn00"));
FName APortPawn::MeshPortComponentName(TEXT("MeshPort00"));

APortPawn::APortPawn(const FObjectInitializer& objInit):Super(objInit)
{
	//RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root00"));
	CreateMesh();
	CreatePhyCon();
	CreatePort();
	CreateSphereColl();

	AIControllerClass = nullptr;
}

void APortPawn::CreateMesh()
{
	m_Mesh = CreateDefaultSubobject<UPhysicsSkMeshComponent>(MeshComponentName);
	//m_Mesh->SetupAttachment(RootComponent);
	RootComponent = m_Mesh;
	m_Mesh->SetCollisionProfileName("PhysicsActor");
	m_Mesh->SetSimulatePhysics(true);
	m_Mesh->SetUseCCD(true);

	
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

void APortPawn::CreateSphereColl()
{
	m_Sphere = CreateDefaultSubobject<USphereComponent>("Sphere00");
	m_Sphere->SetupAttachment(m_Mesh);
	m_Sphere->SetSphereRadius(300.f);
	m_Sphere->SetCollisionProfileName("USBOverlap");
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
	m_MeshPort->CastShadow = false;
	m_MeshPort->SetGenerateOverlapEvents(true);
}

void APortPawn::BeginPlay()
{
	Super::BeginPlay();
	m_MeshPort->InitPort(m_PhyConPort, m_Mesh,m_Sphere);
	m_MeshPort->m_OnConnected.AddUObject(this,&APortPawn::PortConnected);
	m_MeshPort->m_OnDisconnected.AddUObject(this, &APortPawn::PortDisConnected);


}

void APortPawn::OnInit(AObjectGiver * objGiver)
{
	m_Spawner = objGiver;
}

void APortPawn::OnPullEnque()
{
}

