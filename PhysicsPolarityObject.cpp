// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsPolarityObject.h"
#include "Engine.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"



APhysicsPolarityObject::APhysicsPolarityObject()
{
	InfluenceRange = 1000.0f;
	PhysicsPower = 200.0f;

	StaticMeshComponent = CreateOptionalDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	if (StaticMeshComponent) {
		StaticMeshComponent->SetSimulatePhysics(true);
		StaticMeshComponent->SetMassOverrideInKg(NAME_None, 5000.0f, true);
		StaticMeshComponent->SetLinearDamping(1.0f);
		StaticMeshComponent->SetAngularDamping(0.4f);
		StaticMeshComponent->SetEnableGravity(true);
		RootComponent = StaticMeshComponent;
		Root->DestroyComponent();
		InitPolarMesh(StaticMeshComponent);
	}

	SphereArea = CreateDefaultSubobject<USphereComponent>(TEXT("SphereArea"));
	if (SphereArea) {
		SphereArea->InitSphereRadius(InfluenceRange);
		SphereArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereArea->AttachTo(StaticMeshComponent);
		SphereArea->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
		InfluenceAreas.Emplace(SphereArea);
	}

	bInteractingWithInfluenceAreas = true;
}


void APhysicsPolarityObject::BeginPlay()
{
	Super::BeginPlay();
}


void APhysicsPolarityObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SphereArea->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
}


void APhysicsPolarityObject::ApplyAffectedForce_Implementation(float DeltaTime)
{
	PolarityMesh->AddForce(ConsumeAffectedForce() * PhysicsPower * Weight * DeltaTime, NAME_None, true);
}

#if WITH_EDITOR
void APhysicsPolarityObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const UProperty* PropertyThatChanged = PropertyChangedEvent.MemberProperty;
	if (PropertyThatChanged) {
		if (PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UStaticMeshComponent, RelativeScale3D))
		{
			SphereArea->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
		}
		else if (PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(APhysicsPolarityObject, InfluenceRange))
		{
			SphereArea->SetSphereRadius(InfluenceRange);
		}
	}
}
#endif // WITH_EDITOR