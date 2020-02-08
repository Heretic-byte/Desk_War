// Fill out your copyright notice in the Description page of Project Settings.


#include "USB_PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/MeshComponent.h"
#include "Components/InteractableComponent.h"
#include "Managers/USB_GameManager.h"
//
#include "Components/Port.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Components/Port.h"
#include "Components/Connector.h"

#include "Actors/USB_PlayerController.h"


#include "UObject/ConstructorHelpers.h"
// Sets default values
AUSB_PlayerCharacter::AUSB_PlayerCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer
		//.DoNotCreateDefaultSubobject(ACharacter::MeshComponentName)
		//.DoNotCreateDefaultSubobject(TEXT("Arrow"))
		//.DoNotCreateDefaultSubobject(ACharacter::CapsuleComponentName)
	)
{
	PrimaryActorTick.bCanEverTick = true;
	

	m_GameManager = nullptr;
	m_InteractObj = nullptr;
	m_PlayerController = nullptr;
	m_CollUSB = nullptr;
	m_MeshUSB = nullptr;
	m_Mesh4Pin = nullptr;
	m_Coll4Pin = nullptr;
	m_SpineFriction = nullptr;
	//
	m_bIsComponentLocal = true;
	m_NameUSBSocket = FName(TEXT("joint9"));
	m_Name4PinSocket = FName(TEXT("joint12"));
	m_NameConnectSocket = FName(TEXT("ConnectPoint"));
	m_NameConnectStartSocket = FName(TEXT("ConnectStart"));
	m_NameConnectPushPointSocket = FName(TEXT("PushPoint"));
	//
	m_ArySpineColls.Empty();
	m_ArySpineColls.Reserve(10);
	m_ArySplineMeshCompos.Empty();
	m_ArySplineMeshCompos.Reserve(10);
	m_SplineSpace = ESplineCoordinateSpace::Local;


	m_fYawSpeed = 0.3f;
	m_fPitchSpeed = 0.3f;
	m_bCanCamRotate = true;

	m_fHeadChangeCD = 0.5f;
	m_fConnectHorizontalAngle = 0.06f;

	m_fConnectFailImpulseOther = 2000000.f;
	m_fConnectFailImpulseSelf = 40000.f;

	m_TraceTypes.Empty();
	m_TraceTypes.Emplace(EObjectTypeQuery::ObjectTypeQuery1);// world static
	m_TraceTypes.Emplace(EObjectTypeQuery::ObjectTypeQuery2);// world dynamic
	m_TraceTypes.Emplace(EObjectTypeQuery::ObjectTypeQuery3);// pawn

	CreateComponent();
}

