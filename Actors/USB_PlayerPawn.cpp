// Fill out your copyright notice in the Description page of Project Settings.


#include "USB_PlayerPawn.h"
#include "ConstructorHelpers.h"
#include "Datas/USB_Macros.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimBlueprint.h"
#include "Components/PhysicsSkMeshComponent.h"
#include "Components/PortSkMeshComponent.h"
#include "Actors/PortPawn.h"
#include "GameFramework/PlayerController.h"
#include "UObjects/EjectionCamShake.h"



//누구이든간 스켈메쉬를 머리꼬리로 바꿔버리니까
//어댑터가 꼬리면 어댑터를 분리해야하고
//어댑터가 없거나 어댑터 연결이 없으면 꼬리에 붙어있는 포트를
//분리해야한다
AUSB_PlayerPawn::AUSB_PlayerPawn(const FObjectInitializer& objInit):Super(objInit)
{
	m_fMaxConnectRotTime = 3.f;
	m_fMinConnectRotTime = 1.f;
	m_fDefaultFailImpulsePower = 1000.f;
	InitPlayerPawn();
	CreatePhysicMovement();
	CreateCameraFamily();
	CreateSkFaceMesh();

	m_ActionManager = CreateDefaultSubobject<UActionManagerComponent>("ActionManager00");

	m_AryPhysicsBody.Reserve(10);

	m_bBlockChargeClick = false;
	m_bBlockHeadChange = false;
	m_bBlockJump = false;

}

void AUSB_PlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	for (auto* Sphere : m_ArySpineColls)
	{
		AddPhysicsBody(Sphere);
	}
	AddPhysicsBody(m_PinUSB);
	AddPhysicsBody(m_Pin5Pin);
	m_PlayerCon = Cast<APlayerController>(GetController());
	SetHeadTail(m_CurrentHead, m_CurrentTail);
	InitTraceIgnoreAry();
	Cast<UPinSkMeshComponent>(m_CurrentHead)->m_fFailImpulsePower = m_fDefaultFailImpulsePower;
	Cast<UPinSkMeshComponent>(m_CurrentTail)->m_fFailImpulsePower = m_fDefaultFailImpulsePower;
}

USceneComponent * AUSB_PlayerPawn::GetFocusedPortTarget()
{
	return m_CurrentFocusedPort->GetParentSkMesh();
}

void AUSB_PlayerPawn::ZoomIn()
{
	m_MainSpringArm->ZoomIn();
}

void AUSB_PlayerPawn::ZoomOut()
{
	m_MainSpringArm->ZoomOut();
}

void AUSB_PlayerPawn::InitPlayerPawn()
{
	m_fConnectHorizontalAngle = 0.06f;
	m_fEjectionPower = 4200.f;
	m_fSpineAngularDamping = 1.f;
	m_fSpineLinearDamping = 0.01f;
	m_fCollMass = 1.f;
	m_fMaxAngularVelocity = 150.f;
	m_fBlockMoveTimeWhenEject = 1.5f;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	m_CurrentHead = m_PinUSB;
	m_CurrentTail = m_Pin5Pin;
	m_BaseHeadPin = Cast<UPinSkMeshComponent>( m_CurrentHead);
	m_BaseTailPin = Cast<UPinSkMeshComponent>(m_CurrentTail);
	m_fPortTraceRange = 77.f;
	m_fHeadChangeCD = 0.5f;
	m_fHeadChangeCDTimer = 0.f;
	
	m_fConnectReadyDuration = 5.f;
	m_fConnectPushDuration = 0.3f;
}

void AUSB_PlayerPawn::CreatePhysicMovement()
{
	m_UsbMovement = CreateDefaultSubobject<UPhysicsMovement>(TEXT("Movement00"));

	m_UsbMovement->SetUpdatedComponent(m_CurrentHead);

	m_UsbMovement->m_fMovingForce=38000.f;
	m_UsbMovement->m_fGroundCastBoxSize = 10.f;
	m_UsbMovement->m_fGroundCastOffset = -20.f;
	m_UsbMovement->m_fWalkableSlopeAngle = 55.f;
	m_UsbMovement->m_fJumpZVelocity = 2000.f;
	m_UsbMovement->m_nJumpMaxCount = 2;
	m_UsbMovement->m_fAirControl = 0.6f;
}

