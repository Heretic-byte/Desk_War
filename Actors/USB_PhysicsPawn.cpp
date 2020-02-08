// Fill out your copyright notice in the Description page of Project Settings.


#include "USB_PhysicsPawn.h"
#include "Components/Connector.h"
#include "UObject/ConstructorHelpers.h"
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

	m_ArySpineColls.Empty();
	m_ArySplineMeshCompos.Empty();

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	CreateConnectorUSB();
	CreateConnector4Pin();
	CreateSpline();
}

void AUSB_PhysicsPawn::CreateConnectorUSB()
{
	m_USB_Pin = CreateDefaultSubobject<UConnector>(TEXT("Connector_USB_00"));
	m_USB_Pin->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundMeshPortUSB(TEXT("SkeletalMesh'/Game/01_FinalUSB/Mesh/Head/NewHead0120/USB_Head_Mesh_06.USB_Head_Mesh_06'"));
	check(FoundMeshPortUSB.Object);
	m_USB_Pin->GetPin()->SetSkeletalMesh(FoundMeshPortUSB.Object);
	m_USB_Pin->RelativeScale3D = FVector(3.000000, 3.000000, 3.000000);
}

void AUSB_PhysicsPawn::CreateConnector4Pin()
{
	m_4Pin_Pin = CreateDefaultSubobject<UConnector>(TEXT("Connector_4Pin_00"));
	m_4Pin_Pin->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundMeshPort4Pin(TEXT("SkeletalMesh'/Game/01_FinalUSB/Mesh/Tail/Tail_03.Tail_03'"));
	check(FoundMeshPort4Pin.Object);
	m_4Pin_Pin->GetPin()->SetSkeletalMesh(FoundMeshPort4Pin.Object);
	m_4Pin_Pin->RelativeLocation = FVector(-83.560440, 0.000000, 0.000000);
	m_4Pin_Pin->RelativeRotation = FRotator(0.f, 180.f, 0.f);
	m_4Pin_Pin->RelativeScale3D = FVector(3.000000, 3.000000, 3.000000);
}

void AUSB_PhysicsPawn::CreateSpline()
{
	m_SpineSpline = CreateDefaultSubobject<USplineComponent>(TEXT("Spine00"));
	//
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundMeshSpine(TEXT("StaticMesh'/Game/01_FinalUSB/Mesh/Body/USB_Parts_Body.USB_Parts_Body'"));
	check(FoundMeshSpine.Object);
	m_SpineMesh = FoundMeshSpine.Object;
}


void AUSB_PhysicsPawn::InitUSB()
{
	int SpineCount=SetTailLocation();
	SpawnSpineColls(SpineCount);
	InitSplineComponent();
	InitSplineMesh();
	InitPhysicsConstraints();
}

int AUSB_PhysicsPawn::SetTailLocation()
{
	m_4Pin_Pin->SetSimulatePhysics(false);
	FVector SocketUSB = m_USB_Pin->GetNeckLoc();
	FVector Socket4Pin = m_4Pin_Pin->GetNeckLoc();

	float Dividend = (FVector::Distance(SocketUSB, Socket4Pin) / GetActorScale3D().X) - m_fLineExtraSpacing;
	float Divisor = (m_fLineRadius * 2.f) + m_fLineExtraSpacing;

	int Count = Dividend / Divisor;
	float Offset = Dividend - (Count*Divisor);

	FVector NewLoc(Offset, 0, 0);
	FVector Pin4Loc = m_4Pin_Pin->RelativeLocation;
	m_4Pin_Pin->SetRelativeLocation(Pin4Loc + NewLoc);
	return Count;
}

void AUSB_PhysicsPawn::SpawnSpineColls(int nSpineCount)
{
	FVector ActorLoc = GetActorLocation();
	float Offset = FVector::Distance(m_USB_Pin->GetNeckLoc(), ActorLoc) / GetActorScale3D().X;
	Offset += m_fLineExtraSpacing+ m_fLineRadius;
	Offset *= -1;

	int SpineCount = nSpineCount;

	m_ArySpineColls.Empty();

	while (SpineCount--)
	{
		FVector SphereLocation(Offset, 0, 0);
		FTransform SpineSphereTrans(SphereLocation);

		auto* SphereSpawned = AddSceneComponent<USphereComponent>(USphereComponent::StaticClass(), RootComponent, FTransform());

		SphereSpawned->SetCollisionProfileName(FName(TEXT("USBMesh")));
		SphereSpawned->SetSphereRadius(m_fLineRadius);
		SphereSpawned->SetMassOverrideInKg(NAME_None, m_fCollMass);
		SphereSpawned->SetAngularDamping(m_fSpineAngularDamping);
		SphereSpawned->SetLinearDamping(m_fSpineLinearDamping);
		SphereSpawned->SetPhysMaterialOverride(m_SpineFriction);
		SphereSpawned->SetGenerateOverlapEvents(false);
		m_ArySpineColls.Emplace(SphereSpawned);
		SphereSpawned->SetSimulatePhysics(true);
		SphereSpawned->SetRelativeLocation(SphereLocation);
		Offset -= (m_fLineRadius * 2) + m_fLineExtraSpacing;
	}
}

void AUSB_PhysicsPawn::InitSplineComponent()
{
	m_SpineSpline->SetAbsolute(true, true, true);
	m_SpineSpline->ClearSplinePoints();
	UpdateSplinePoint();
}