void AUSB_PlayerCharacter::CreateComponent()
{
	bUseControllerRotationYaw = false;

	RootComponent= CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	m_CollUSB = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Coll01"));
	m_CollUSB->SetupAttachment(RootComponent);
	m_CollUSB->SetCapsuleRadius(12.87f);
	m_CollUSB->SetCapsuleHalfHeight(12.87f);
	m_CollUSB->SetCollisionProfileName(FName(TEXT("USBMovement")));
	CapsuleComponent=m_CollUSB;

	CharacterMovement = CreateDefaultSubobject<UCharacterMovementComponent>(ACharacter::CharacterMovementComponentName);
	if (CharacterMovement)
	{
		CharacterMovement->UpdatedComponent = CapsuleComponent;
		CrouchedEyeHeight = CharacterMovement->CrouchedHalfHeight * 0.80f;
	}

	GetCharacterMovement()->SetUpdatedComponent(m_CollUSB);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	//
	
	//
	m_CamRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CamRoot"));
	m_CamRoot->SetupAttachment(m_CollUSB);

	m_MainSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring00"));
	m_MainSpringArm->SetupAttachment(m_CamRoot);
	m_MainSpringArm->TargetArmLength = 600.f;
	m_MainSpringArm->SocketOffset = FVector(50.f, 0, 0);
	m_MainSpringArm->TargetOffset = FVector(0.f, 0.f, 400.f);
	m_MainSpringArm->bUsePawnControlRotation = true;
	m_MainSpringArm->bInheritRoll = false;
	m_MainSpringArm->bEnableCameraLag = true;
	m_MainSpringArm->bEnableCameraRotationLag = true;
	m_MainSpringArm->CameraLagSpeed = 25.f;
	m_MainSpringArm->CameraRotationLagSpeed = 20.f;
	m_MainSpringArm->bAutoActivate = true;
	//

	m_MainCam = CreateDefaultSubobject<UCameraComponent>(TEXT("Cam00"));
	m_MainCam->SetupAttachment(m_MainSpringArm);
	m_MainCam->bAutoActivate = true;
	m_MainCam->FieldOfView = 100.f;
	m_MainCam->RelativeRotation = FRotator(-30.f, 0.f, 0.f);
	//

	m_MeshUSB = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh01"));
	m_MeshUSB->SetupAttachment(m_CollUSB);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundMeshPortUSB(TEXT("SkeletalMesh'/Game/01_FinalUSB/Mesh/Head/NewHead0120/USB_Head_Mesh_06.USB_Head_Mesh_06'"));
	check(FoundMeshPortUSB.Object);
	m_MeshUSB->SkeletalMesh = FoundMeshPortUSB.Object;

	m_MeshUSB->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	m_MeshUSB->OverrideMaterials.Reserve(2);
	m_MeshUSB->OverrideMaterials.Add(nullptr);
	m_MeshUSB->SetGenerateOverlapEvents(true);
	//
	m_MeshUSB->SetCollisionProfileName(FName(TEXT("USBMesh")));
	m_MeshUSB->BodyInstance.bUseCCD = false;
	m_MeshUSB->BodyInstance.bNotifyRigidBodyCollision = true;
	m_MeshUSB->BodyInstance.bLockXRotation = true;
	m_MeshUSB->BodyInstance.LinearDamping = 6.000000f;
	m_MeshUSB->BodyInstance.AngularDamping = 8.000000f;
	m_MeshUSB->RelativeRotation = FRotator(0.000027, 0.000000, 0.000000);
	m_MeshUSB->RelativeScale3D = FVector(3.000000, 3.000000, 3.000000);
	//
	m_Coll4Pin = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Coll4Pin01"));
	m_Coll4Pin->SetupAttachment(RootComponent);
	m_Coll4Pin->RelativeLocation = FVector(-83.560440, 0.000000, 0.000000);
	m_Coll4Pin->RelativeRotation = FRotator(0.f,180.f,0.f);
	m_Coll4Pin->SetCapsuleRadius(12.87f);
	m_Coll4Pin->SetCapsuleHalfHeight(12.87f);
	m_Coll4Pin->SetCollisionProfileName(FName(TEXT("USBMovement")));
	//
	m_Mesh4Pin = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh4Pin01"));
	m_Mesh4Pin->SetupAttachment(m_Coll4Pin);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundMeshPort4Pin(TEXT("SkeletalMesh'/Game/01_FinalUSB/Mesh/Tail/Tail_03.Tail_03'"));
	check(FoundMeshPort4Pin.Object);
	m_Mesh4Pin->SkeletalMesh = FoundMeshPort4Pin.Object;

	m_Mesh4Pin->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	m_Mesh4Pin->SetCollisionProfileName(FName(TEXT("USBMesh")));
	m_Mesh4Pin->BodyInstance.bUseCCD = false;
	m_Mesh4Pin->BodyInstance.bNotifyRigidBodyCollision = false;
	m_Mesh4Pin->BodyInstance.LinearDamping = 6.000000f;
	m_Mesh4Pin->BodyInstance.AngularDamping = 8.000000f;
	m_Mesh4Pin->RelativeScale3D = FVector(3.000000, 3.000000, 3.000000);
	m_Mesh4Pin->SetSimulatePhysics(false);
	m_Mesh4Pin->BodyInstance.bLockXRotation = true;
	//
	//misc
	m_SpineSpline = CreateDefaultSubobject<USplineComponent>(TEXT("Spine00"));
	//
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundMeshSpine(TEXT("StaticMesh'/Game/01_FinalUSB/Mesh/Body/USB_Parts_Body.USB_Parts_Body'"));
	check(FoundMeshSpine.Object);
	m_SpineMesh = FoundMeshSpine.Object;
	//
	m_ActionManager = CreateDefaultSubobject<UActionManagerComponent>(TEXT("Action00"));
	//m_ConnectorHead = CreateDefaultSubobject<UConnector>(TEXT("ConnectorUSB"));
	//m_ConnectorHead->SetColl(m_CollUSB);
	//m_ConnectorHead->SetMesh(m_MeshUSB);
	//m_ConnectorTail = CreateDefaultSubobject<UConnector>(TEXT("ConnectorPin4"));
	//m_ConnectorTail->SetColl(m_Coll4Pin);
	//m_ConnectorTail->SetMesh(m_Mesh4Pin);
}