void AUSB_PlayerPawn::CreateCameraFamily()
{
	m_CamRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CamRoot"));
	m_CamRoot->SetupAttachment(m_PinUSB);
	m_CamRoot->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	//
	m_MainSpringArm = CreateDefaultSubobject<UUSB_SpringArm>(TEXT("Spring00"));
	m_MainSpringArm->SetupAttachment(m_CamRoot);
	m_MainSpringArm->TargetArmLength = 600.f;
	m_MainSpringArm->SocketOffset = FVector(0, 0, 0);
	m_MainSpringArm->TargetOffset = FVector(0.f, 0.f, 0.f);
	m_MainSpringArm->m_RotOffset = FRotator(-30.f,0.f,0.f);
	m_MainSpringArm->m_fMinimumArmLength = 33.f;
	m_MainSpringArm->bUsePawnControlRotation = true;
	m_MainSpringArm->bInheritRoll = false;
	m_MainSpringArm->bEnableCameraLag = true;
	m_MainSpringArm->bEnableCameraRotationLag = true;
	m_MainSpringArm->CameraLagSpeed = 10.f;
	m_MainSpringArm->CameraRotationLagSpeed = 10.f;
	m_MainSpringArm->bAutoActivate = true;
	//
	m_MainCam = CreateDefaultSubobject<UCameraComponent>(TEXT("Cam00"));
	m_MainCam->SetupAttachment(m_MainSpringArm);
	m_MainCam->bAutoActivate = true;
	m_MainCam->FieldOfView = 100.f;
}

void AUSB_PlayerPawn::CreateSkFaceMesh()
{
	m_MeshFaceSk = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FaceSkMesh00"));
	m_MeshFaceSk->SetupAttachment(m_PinUSB);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundSkMesh(TEXT("SkeletalMesh'/Game/Meshes/Characters/Player_USB/SK_USB_Face.SK_USB_Face'"));

	if (FoundSkMesh.Succeeded())
	{
		m_MeshFaceSk->SetSkeletalMesh(FoundSkMesh.Object);
	}

	m_MeshFaceSk->SetRelativeLocation(FVector(-28.f,0, 0.8f));
	m_MeshFaceSk->SetRelativeScale3D(FVector(2.64f, 2.64f, 2.64f));
}

void AUSB_PlayerPawn::SetHeadTail(UPhysicsSkMeshComponent * headWant, UPhysicsSkMeshComponent * tailWant)
{
	m_CurrentHead->OnComponentBeginOverlap.RemoveAll(this);//remove from older

	m_CurrentHead = headWant;
	m_CurrentTail = tailWant;

	m_UsbMovement->SetUpdatedComponent(m_CurrentHead);
	
	m_CamRoot->AttachToComponent(m_CurrentHead,FAttachmentTransformRules::KeepRelativeTransform);

	m_CurrentHead->OnComponentBeginOverlap.AddDynamic(this,&AUSB_PlayerPawn::TryConnect);

	m_CurrentHeadPin = Cast<UPinSkMeshComponent>(m_CurrentHead);
}

void AUSB_PlayerPawn::AddIgnoreActorsToQuery(FCollisionQueryParams & queryParam)
{
	queryParam.AddIgnoredActors(m_AryTraceIgnoreActors);
}

void AUSB_PlayerPawn::SetupPlayerInputComponent(UInputComponent * PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AUSB_PlayerPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUSB_PlayerPawn::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AUSB_PlayerPawn::RotateYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &AUSB_PlayerPawn::RotatePitch);
	//Connect
	PlayerInputComponent->BindAction(FName("Connect"),EInputEvent::IE_Pressed,this, &AUSB_PlayerPawn::ConnectShot);
	PlayerInputComponent->BindAction(FName("Jump"), EInputEvent::IE_Pressed, this, &AUSB_PlayerPawn::Jump);
	PlayerInputComponent->BindAction(FName("Jump"), EInputEvent::IE_Released, this, &AUSB_PlayerPawn::StopJumping);
	PlayerInputComponent->BindAction(FName("HeadChange"), EInputEvent::IE_Pressed, this, &AUSB_PlayerPawn::ChangeHeadTail);
	PlayerInputComponent->BindAction(FName("Disconnect"), EInputEvent::IE_Pressed, this, &AUSB_PlayerPawn::DisconnectShot);
	PlayerInputComponent->BindAction(FName("ZoomIn"), EInputEvent::IE_Pressed, this, &AUSB_PlayerPawn::ZoomIn);
	PlayerInputComponent->BindAction(FName("ZoomOut"), EInputEvent::IE_Pressed, this, &AUSB_PlayerPawn::ZoomOut);
}

void AUSB_PlayerPawn::InitTraceIgnoreAry()
{
	m_AryTraceIgnoreActors.Reset();
	m_AryTraceIgnoreActors.Reserve(10);
	AddTraceIgnoreActor(this);
	//m_UsbMovement->AddIgnoreActorsToQuery(&m_AryTraceIgnoreActors);
}

