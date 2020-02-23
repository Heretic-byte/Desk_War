// Fill out your copyright notice in the Description page of Project Settings.


#include "PortPawn.h"
#include "ConstructorHelpers.h"

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
	m_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshPawn00"));
	m_Mesh->SetupAttachment(RootComponent);
	m_Mesh->SetCollisionProfileName("PhysicsActor");
	m_Mesh->SetSimulatePhysics(true);
}

void APortPawn::CreatePhyCon()
{
	m_PhyConPort = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhyCon00"));
	m_PhyConPort->SetupAttachment(RootComponent);
	m_PhyConPort->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhyConPort->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhyConPort->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhyConPort->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhyConPort->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhyConPort->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhyConPort->SetDisableCollision(true);
}

void APortPawn::CreatePort()
{
	m_MeshPort = CreateDefaultSubobject<UPortSkMeshComponent>(TEXT("MeshPort00"));
	m_MeshPort->SetupAttachment(m_Mesh);
	m_MeshPort->InitPort(m_PhyConPort, m_Mesh);
}

void APortPawn::BeginPlay()
{
	Super::BeginPlay();

}