void AUSB_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	m_GameManager = GetGameInstance<UUSB_GameManager>();
	m_PlayerController = Cast<APlayerController>(GetController());
	
	//InitUSB();
}

void AUSB_PlayerCharacter::InitUSB()
{
	SetTailLocation();
	SpawnSpineColls();
	InitSplineComponent();
	InitSplineMesh();
	InitPhysicsConstraints();
	InitMovement();
	SetHeadTailDefault();
}

void AUSB_PlayerCharacter::SetHeadTail(UConnector* head, UConnector* tail)
{
	m_ConnectorHead = head;
	m_ConnectorTail = tail;
	//
	//m_ConnectorHead->GetMesh()->SetSimulatePhysics(false);
	//m_ConnectorHead->GetColl()->SetWorldLocation(m_ConnectorHead->GetMesh()->GetComponentLocation(),false,nullptr,ETeleportType::ResetPhysics);
	//FRotator Rot(0,0,0);
	//Rot .Yaw= m_ConnectorHead->GetMesh()->GetComponentRotation().Yaw;

	//m_ConnectorHead->GetColl()->SetWorldRotation(Rot);
	//m_ConnectorHead->GetMesh()->AttachToComponent(m_ConnectorHead->GetColl(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
	////
	//m_ConnectorHead->GetColl()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//m_ConnectorTail->GetColl()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//m_ConnectorTail->GetMesh()->SetSimulatePhysics(true);
	////
	//m_MainSpringArm->AttachToComponent(m_ConnectorHead->GetColl(), FAttachmentTransformRules::KeepRelativeTransform);
	//CapsuleComponent = m_ConnectorHead->GetColl();
	//GetCharacterMovement()->SetUpdatedComponent(m_ConnectorHead->GetColl());
}

void AUSB_PlayerCharacter::SetTailLocation()
{
	FVector SocketUSB = GetUsbNeckSocLoc();
	FVector Socket4Pin = Get4PinNeckSocLoc();
	m_Mesh4Pin->SetSimulatePhysics(false);

	m_fWorldScaleX = GetActorScale3D().X;

	float Dividend = (FVector::Distance(SocketUSB, Socket4Pin) / m_fWorldScaleX) - m_fLineExtraSpacing;
	float Divisor = (m_fLineRadius * 2.f) + m_fLineExtraSpacing;


	int Count = Dividend / Divisor;
	float Offset = Dividend - (Count*Divisor);

	m_nSpineCount = Count;
	FVector NewLoc(Offset, 0, 0);
	FVector Pin4Loc = m_Coll4Pin->RelativeLocation;
	m_Coll4Pin->SetRelativeLocation(Pin4Loc+NewLoc,false,nullptr,ETeleportType::TeleportPhysics);
	m_Mesh4Pin->SetSimulatePhysics(true);
}
//
void AUSB_PlayerCharacter::SpawnSpineColls()
{
	//물리버그때문에 블프로 씀
	FVector ActorLoc = GetActorLocation();
	float Offset = FVector::Distance(GetUsbNeckSocLoc(), ActorLoc) / m_fWorldScaleX;
	Offset *= -1;
	Offset -= m_fLineExtraSpacing;
	Offset -= m_fLineRadius;

	int SpineCount = m_nSpineCount;

	m_ArySpineColls.Empty();

	while (SpineCount--)
	{
		FVector SphereLocation(Offset,0,0);
		FTransform SpineSphereTrans(SphereLocation);

		auto* SphereSpawned =AddSceneComponent<USphereComponent>(USphereComponent::StaticClass(), RootComponent, SpineSphereTrans);


		SphereSpawned->SetSimulatePhysics(true);

		SphereSpawned->SetCollisionProfileName(FName(TEXT("USBMesh")));
		SphereSpawned->SetSphereRadius(m_fLineRadius);
		SphereSpawned->SetMassOverrideInKg(NAME_None, m_fCollMass);
		SphereSpawned->SetAngularDamping(m_fAngularDamping);//m_fAngularDamping
		SphereSpawned->SetLinearDamping(m_fLinearDamping);//m_fLinearDamping
		SphereSpawned->SetPhysMaterialOverride(m_SpineFriction);
		SphereSpawned->bReplicatePhysicsToAutonomousProxy = false;
		SphereSpawned->SetGenerateOverlapEvents(false);
		m_ArySpineColls.Emplace(SphereSpawned);

		Offset -= (m_fLineRadius * 2) + m_fLineExtraSpacing;
	}
	

}

