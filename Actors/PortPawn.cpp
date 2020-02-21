// Fill out your copyright notice in the Description page of Project Settings.


#include "PortPawn.h"
#include "ConstructorHelpers.h"

// Sets default values
APortPawn::APortPawn(const FObjectInitializer& objInit):Super(objInit)
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root00"));

	m_MeshPawn = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshPawn00"));
	m_MeshPawn->SetupAttachment(RootComponent);

	m_MeshPort = CreateDefaultSubobject<UPortSkMeshComponent>(TEXT("MeshPort00"));
	m_MeshPort->SetupAttachment(m_MeshPawn);

	m_PhyConPort=CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhyCon00"));
	m_PhyConPort->SetupAttachment(RootComponent);
	m_PhyConPort->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhyConPort->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhyConPort->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhyConPort->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhyConPort->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhyConPort->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);

	m_PhyConPort->SetDisableCollision(true);
	m_MeshPort->InitPort(m_PhyConPort,m_MeshPawn);

	SetTickGroup(ETickingGroup::TG_StartPhysics);
}

void APortPawn::BeginPlay()
{
	Super::BeginPlay();

}