void AUSB_PlayerPawn::Tick(float DeltaTime)
{
	m_fHeadChangeCDTimer += DeltaTime;
	Super::Tick(DeltaTime);
	TickTracePortable();

	if (!m_CurrentFocusedPort)
	{
		return;
	}

	FVector HeadPos = GetHead()->GetComponentLocation();
	FVector PortPst = m_CurrentFocusedPort->GetComponentLocation();
	DrawDebugLine(GetWorld(), HeadPos, PortPst, FColor::Cyan, false, -1, 0.1f);
}

void AUSB_PlayerPawn::MoveForward(float v)
{
	if ((Controller != NULL) && (v != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, v);
	}
}

void AUSB_PlayerPawn::MoveRight(float v)
{
	if ((Controller != NULL) && (v != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, v);
	}
}

void AUSB_PlayerPawn::RotatePitch(float v)
{
	AddControllerPitchInput(v);
}
void AUSB_PlayerPawn::RotateYaw(float v)
{
	AddControllerYawInput(v);
}

void AUSB_PlayerPawn::ConnectShot()
{

	if (m_bBlockChargeClick)
	{
		return;
	}

	if (!m_CurrentFocusedPort)
	{
		return;
	}

	if (!m_CurrentHeadPin)
	{
		return;
	}

	if (m_CurrentHeadPin->GetPortConnected())
	{
		return;
	}



	if (!m_UsbMovement->IsMovingOnGround())//sky connect
	{ 
		//공중삽입의 문제점

		float Distance = FVector::Dist(m_CurrentHeadPin->GetComponentLocation(), m_CurrentFocusedPort->GetComponentLocation());

		PRINTF("Distance is : %f", Distance);
		//77 일때 3
		float Rate = Distance / m_fPortTraceRange;
		Rate *= m_fMaxConnectRotTime;

		if (Rate < m_fMinConnectRotTime)
		{
			Rate = m_fMinConnectRotTime;
		}

		PRINTF("RateTime IS : %f", Rate);

	}
	ConnectChargingStart();
}


void AUSB_PlayerPawn::TryConnect(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	PRINTF("TryConnect - Called");
	auto* PortCompo = Cast<UPortSkMeshComponent>(OtherComp);

	if(!PortCompo)
	{
		PRINTF("FailConnect - Port Is Different");
		FailConnection(PortCompo,SweepResult);
		return;
	}

	if (!PortCompo->CheckConnectTransform(GetHead(), m_UsbMovement->IsMovingOnGround()))
	{
		PRINTF("FailConnect - Rotation");
		FailConnection(PortCompo, SweepResult);
		return;
	}

	if (!m_CurrentHeadPin->Connect(PortCompo))
	{
		PRINTF("FailConnect - PinType");
		FailConnection(PortCompo, SweepResult);
		return;
	}

	SuccessConnection(PortCompo);
}

void AUSB_PlayerPawn::ConnectChargingStart()
{
	PRINTF("Charging Start");
	//m_bBlockChargeClick = true;
	//m_CurrentHead->SetGenerateOverlapEvents(true);
	//DisableUSBInput();
	//m_ConnectChargingHandle = m_UsbMovement->m_OnAutoMoveEnd.AddUObject(this, &AUSB_PlayerPawn::ConnectChargingEnd);
	//EnableAutoMove(m_CurrentHead->GetForwardVector(), 3.f);
}

void AUSB_PlayerPawn::ConnectChargingEnd()//사실상 실패랑 똑같은데 팅기는것만 없어야함
{ 
	//지속시간 끝나고 불리는데 성공이후로도 불리고 있음
	PRINTF("Charging End, it should be call once");
	//m_bBlockChargeClick = false;
	//m_CurrentHead->SetGenerateOverlapEvents(false);
	//EnableUSBInput();
	//m_UsbMovement->m_OnAutoMoveEnd.Remove(m_ConnectChargingHandle);
}
//움직임 끊기 존재해야함
void AUSB_PlayerPawn::SuccessConnection(UPortSkMeshComponent* portConnect)
{
	PRINTF("SuccessConnection");
	
	FVector PortPoint = portConnect->GetParentSkMesh()->GetBoneLocation("PortPoint");
	FVector ConnectPoint = PortPoint + (m_CurrentHeadPin->GetComponentLocation() - m_CurrentHeadPin->GetSocketLocation("PinPoint"));
	FRotator PortRot = portConnect->GetParentSkMesh()->GetComponentRotation();
	m_CurrentHeadPin->SetWorldLocationAndRotationNoPhysics(ConnectPoint, PortRot);
	portConnect->Connect(m_CurrentHeadPin);//이거 순서를 바꿔볼것
	SetHeadTail(portConnect->GetParentSkMesh(), m_CurrentTail);
	AddTraceIgnoreActor(portConnect->GetOwner());
	AddPhysicsBody(portConnect->GetParentSkMesh());
}