void AUSB_PlayerCharacter::SetUpSceneComponent(USceneComponent * compo, USceneComponent* parent, FTransform trans)
{
	SetUpActorComponent(compo);
	compo->AttachToComponent(parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	compo->SetRelativeTransform(trans);
}

void AUSB_PlayerCharacter::SetUpActorComponent(UActorComponent * compo)
{
	compo->RegisterComponent();
	AddInstanceComponent(compo);
}

void AUSB_PlayerCharacter::CantUp()
{
	bCantUp = true;
}

void AUSB_PlayerCharacter::CantDown()
{
	bCantDown = true;
}

void AUSB_PlayerCharacter::CantRight()
{
	bCantRight = true;
}

void AUSB_PlayerCharacter::CantLeft()
{
	bCantLeft = true;
}

void AUSB_PlayerCharacter::Clear()
{

	bCantRight = false;
	bCantLeft = false;
	bCantDown = false;
	bCantUp = false;
}

void AUSB_PlayerCharacter::InitSplineComponent()
{
	m_SpineSpline->SetAbsolute(false, false, m_bIsComponentLocal);
	m_SpineSpline->ClearSplinePoints();
	UpdateSplinePoint();
}

void AUSB_PlayerCharacter::UpdateSplinePoint()
{
	m_SpineSpline->ClearSplinePoints(false);
	m_SpineSpline->AddSplinePoint(GetUsbNeckSocLoc(), m_SplineSpace, false);

	for (auto* SpineColl : m_ArySpineColls)
	{
		FVector CollLoc = SpineColl->GetComponentLocation();
		m_SpineSpline->AddSplinePoint(CollLoc, m_SplineSpace, false);

	}
	m_SpineSpline->AddSplinePoint(Get4PinNeckSocLoc(), m_SplineSpace, true);
}

void AUSB_PlayerCharacter::InitSplineMesh()
{
	int MaxColl = m_SpineSpline->GetNumberOfSplinePoints() - 1;

	FTransform Trans;
	float Scale = m_fWorldScaleX * 4.f;
	FVector2D SplineScale(Scale, Scale);
	m_ArySplineMeshCompos.Reset();
	while (MaxColl--)
	{
		USplineMeshComponent* SplineMesh = AddSceneComponent<USplineMeshComponent>(USplineMeshComponent::StaticClass(), nullptr,Trans);

		SplineMesh->SetMobility(EComponentMobility::Movable);
		SplineMesh->SetCollisionProfileName(FName(TEXT("NoCollision")));
		SplineMesh->SetStaticMesh(m_SpineMesh);
		SplineMesh->SetStartScale(SplineScale);
		SplineMesh->SetEndScale(SplineScale);
		m_ArySplineMeshCompos.Emplace(SplineMesh);
	}


	UpdateSplineMesh();
}

void AUSB_PlayerCharacter::UpdateSplineMesh()
{
	FVector PointLoc1;
	FVector PointTangent1;
	FVector PointLoc2;
	FVector PointTangent2;
	FVector UpVector;

	m_SpineSpline->GetLocationAndTangentAtSplinePoint(0, PointLoc1, PointTangent1, m_SplineSpace);

	UpVector = m_SpineSpline->GetUpVectorAtSplinePoint(0, m_SplineSpace);

	for (int i = 0; i < m_ArySplineMeshCompos.Num(); i++)
	{
		m_SpineSpline->GetLocationAndTangentAtSplinePoint(i + 1, PointLoc2, PointTangent2, m_SplineSpace);


		m_ArySplineMeshCompos[i]->SetStartAndEnd(PointLoc1, PointTangent1, PointLoc2, PointTangent2, false);

		PointLoc1 = PointLoc2;
		PointTangent1 = PointTangent2;

		m_ArySplineMeshCompos[i]->SetSplineUpDir(UpVector);
	}
}

void AUSB_PlayerCharacter::SlopeHeadMeshControl(float deltaTime)
{
	FVector TraceStart= GetHeadCollision()->GetComponentLocation();

	float Offset = (m_fLineRadius*m_fWorldScaleX) + 30.f;
	Offset *= -1;

	FVector TraceEnd = TraceStart;
	TraceEnd.Z += Offset;

	FHitResult HitOut;
	FCollisionQueryParams QueryParam;
	QueryParam.AddIgnoredActor(this);

	float YawWant= GetHeadMesh()->GetComponentRotation().Yaw;

	if (GetWorld()->LineTraceSingleByChannel(HitOut, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParam))
	{
	

		FVector ImpactNormal = HitOut.ImpactNormal;
		FVector ForwardVector = GetHeadMesh()->GetForwardVector();
		FVector RightVector = GetHeadMesh()->GetRightVector();


		float RollWant= UKismetMathLibrary::MakeRotFromXZ(ForwardVector, ImpactNormal).Roll;

		float PitchWant = UKismetMathLibrary::MakeRotFromYZ(RightVector, ImpactNormal).Pitch;

		SetHeadRotInterp(FRotator(PitchWant, YawWant, RollWant), deltaTime, 5.f);
	}
	else
	{
		
		SetHeadRotInterp(FRotator(0,YawWant,0), deltaTime, 1.f);
	}

}

void AUSB_PlayerCharacter::SetHeadRotInterp(FRotator targetRot, float deltaTime, float interTime)
{
	FRotator CurrentHeadRot = GetHeadMesh()->GetComponentRotation();


	GetHeadMesh()->SetWorldRotation(FMath::RInterpTo(CurrentHeadRot, targetRot, deltaTime, interTime));
}

void AUSB_PlayerCharacter::InitPhysicsConstraints()
{
	UPhysicsConstraintComponent* UsbPhyCon = AddPhysicsConstraint(m_MeshUSB->GetComponentTransform());
	m_AryPhysicsCompos.Reset();
	m_AryPhysicsCompos.Emplace(UsbPhyCon);
	UsbPhyCon->SetWorldLocation(GetUsbNeckSocLoc());//GetUsbSocLoc
	UsbPhyCon->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	UsbPhyCon->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	UsbPhyCon->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.f);
	UsbPhyCon->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	UsbPhyCon->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	UsbPhyCon->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);

	
	UsbPhyCon->SetConstrainedComponents(m_MeshUSB, m_NameUSBSocket, m_ArySpineColls[0], NAME_None);//Mesh

	FVector CollLocTemp = m_ArySpineColls[0]->GetComponentLocation();

	for (int i = 1; i < m_ArySpineColls.Num(); i++)
	{
		UPhysicsConstraintComponent* SpinePhyCon = AddPhysicsConstraint(m_ArySpineColls[i - 1]->GetComponentTransform());
		m_AryPhysicsCompos.Emplace(SpinePhyCon);

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
	m_AryPhysicsCompos.Emplace(Pin4PhyCon);
	Pin4PhyCon->SetWorldLocation(Get4PinNeckSocLoc());//Get4PinLoc
	Pin4PhyCon->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	Pin4PhyCon->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	Pin4PhyCon->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.f);
	Pin4PhyCon->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	Pin4PhyCon->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	Pin4PhyCon->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	Pin4PhyCon->SetConstrainedComponents(m_Mesh4Pin, m_Name4PinSocket, m_ArySpineColls[m_ArySpineColls.Num() - 1], NAME_None);

}

