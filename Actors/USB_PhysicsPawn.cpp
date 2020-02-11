// Fill out your copyright notice in the Description page of Project Settings.


#include "USB_PhysicsPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
// Sets default values
AUSB_PhysicsPawn::AUSB_PhysicsPawn(const FObjectInitializer& objInit):Super(objInit)
{
	PrimaryActorTick.bCanEverTick = true;
	SetTickGroup(ETickingGroup::TG_PostPhysics);

	m_fLineRadius = 5.3f;
	m_fLineExtraSpacing = 2.f;
	m_fCollMass = 0.5;
	m_fSpineLinearDamping = 1.f;
	m_fSpineAngularDamping = 1.f;
	m_fMaxAngularVelocity = 500.f;
	m_ArySpineColls.Empty();
	m_ArySplineMeshCompos.Empty();

	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> FoundPhyMat(TEXT("PhysicalMaterial'/Game/01_FinalUSB/PhysicsMaterial/PM_LowFriction1.PM_LowFriction1'"));
	if (FoundPhyMat.Succeeded())
	{
		m_SpineFriction = FoundPhyMat.Object;
	}

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	CreatePinUSB();
	CreatePin4Pin();
	CreateSpline();

	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> FoundUSBPhyMat(TEXT("/Game/01_FinalUSB/PhysicsMaterial/PM_USB_Main.PM_USB_Main"));
	if (FoundUSBPhyMat.Succeeded())
	{
		m_PinFriction = FoundUSBPhyMat.Object;
	}
}

void AUSB_PhysicsPawn::CreatePinUSB()
{
	m_PinUSB = CreateDefaultSubobject<UPinSkMeshComponent>(TEXT("Pin_USB_00"));
	m_PinUSB->SetupAttachment(RootComponent);
	m_PinUSB->SetPinType(E_PinPortType::EUSB);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundMeshPortUSB(TEXT("SkeletalMesh'/Game/01_FinalUSB/Mesh/Head/NewHead0120/USB_Head_Mesh_06.USB_Head_Mesh_06'"));
	check(FoundMeshPortUSB.Object);
	m_PinUSB->SetSkeletalMesh(FoundMeshPortUSB.Object);
	m_PinUSB->SetCollisionProfileName(TEXT("USBMesh"));
	m_PinUSB->RelativeScale3D = FVector(3.000000, 3.000000, 3.000000);
}

void AUSB_PhysicsPawn::CreatePin4Pin()
{
	m_Pin5Pin = CreateDefaultSubobject<UPinSkMeshComponent>(TEXT("m_Pin_5Pin_00"));
	m_Pin5Pin->SetupAttachment(RootComponent);
	m_Pin5Pin->SetPinType(E_PinPortType::E5Pin);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundMeshPort4Pin(TEXT("SkeletalMesh'/Game/01_FinalUSB/Mesh/Tail/Tail_03.Tail_03'"));
	check(FoundMeshPort4Pin.Object);
	m_Pin5Pin->SetSkeletalMesh(FoundMeshPort4Pin.Object);
	m_Pin5Pin->SetCollisionProfileName(TEXT("USBMesh"));
	m_Pin5Pin->SetNeckName(TEXT("joint12"));
	m_Pin5Pin->SetVelocityPivotName(TEXT("joint11"));
	m_Pin5Pin->RelativeLocation = FVector(-83.560440, 0.000000, 0.000000);
	m_Pin5Pin->RelativeRotation = FRotator(0.f, 180.f, 0.f);
	m_Pin5Pin->RelativeScale3D = FVector(3.000000, 3.000000, 3.000000);
}

void AUSB_PhysicsPawn::CreateSpline()
{
	m_SpineSpline = CreateDefaultSubobject<USplineComponent>(TEXT("Spine00"));
	m_SpineSpline->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundMeshSpine(TEXT("StaticMesh'/Game/01_FinalUSB/Mesh/Body/USB_Parts_Body.USB_Parts_Body'"));
	check(FoundMeshSpine.Object);
	m_SpineMesh = FoundMeshSpine.Object;
}

void AUSB_PhysicsPawn::InitUSB()
{
	m_nSphereSpineCount =SetTailLocation();
	if (m_nSphereSpineCount <= 0)
	{
		PRINTF("Spine Count 0");
		return;
	}
	SpawnSpineColls();
	InitSplineComponent();
	InitSplineMesh();
	InitPhysicsConstraints();
}

int AUSB_PhysicsPawn::SetTailLocation()
{
	m_Pin5Pin->SetSimulatePhysics(false);
	FVector SocketUSB = m_PinUSB->GetNeckLoc();
	FVector Socket4Pin = m_Pin5Pin->GetNeckLoc();

	float Dividend = (FVector::Distance(SocketUSB, Socket4Pin) / GetActorScale3D().X) - m_fLineExtraSpacing;
	float Divisor = (m_fLineRadius * 2.f) + m_fLineExtraSpacing;

	int Count = Dividend / Divisor;
	float Offset = Dividend - (Count*Divisor);

	FVector NewLoc(Offset, 0, 0);
	FVector Pin5Loc = m_Pin5Pin->RelativeLocation;
	m_Pin5Pin->SetRelativeLocation(Pin5Loc + NewLoc, false, nullptr, ETeleportType::ResetPhysics);
	return Count;
}