void AUSB_PhysicsPawn::UpdateSplinePoint()
{
	m_SpineSpline->ClearSplinePoints(false);
	m_SpineSpline->AddSplinePoint(m_USB_Pin->GetNeckLoc(), ESplineCoordinateSpace::World, false);

	for (auto* SpineColl : m_ArySpineColls)
	{
		FVector CollLoc = SpineColl->GetComponentLocation();
		m_SpineSpline->AddSplinePoint(CollLoc, ESplineCoordinateSpace::World, false);

	}
	m_SpineSpline->AddSplinePoint(m_4Pin_Pin->GetNeckLoc(), ESplineCoordinateSpace::World, true);
}

void AUSB_PhysicsPawn::InitSplineMesh()
{
	int MaxColl = m_SpineSpline->GetNumberOfSplinePoints() - 1;

	FTransform Trans;
	float Scale = GetActorScale3D().X * 4.f;
	FVector2D SplineScale(Scale, Scale);
	m_ArySplineMeshCompos.Reset();
	while (MaxColl--)
	{
		USplineMeshComponent* SplineMesh = AddSceneComponent<USplineMeshComponent>(USplineMeshComponent::StaticClass(), RootComponent, Trans);

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
	FVector PointLoc1;
	FVector PointTangent1;
	FVector PointLoc2;
	FVector PointTangent2;
	FVector UpVector;

	m_SpineSpline->GetLocationAndTangentAtSplinePoint(0, PointLoc1, PointTangent1, ESplineCoordinateSpace::World);

	UpVector = m_SpineSpline->GetUpVectorAtSplinePoint(0, ESplineCoordinateSpace::World);

	for (int i = 0; i < m_ArySplineMeshCompos.Num(); i++)
	{
		m_SpineSpline->GetLocationAndTangentAtSplinePoint(i + 1, PointLoc2, PointTangent2, ESplineCoordinateSpace::World);


		m_ArySplineMeshCompos[i]->SetStartAndEnd(PointLoc1, PointTangent1, PointLoc2, PointTangent2, false);

		PointLoc1 = PointLoc2;
		PointTangent1 = PointTangent2;

		m_ArySplineMeshCompos[i]->SetSplineUpDir(UpVector);
	}
}




void AUSB_PhysicsPawn::InitPhysicsConstraints()
{
	m_USB_Pin->SetSimulatePhysics(true);
	m_4Pin_Pin->SetSimulatePhysics(true);
	m_ArySpineColls[0]->SetSimulatePhysics(true);
	m_ArySpineColls[m_ArySpineColls.Num() - 1]->SetSimulatePhysics(true);

	UPhysicsConstraintComponent* UsbPhyCon = AddPhysicsConstraint(m_USB_Pin->GetComponentTransform());
	UsbPhyCon->SetWorldLocation(m_USB_Pin->GetNeckLoc());//GetUsbSocLoc
	UsbPhyCon->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	UsbPhyCon->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	UsbPhyCon->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.f);
	UsbPhyCon->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	UsbPhyCon->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	UsbPhyCon->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);


	UsbPhyCon->SetConstrainedComponents(m_USB_Pin->GetPin(), m_USB_Pin->GetBoneNeck(), m_ArySpineColls[0], NAME_None);//Mesh

	FVector CollLocTemp = m_ArySpineColls[0]->GetComponentLocation();

	for (int i = 1; i < m_ArySpineColls.Num(); i++)
	{
		m_ArySpineColls[i]->SetSimulatePhysics(true);

		UPhysicsConstraintComponent* SpinePhyCon = AddPhysicsConstraint(m_ArySpineColls[i - 1]->GetComponentTransform());

		SpinePhyCon->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 35.f);//35
		SpinePhyCon->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 20.f);//20
		SpinePhyCon->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, 20.f);//20
		SpinePhyCon->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
		SpinePhyCon->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
		SpinePhyCon->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);


		SpinePhyCon->SetWorldLocation((CollLocTemp + m_ArySpineColls[i]->GetComponentLocation()) / 2.f);
		CollLocTemp = m_ArySpineColls[i]->GetComponentLocation();

		SpinePhyCon->SetConstrainedComponents(m_ArySpineColls[i], NAME_None, m_ArySpineColls[i - 1], NAME_None);
	}

	UPhysicsConstraintComponent* Pin4PhyCon = AddPhysicsConstraint(m_ArySpineColls[m_ArySpineColls.Num() - 1]->GetComponentTransform());
	Pin4PhyCon->SetWorldLocation(m_4Pin_Pin->GetNeckLoc());//Get4PinLoc
	Pin4PhyCon->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	Pin4PhyCon->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	Pin4PhyCon->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.f);
	Pin4PhyCon->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	Pin4PhyCon->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	Pin4PhyCon->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	Pin4PhyCon->SetConstrainedComponents(m_4Pin_Pin->GetPin(), m_4Pin_Pin->GetBoneNeck(), m_ArySpineColls[m_ArySpineColls.Num() - 1], NAME_None);
}



// Called every frame
void AUSB_PhysicsPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateSplinePoint();
	UpdateSplineMesh();
}



USkeletalMeshComponent * AUSB_PhysicsPawn::GetHead() const
{
	return m_ConnectorHead->GetPin();;
}
USkeletalMeshComponent * AUSB_PhysicsPawn::GetTail() const
{
	return m_ConnectorTail->GetPin();
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
	SetUpActorComponent(compo);
	compo->SetRelativeTransform(trans);
	compo->AttachToComponent(parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}