void AUSB_PlayerCharacter::InitMovement()
{

	UCharacterMovementComponent* MoveCompo= GetCharacterMovement();

	MoveCompo->SetUpdatedComponent(m_CollUSB);

	MoveCompo->SetWalkableFloorAngle(58.f);

	MoveCompo->MaxStepHeight = 15.f;

	MoveCompo->GroundFriction = 2.f;

	MoveCompo->MaxWalkSpeed = 380.f;

	MoveCompo->BrakingDecelerationWalking = 400.f;

	MoveCompo->GravityScale = 2.1f;

	MoveCompo->MaxAcceleration = 600.f;

	MoveCompo->JumpZVelocity = 840.f;

	MoveCompo->AirControl = 1.f;

	MoveCompo->FallingLateralFriction = 1.f;

	//JumpMaxCount = 2;

	//MoveCompo->RotationRate = FRotator(0,0,240.f);

	//MoveCompo->bOrientRotationToMovement = true;

	//bReverSeMoveYaw = true;

}

void AUSB_PlayerCharacter::TryFocusInteract()
{
	FHitResult HitResult;
	FVector StartTrace = GetHeadCollision()->GetComponentLocation();
	FVector EndTrace= (GetController()->GetRootComponent()->GetForwardVector() * m_fInteractRange) + StartTrace;
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (m_InteractObj)
	{
		m_InteractObj->SetFocusOut();
	}

	DrawDebugLine(
		GetWorld(),
		StartTrace,
		EndTrace,
		FColor(255, 0, 0),
		false, -1, 0,
		6.333
	);

	if (GetWorld()->
		LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_GameTraceChannel4, QueryParams))
	{
		UInteractableComponent* InteractComponent = Cast<UInteractableComponent>(HitResult.GetActor()->GetComponentByClass(UInteractableComponent::StaticClass()));

		if (InteractComponent)
		{
			m_InteractObj = InteractComponent;

			m_InteractObj->SetFocusIn();

			return;
		}
	}

	m_InteractObj = nullptr;
}