void AUSB_PhysicsPawn::SpawnSpineColls()
{
	FVector ActorLoc = GetActorLocation();
	float Offset = FVector::Distance(m_PinUSB->GetNeckLoc(), ActorLoc) / GetActorScale3D().X;
	Offset += m_fLineExtraSpacing+ m_fLineRadius;
	Offset *= -1;

	int SpineCount = m_nSphereSpineCount;

	m_ArySpineColls.Empty();

	while (SpineCount--)
	{
		FVector SphereLocation(Offset, 0, 0);

		auto* SphereSpawned = AddSceneComponent<USphereComponent>(USphereComponent::StaticClass(), RootComponent, FTransform());
		SphereSpawned->SetCollisionProfileName(FName(TEXT("USBMesh")));
		SphereSpawned->SetSphereRadius(m_fLineRadius);
		SphereSpawned->SetMassOverrideInKg(NAME_None, m_fCollMass);
		SphereSpawned->SetAngularDamping(m_fSpineAngularDamping);
		SphereSpawned->SetLinearDamping(m_fSpineLinearDamping);
		SphereSpawned->SetPhysMaterialOverride(m_SpineFriction);
		SphereSpawned->SetGenerateOverlapEvents(false);
		SphereSpawned->SetRelativeLocation(SphereLocation,false,nullptr,ETeleportType::ResetPhysics);
		SphereSpawned->SetPhysicsMaxAngularVelocityInDegrees(m_fMaxAngularVelocity);
		SphereSpawned->SetSimulatePhysics(true);
		m_ArySpineColls.Emplace(SphereSpawned);
		Offset -= (m_fLineRadius * 2) + m_fLineExtraSpacing;
	}
}

void AUSB_PhysicsPawn::InitSplineComponent()
{

	m_SpineSpline->ClearSplinePoints();
	m_SpineSpline->AddSplinePoint(m_PinUSB->GetNeckLoc(), ESplineCoordinateSpace::Type::World, false);
	
	for (auto* SpineColl : m_ArySpineColls)
	{
		FVector CollLoc = SpineColl->GetComponentLocation();
		m_SpineSpline->AddSplinePoint(CollLoc, ESplineCoordinateSpace::Type::World, false);

	}
	m_SpineSpline->AddSplinePoint(m_Pin5Pin->GetNeckLoc(), ESplineCoordinateSpace::Type::World, true);
}

void AUSB_PhysicsPawn::UpdateSplinePoint()
{
	m_SpineSpline->SetWorldLocationAtSplinePoint(0, m_PinUSB->GetNeckLoc());

	int SphereIter = m_SpineSpline->GetNumberOfSplinePoints() - 1;

	for (int i = 1; i < SphereIter; i++)
	{
		auto * SphereCurrent = m_ArySpineColls[i - 1];
		m_SpineSpline->SetWorldLocationAtSplinePoint(i, SphereCurrent->GetComponentLocation());
	}

	m_SpineSpline->SetWorldLocationAtSplinePoint(SphereIter, m_Pin5Pin->GetNeckLoc());
}

void AUSB_PhysicsPawn::InitSplineMesh()
{
	int MaxSplinePoints = m_SpineSpline->GetNumberOfSplinePoints() - 1;

	float Scale = GetActorScale3D().X * 4.f;
	FVector2D SplineScale(Scale, Scale);
	m_ArySplineMeshCompos.Reset(MaxSplinePoints);
	while (MaxSplinePoints--)
	{
		USplineMeshComponent* SplineMesh =
			AddSceneComponent<USplineMeshComponent>(USplineMeshComponent::StaticClass(), RootComponent, RootComponent->GetComponentTransform());
		SplineMesh->SetCollisionProfileName(FName(TEXT("NoCollision")));
		SplineMesh->SetStaticMesh(m_SpineMesh);
		SplineMesh->SetStartScale(SplineScale);
		SplineMesh->SetEndScale(SplineScale);
		m_ArySplineMeshCompos.Emplace(SplineMesh);
	}

	UpdateSplineMesh();
}