void AUSB_PlayerPawn::FailConnection(UPortSkMeshComponent* portConnect,const FHitResult & hitResult)
{//움직이는 중간에 끊기는거
	PRINTF("FailConnection");
	portConnect->FailConnection(hitResult);
	m_CurrentHeadPin->FailConnection(hitResult);
}



void AUSB_PlayerPawn::ChangeHeadTail()
{
	if (m_fHeadChangeCDTimer < m_fHeadChangeCD)
	{
		return;
	}

	if (m_bBlockHeadChange)
	{
		return;
	}
	m_fHeadChangeCDTimer = 0.f;
	
	auto* BeforeHead = m_BaseHeadPin;
	m_BaseHeadPin = m_BaseTailPin;
	m_BaseTailPin = BeforeHead;

	SetHeadTail(m_CurrentTail,m_CurrentHead);
}

void AUSB_PlayerPawn::AddPhysicsBody(UPrimitiveComponent * wantP)
{
	m_AryPhysicsBody.Emplace(wantP);
	m_fTotalMass += wantP->GetMass();
}

void AUSB_PlayerPawn::RemovePhysicsBody(UPrimitiveComponent * wantP)
{
	m_fTotalMass -= wantP->GetMass();
	m_AryPhysicsBody.Remove(wantP);
}

void AUSB_PlayerPawn::SetPhysicsVelocityAllBody(FVector linearV)
{
	for (auto* Phy : m_AryPhysicsBody)
	{
		Phy->SetPhysicsLinearVelocity(linearV);
		Phy->SetPhysicsAngularVelocityInDegrees(linearV);
	}
}

bool AUSB_PlayerPawn::TryDisconnect()
{
	auto* Tail = Cast<UPinSkMeshComponent>(GetTail());

	if (!Tail)
	{
		Tail = m_BaseTailPin;
	}

	if (!Tail->GetPortConnected())
	{
		return false;
	}

	RemoveTraceIgnoreActor(Tail->GetPortConnected()->GetOwner());
	RemovePhysicsBody(Tail->GetPortConnected());
	Tail->Disconnect();
	SetHeadTail(m_CurrentHead, Tail);
	return true;
}

void AUSB_PlayerPawn::AddTraceIgnoreActor(AActor * actorWant)
{
	m_AryTraceIgnoreActors.Emplace(actorWant);
}

bool AUSB_PlayerPawn::RemoveTraceIgnoreActor(AActor * actorWant)
{
	return m_AryTraceIgnoreActors.Remove(actorWant);
}

UPhysicsSkMeshComponent * AUSB_PlayerPawn::GetHead()
{
	return _inline_GetHead();
}

UPhysicsSkMeshComponent * AUSB_PlayerPawn::GetTail()
{
	return _inline_GetTail();
}

void AUSB_PlayerPawn::DisconnectShot()
{
	if (TryDisconnect())
	{
		FVector ImpulseDir = GetTail()->GetForwardVector()*-1.f * m_fEjectionPower;
		//m_UsbMovement->AddImpulse(ImpulseDir);
		m_PlayerCon->PlayerCameraManager->PlayCameraShake(UEjectionCamShake::StaticClass(),1.0f);
	}
}

void AUSB_PlayerPawn::Jump()
{
	if(!m_bBlockJump)
	m_UsbMovement->Jump();
}

void AUSB_PlayerPawn::StopJumping()
{
	m_UsbMovement->StopJumping();
}

void AUSB_PlayerPawn::TickTracePortable()
{
	FHitResult HitResult;
	FVector StartTrace = GetHead()->GetComponentLocation();
	FVector Forward = GetHead()->GetForwardVector();
	FVector EndTrace = (GetHead()->GetForwardVector()* m_fPortTraceRange) + StartTrace;

	FCollisionQueryParams QueryParams;
	AddIgnoreActorsToQuery(QueryParams);

	
	if (GetWorld()->SweepSingleByChannel(HitResult, StartTrace, EndTrace, FQuat::Identity, ECC_GameTraceChannel9, FCollisionShape::MakeSphere(3.f), QueryParams))
	{
		if (!HitResult.GetActor())
		{
			return;
		}

		UPortSkMeshComponent* PortableCompo = Cast<UPortSkMeshComponent>(HitResult.GetActor()->GetComponentByClass(UPortSkMeshComponent::StaticClass())); 

		if (PortableCompo&& !PortableCompo->GetPinConnected())
		{
			m_CurrentFocusedPort = PortableCompo;
			return;
		}
	}
	m_CurrentFocusedPort = nullptr;
}