void AUSB_PlayerCharacter::Kill()
{
	PRINTF("Player Dead");
	m_OnKilled.Broadcast();
	m_GameManager->GameOver();
}

void AUSB_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TryFocusInteract();
	TickUpdateSpine();
	SlopeHeadMeshControl(DeltaTime);
	m_fHeadChangeCDTimer += DeltaTime;

	FVector PlayerPointA = GetConnectorCenterSocLoc();
	FVector PlayerPointB = (GetHeadCollision()->GetForwardVector() * 80.f) + PlayerPointA;
	DrawDebugLine(
		GetWorld(),
		PlayerPointA,
		PlayerPointB,
		FColor(55, 120, 155),
		false, -1, 0,
		6.333
	);
}

void AUSB_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AUSB_PlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUSB_PlayerCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AUSB_PlayerCharacter::RotateYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &AUSB_PlayerCharacter::RotatePitch);
}

FVector AUSB_PlayerCharacter::GetUsbNeckSocLoc()const
{
	return m_MeshUSB->GetSocketLocation(m_NameUSBSocket);
}

FVector AUSB_PlayerCharacter::Get4PinNeckSocLoc()const
{
	return m_Mesh4Pin->GetSocketLocation(m_Name4PinSocket);
}

FVector AUSB_PlayerCharacter::GetConnectorCenterSocLoc()const
{
	return GetHeadMesh()->GetSocketLocation(m_NameConnectSocket);
}

FVector AUSB_PlayerCharacter::GetConnectorPushPosSocLoc() const
{//m_NameConnectPushPointSocket
	return GetHeadMesh()->GetSocketLocation(m_NameConnectPushPointSocket);
}

FVector AUSB_PlayerCharacter::GetConnectorCenterRootSocLoc()const
{
	return GetHeadMesh()->GetSocketLocation(m_NameConnectStartSocket);
}

UPhysicsConstraintComponent* AUSB_PlayerCharacter::AddPhysicsConstraint(const FTransform trans)
{
	auto* Compo=AddSceneComponent<UPhysicsConstraintComponent>(UPhysicsConstraintComponent::StaticClass(), RootComponent, trans);
	return Compo;
}

void AUSB_PlayerCharacter::TryConnect(UPort * connectable)
{
	//interact
	//커넥터블과 나의 타입 비교
	//커넥터블과 나의 삽입구 방향벡터 비교

	DisablePlayerInput();

	bool VerticalAngle = CheckPortVerticalAngle(connectable);
	bool HorizontalAngle = CheckPortHorizontalAngle(connectable);
	if (VerticalAngle)
	{
		PRINTF("1");
	}
	if (HorizontalAngle)
	{
		PRINTF("2");
	}
	if (VerticalAngle && HorizontalAngle)
	{
		ConnectShotSuccess(connectable);
	}
	else
	{
		ConnectShotFail();
	}
}

void AUSB_PlayerCharacter::TryDisconnect()
{




}


bool AUSB_PlayerCharacter::CheckPortHorizontalAngle(UPort * port)
{
	FVector PlayerPointA;
	FVector PlayerPointB;
	GetPortCenterTracePoint(PlayerPointA, PlayerPointB,80.f);

	FVector PlayerConnectDir = UKismetMathLibrary::Normal(PlayerPointB - PlayerPointA, 0.0001f);
	FVector ConnectPortDir = port->GetForwardVector();
	ConnectPortDir *= -1;
	float Dot = UKismetMathLibrary::Dot_VectorVector(PlayerConnectDir, ConnectPortDir);

	return UKismetMathLibrary::Acos(Dot) <= m_fConnectHorizontalAngle;
}

bool AUSB_PlayerCharacter::CheckPortVerticalAngle(UPort * port)
{
	return UKismetMathLibrary::Abs(port->GetUpVector().Z - GetHeadCollision()->GetUpVector().Z) <= 0.1f;
}
void AUSB_PlayerCharacter::ConnectShotSuccess(UPort * portToAdd)
{
	PRINTF("SUCCESS");
	m_ActionManager->RemoveAllActions();
	
	portToAdd->DisableCollider();
	//
	auto* Sequence=UCActionFactory::MakeSequenceAction();

	Sequence->AddAction(MoveForReadyConnect(portToAdd));

	Sequence->AddAction(RotateForConnect(portToAdd));

	auto* PushAction = MoveForPushConnection(portToAdd);

	Sequence->AddAction(PushAction);

	PushAction->m_OnActionComplete.BindLambda( 
		[=]()
		{
	
		m_ConnectorHead->Connect(portToAdd);
		EnablePlayerInput();
		});

	m_ActionManager->RunAction(Sequence);
}