void AUSB_PhysicsPawn::UpdateSplineMesh()
{
	FVector CurrentLoc;
	FVector CurrentTangent;
	FVector NextLoc;
	FVector NextTangent;
	FVector CurrentUpVector;

	FVector Soc = m_PinUSB->GetNeckLoc();

	m_SpineSpline->GetLocationAndTangentAtSplinePoint(0, CurrentLoc, CurrentTangent, ESplineCoordinateSpace::Type::Local);

	for (int i = 0; i < m_ArySplineMeshCompos.Num(); i++)
	{
		m_SpineSpline->GetLocationAndTangentAtSplinePoint(i + 1, NextLoc, NextTangent, ESplineCoordinateSpace::Type::Local);

		m_ArySplineMeshCompos[i]->SetStartAndEnd(CurrentLoc, CurrentTangent, NextLoc, NextTangent, false);

		FVector GuessWorld = UKismetMathLibrary::InverseTransformLocation(RootComponent->GetComponentTransform(), CurrentLoc);
		FVector GuessLcal = UKismetMathLibrary::InverseTransformLocation(m_ArySplineMeshCompos[i]->GetComponentTransform(), CurrentLoc);
		FVector GuessLocalSpline = UKismetMathLibrary::InverseTransformLocation(m_SpineSpline->GetComponentTransform(), CurrentLoc);
		USplineMeshComponent* CurrentMesh= m_ArySplineMeshCompos[i];
		CurrentLoc = NextLoc;
		CurrentTangent = NextTangent;

		CurrentUpVector = m_SpineSpline->GetUpVectorAtSplinePoint(i, ESplineCoordinateSpace::Type::Local);
		m_ArySplineMeshCompos[i]->SetSplineUpDir(CurrentUpVector,true);
	}
}




void AUSB_PhysicsPawn::InitPhysicsConstraints()
{
	m_PinUSB->SetPhysMaterialOverride(m_PinFriction);
	m_Pin5Pin->SetPhysMaterialOverride(m_PinFriction);

	m_PinUSB->SetSimulatePhysics(true);
	m_Pin5Pin->SetSimulatePhysics(true);
	m_ArySpineColls[0]->SetSimulatePhysics(true);
	m_ArySpineColls[m_ArySpineColls.Num() - 1]->SetSimulatePhysics(true);

	m_PinUSB->SetPhysicsMaxAngularVelocityInDegrees(m_fMaxAngularVelocity);
	m_Pin5Pin->SetPhysicsMaxAngularVelocityInDegrees(m_fMaxAngularVelocity);

	UPhysicsConstraintComponent* UsbPhyCon = AddPhysicsConstraint(FTransform());
	UsbPhyCon->SetWorldLocation(m_PinUSB->GetNeckLoc(), false, nullptr, ETeleportType::ResetPhysics);
	UsbPhyCon->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	UsbPhyCon->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	UsbPhyCon->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.f);
	UsbPhyCon->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	UsbPhyCon->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	UsbPhyCon->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);

	UsbPhyCon->SetConstrainedComponents(m_PinUSB, m_PinUSB->GetBoneNeck(), m_ArySpineColls[0], NAME_None);//Mesh

	FVector CollLocTemp = m_ArySpineColls[0]->GetComponentLocation();

	for (int i = 1; i < m_ArySpineColls.Num(); i++)
	{
		m_ArySpineColls[i]->SetSimulatePhysics(true);

		UPhysicsConstraintComponent* SpinePhyCon = AddPhysicsConstraint(FTransform());

		SpinePhyCon->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 35.f);//35
		SpinePhyCon->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 20.f);//20
		SpinePhyCon->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, 20.f);//20
		SpinePhyCon->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
		SpinePhyCon->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
		SpinePhyCon->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);


		SpinePhyCon->SetWorldLocation((CollLocTemp + m_ArySpineColls[i]->GetComponentLocation()) / 2.f, false, nullptr, ETeleportType::ResetPhysics);
		CollLocTemp = m_ArySpineColls[i]->GetComponentLocation();

		SpinePhyCon->SetConstrainedComponents(m_ArySpineColls[i], NAME_None, m_ArySpineColls[i - 1], NAME_None);
	}

	UPhysicsConstraintComponent* Pin4PhyCon = AddPhysicsConstraint(FTransform());
	Pin4PhyCon->SetWorldLocation(m_Pin5Pin->GetNeckLoc(), false, nullptr, ETeleportType::ResetPhysics);
	Pin4PhyCon->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	Pin4PhyCon->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	Pin4PhyCon->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.f);
	Pin4PhyCon->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	Pin4PhyCon->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	Pin4PhyCon->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	Pin4PhyCon->SetConstrainedComponents(m_Pin5Pin, m_Pin5Pin->GetBoneNeck(), m_ArySpineColls[m_ArySpineColls.Num() - 1], NAME_None);
}

void AUSB_PhysicsPawn::BeginPlay()
{
	Super::BeginPlay();
	InitUSB();
}

void AUSB_PhysicsPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateSplinePoint();
	UpdateSplineMesh();
}


void AUSB_PhysicsPawn::SetUpActorComponent(UActorComponent * compo)
{
	compo->RegisterComponent();
	AddInstanceComponent(compo);
}

UPhysicsConstraintComponent * AUSB_PhysicsPawn::AddPhysicsConstraint(const FTransform trans)
{
	auto* Compo = AddSceneComponent<UPhysicsConstraintComponent>(UPhysicsConstraintComponent::StaticClass(), RootComponent, trans);
	return Compo;
}

void AUSB_PhysicsPawn::SetUpSceneComponent(USceneComponent * compo, USceneComponent * parent, FTransform trans)
{
	compo->SetupAttachment(parent);
	SetUpActorComponent(compo);
}