void AUSB_PlayerCharacter::ConnectShotFail()
{
	PRINTF("Fail");

	auto* Action=UCActionFactory::MakeMoveForceMoveComponentToAction(GetCharacterMovement(), GetHeadMesh()->GetForwardVector() * 310000.f,0.5f);

	Action->m_OnActionTick.BindUObject(this,&AUSB_PlayerCharacter::TickCastForKnockBack);
	Action->m_OnActionComplete.BindUObject(this, &AUSB_PlayerCharacter::EnablePlayerInput);

	m_ActionManager->RunAction(Action);
}

UCActionBaseInterface* AUSB_PlayerCharacter::MoveForReadyConnect(UPort * portWant)
{
	FVector HeadLoc = GetHeadCollision()->GetComponentLocation();
	FVector ConnectLoc = portWant->GetComponentLocation();//여기에서 x축에
	FVector PushReadySocLoc = GetConnectorPushPosSocLoc();
	FVector Dest = HeadLoc - PushReadySocLoc + ConnectLoc;

	auto* MoveAction = UCActionFactory::MakeMoveComponentToAction(GetHeadCollision(), Dest, 0.5f, ETimingFunction::EaseInCube);

	return MoveAction;
}

UCActionBaseInterface* AUSB_PlayerCharacter::RotateForConnect(UPort * portWant)
{
	 FRotator ConnectRot = portWant->GetComponentRotation();

	 ConnectRot.Yaw += 180;

	 auto* Action= UCActionFactory::MakeRotateComponentToAction(GetHeadCollision(),ConnectRot,0.2f, ETimingFunction::Linear);

	 return Action;
}

UCActionBaseInterface* AUSB_PlayerCharacter::MoveForPushConnection(UPort * portWant)
{
	FVector HeadLoc = GetHeadCollision()->GetComponentLocation();
	FVector ConnectLoc = portWant->GetComponentLocation();//여기에서 x축에
	FVector ConnectSocStartLoc = GetConnectorCenterRootSocLoc();
	FVector Dest = HeadLoc - ConnectSocStartLoc + ConnectLoc;

	auto* MoveAction = UCActionFactory::MakeMoveComponentToAction(GetHeadCollision(), Dest, 0.3f, ETimingFunction::EaseInCube);

	return MoveAction;
}

void AUSB_PlayerCharacter::TickCastForKnockBack(float time)
{
	FHitResult HitResult;
	FVector StartPoint;
	FVector EndPoint;

	const float PitchOffset = 20.f;
	const float YawOffset = GetHeadMesh()->RelativeRotation.Yaw;
	float RollOffset = 0.f;

	GetPortCenterTracePoint(StartPoint, EndPoint, 4.f);
	if (KnockBackCast(StartPoint, EndPoint, HitResult, FRotator(PitchOffset, YawOffset, RollOffset)))
	{
		return;
	}

	GetPortRightSideTracePoint(StartPoint, EndPoint, 4.f);
	RollOffset = -1.2f*PitchOffset;
	if (KnockBackCast(StartPoint, EndPoint, HitResult, FRotator(PitchOffset, YawOffset, RollOffset)))
	{
		return;
	}

	GetPortLeftSideTracePoint(StartPoint, EndPoint, 4.f);
	RollOffset = 1.2f*PitchOffset;
	KnockBackCast(StartPoint, EndPoint, HitResult, FRotator(PitchOffset, YawOffset, RollOffset));
}

bool AUSB_PlayerCharacter::KnockBackCast(const FVector & StartPoint, const FVector & EndPoint, FHitResult & HitResult, FRotator rotateWant)
{
	if (UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), StartPoint, EndPoint, m_TraceTypes,
		false, m_IgnoreActors, EDrawDebugTrace::ForDuration, HitResult, true, FLinearColor::Green))
	{
		HitResult.Component.Get()->AddImpulse(GetHeadMesh()->GetForwardVector() * m_fConnectFailImpulseOther);

		GetHeadMesh()->SetRelativeRotation(rotateWant);
		CancelConnetFailShot();
		return true;
	}
	return false;
}

void AUSB_PlayerCharacter::CancelConnetFailShot()
{
	m_ActionManager->RemoveAllActions();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->AddImpulse(GetHeadMesh()->GetForwardVector() * - m_fConnectFailImpulseSelf);
	EnablePlayerInput();
	//Rotate
}

void AUSB_PlayerCharacter::TickUpdateSpine()
{
	UpdateSplinePoint();
	UpdateSplineMesh();
}

void AUSB_PlayerCharacter::EnablePlayerInput()
{
	EnableInput(m_PlayerController);
}

void AUSB_PlayerCharacter::DisablePlayerInput()
{
	DisableInput(m_PlayerController);
}

void AUSB_PlayerCharacter::Interact()
{
	if (!m_InteractObj)
	{
		return;
	}
	m_InteractObj->Interact(this);
	PRINTF("!! %s", *m_InteractObj->GetOwner()->GetName());
}

void AUSB_PlayerCharacter::ChangeHead()
{

	if (m_fHeadChangeCDTimer < m_fHeadChangeCD)
	{
		return;
	}
	m_fHeadChangeCDTimer = 0.f;

	auto CurrentHead = m_ConnectorHead;
	auto CurrentTail = m_ConnectorTail;

	SetHeadTail(m_ConnectorTail, m_ConnectorHead);
}

void AUSB_PlayerCharacter::SetHeadTailDefault()
{
	SetHeadTail(m_ConnectorHead, m_ConnectorTail);
}

 UCapsuleComponent* AUSB_PlayerCharacter::GetHeadCollision() const
{
	//return m_ConnectorHead->GetColl();
	return nullptr;
}

 UCapsuleComponent* AUSB_PlayerCharacter::GetTailCollision() const
{
	 return nullptr;
	//return m_ConnectorTail->GetColl();
}

 USkeletalMeshComponent* AUSB_PlayerCharacter::GetHeadMesh() const
{
	 return nullptr;
	//return m_ConnectorHead->GetMesh();
}

 USkeletalMeshComponent* AUSB_PlayerCharacter::GetTailMesh() const
{
	 //return m_ConnectorTail->GetMesh();
	 return nullptr;
}

 FVector AUSB_PlayerCharacter::GetConnectorRightSocLoc() const
 {
	 //return m_ConnectorHead->GetMesh()->GetSocketLocation(m_NameConnectRightSide);
	 return FVector();
 }

 FVector AUSB_PlayerCharacter::GetConnectorLeftSocLoc() const
 {
	 //return m_ConnectorHead->GetMesh()->GetSocketLocation(m_NameConnectLeftSide);
	 return FVector();
 }

void AUSB_PlayerCharacter::MoveForward(float v)
{
	if ((Controller != NULL) && (v != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		if (bCantDown)
		{
			v=FMath::Clamp<float>(v, 0, 1);
		}
		
		if (bCantUp)
		{
			v=FMath::Clamp<float>(v, -1, 0);
		}

		AddMovementInput(Direction, v);
	}
}

void AUSB_PlayerCharacter::MoveRight(float v)
{
	//인풋의 방향을 인버스 월드?
	if ((Controller != NULL) && (v != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		if (bCantRight)
		{
			v=FMath::Clamp<float>(v, -1, 0);
		}

		if (bCantLeft)
		{
			v=FMath::Clamp<float>(v, 0, 1);
		}

		AddMovementInput(Direction, v);
	}
}

void AUSB_PlayerCharacter::RotateYaw(float v)
{
	if (!m_bCanCamRotate)
	{
		return;
	}

	AddControllerYawInput(v);
}

void AUSB_PlayerCharacter::RotatePitch(float v)
{
	if (!m_bCanCamRotate)
	{
		return;
	}

	AddControllerPitchInput(v);
}

void AUSB_PlayerCharacter::GetPortCenterTracePoint(FVector & startPoint, FVector & endPoint, float length)
{
	startPoint = GetConnectorCenterSocLoc();
	endPoint = (GetHeadMesh()->GetForwardVector() * length) + startPoint;
}

void AUSB_PlayerCharacter::GetPortLeftSideTracePoint(FVector & startPoint, FVector & endPoint, float length)
{
	startPoint = GetConnectorLeftSocLoc();
	endPoint = (GetHeadMesh()->GetForwardVector() * length) + startPoint;
}

void AUSB_PlayerCharacter::GetPortRightSideTracePoint(FVector & startPoint, FVector & endPoint, float length)
{
	startPoint = GetConnectorRightSocLoc();
	endPoint = (GetHeadMesh()->GetForwardVector() * length) + startPoint;
}



