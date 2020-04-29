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
#include "Components/Battery.h"


//누구이든간 스켈메쉬를 머리꼬리로 바꿔버리니까
//어댑터가 꼬리면 어댑터를 분리해야하고
//어댑터가 없거나 어댑터 연결이 없으면 꼬리에 붙어있는 포트를
//분리해야한다
AUSB_PlayerPawn::AUSB_PlayerPawn(const FObjectInitializer& objInit) :Super(objInit)
{
	m_PortHeadPrev = nullptr;
	m_PortTailPrev = nullptr;
	m_fBlockMoveTimeWhenEjectTimer = 0.f;
	m_fDefaultFailImpulsePower = 1000.f;
	InitPlayerPawn();
	CreatePhysicMovement();
	CreateCameraFamily();
	CreateSkFaceMesh();

	m_ReadyActionManager = CreateDefaultSubobject<UActionManagerComponent>("ActionManager00");
	m_ConnectActionManager = CreateDefaultSubobject<UActionManagerComponent>("ActionManager01");

	m_AryPhysicsBody.Reserve(10);

	m_bBlockChargeClick = false;
	m_bBlockHeadChange = false;
	m_bBlockJump = false;
	m_bBlockInputMove = false;
}

void AUSB_PlayerPawn::BeginPlay()
{
	InitUSB();

	m_UsbMovement->InitUSBUpdateComponent(this, m_CurrentHead, m_CurrentTail);
	for (auto* Sphere : m_ArySpineColls)
	{
		AddPhysicsBody(Sphere);
	}

	AddPhysicsBody(m_PinUSB);
	AddPhysicsBody(m_Pin5Pin);

	m_PlayerCon = Cast<APlayerController>(GetController());

	SetHeadTail(m_CurrentHead, m_CurrentTail, nullptr, nullptr);

	InitTraceIgnoreAry();

	Cast<UPinSkMeshComponent>(m_CurrentHead)->m_fFailImpulsePower = m_fDefaultFailImpulsePower;
	Cast<UPinSkMeshComponent>(m_CurrentTail)->m_fFailImpulsePower = m_fDefaultFailImpulsePower;

	APawn::BeginPlay();

	m_fMaxSpeedSqr = m_UsbMovement->GetMaxForce() *m_UsbMovement->GetMaxForce();
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

UBattery * AUSB_PlayerPawn::GetBattery()
{
	for (auto Bat : m_AryBatteries)
	{
		if(Bat)
			return Bat;
	}

	return nullptr;
}

void AUSB_PlayerPawn::Kill()
{
}

void AUSB_PlayerPawn::InitPlayerPawn()
{
	m_fSpineAngularDamping = 1.f;
	m_fSpineLinearDamping = 0.01f;
	m_fCollMass = 1.f;
	m_fMaxAngularVelocity = 150.f;
	m_fBlockMoveTimeWhenEject = 1.5f;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	m_CurrentHead = m_PinUSB;
	m_CurrentTail = m_Pin5Pin;

	m_BaseHeadPin = Cast<UPinSkMeshComponent>(m_CurrentHead);
	m_BaseTailPin = Cast<UPinSkMeshComponent>(m_CurrentTail);

	m_fPortTraceRange = 77.f;
	m_fHeadChangeCD = 0.2f;
	m_fHeadChangeCDTimer = 0.f;

	m_BaseHeadPin->SetGenerateOverlapEvents(true);
	m_BaseTailPin->SetGenerateOverlapEvents(true);

	m_BaseHeadPin->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	m_BaseTailPin->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	m_BaseHeadPin->SetUseCCD(true);
	m_BaseTailPin->SetUseCCD(true);
}

void AUSB_PlayerPawn::CreatePhysicMovement()
{
	m_UsbMovement = CreateDefaultSubobject<UUSBMovement>(TEXT("Movement00"));
	m_UsbMovement->m_fWalkableSlopeAngle = 49.f;
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
	m_MainSpringArm->m_RotOffset = FRotator(-30.f, 0.f, 0.f);
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

	m_MeshFaceSk->SetRelativeLocation(FVector(-10.245544f, 0, 2.795712f));//(X=-10.245544,Y=0.000000,Z=2.795712)
	m_MeshFaceSk->SetRelativeScale3D(FVector(2.4f, 2.4f, 2.4f));
}

void AUSB_PlayerPawn::SetHeadTail(UPhysicsSkMeshComponent * headWant, UPhysicsSkMeshComponent * tailWant,
	UPortSkMeshComponent* headPrevPort, UPortSkMeshComponent* tailPrevPort, bool bRemoveIgnoreOld)
{
	m_CurrentHead = headWant;
	m_CurrentTail = tailWant;

	m_PortHeadPrev = headPrevPort;
	m_PortTailPrev = tailPrevPort;

	m_UsbMovement->SetUSBUpdateComponent(m_CurrentHead, m_CurrentTail, bRemoveIgnoreOld);

	m_CamRoot->AttachToComponent(m_CurrentHead, FAttachmentTransformRules::KeepRelativeTransform);


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
	PlayerInputComponent->BindAction("Connect", EInputEvent::IE_Pressed, this, &AUSB_PlayerPawn::ConnectShot);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AUSB_PlayerPawn::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &AUSB_PlayerPawn::StopJumping);
	PlayerInputComponent->BindAction("HeadChange", EInputEvent::IE_Pressed, this, &AUSB_PlayerPawn::ChangeHeadTail);
	PlayerInputComponent->BindAction("Disconnect", EInputEvent::IE_Pressed, this, &AUSB_PlayerPawn::DisconnectShot);
	PlayerInputComponent->BindAction("ZoomIn", EInputEvent::IE_Pressed, this, &AUSB_PlayerPawn::ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", EInputEvent::IE_Pressed, this, &AUSB_PlayerPawn::ZoomOut);
	//
	PlayerInputComponent->BindAction("ExitGame", EInputEvent::IE_Released, this, &AUSB_PlayerPawn::ExitGame);
}
void AUSB_PlayerPawn::ExitGame()
{
	PRINTF("Exot");
	UKismetSystemLibrary::QuitGame(GetWorld(), m_PlayerCon, EQuitPreference::Quit, true);
}


void AUSB_PlayerPawn::InitTraceIgnoreAry()
{
	m_AryTraceIgnoreActors.Reset();
	m_AryTraceIgnoreActors.Reserve(10);
	AddTraceIgnoreActor(this);
}

void AUSB_PlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_fHeadChangeCDTimer += DeltaTime;

	if (m_fBlockMoveTimeWhenEjectTimer > 0 && m_fBlockMoveTimeWhenEjectTimer !=-1.f)//0
	{
		m_fBlockMoveTimeWhenEjectTimer -= DeltaTime;

		if (m_fBlockMoveTimeWhenEjectTimer <= 0 || IsImpulseVelocityLower())
		{
			EnableUSBInput();
			m_fBlockMoveTimeWhenEjectTimer = 0.f;
		}
	}

	TickTracePortable();

	if (!m_CurrentFocusedPort)
	{
		return;
	}

	FVector HeadPos = GetHead()->GetComponentLocation();
	FVector PortPst = m_CurrentFocusedPort->GetComponentLocation();
	DrawDebugLine(GetWorld(), HeadPos, PortPst, FColor::Cyan, false, -1, 0.1f);
	//
	
}

void AUSB_PlayerPawn::EnableUSBInput()
{
	m_bBlockChargeClick = false;
	m_bBlockHeadChange = false;
	m_bBlockInputMove = false;
	m_bBlockJump = false;
	m_fBlockMoveTimeWhenEjectTimer = 0.f;
	PRINTF("EnableINput");
}

void AUSB_PlayerPawn::DisableUSBInput(float dur)
{
	m_bBlockChargeClick = true;
	m_bBlockHeadChange = true;
	m_bBlockInputMove = true;
	m_bBlockJump = true;

	m_fBlockMoveTimeWhenEjectTimer = dur;

	PRINTF("DisableInput");
}

void AUSB_PlayerPawn::EnableUSBMove()
{
	m_bBlockChargeClick = false;
	//m_bBlockInputMove = false;
	m_bBlockJump = false;
	m_fBlockMoveTimeWhenEjectTimer = 0.f;
	PRINTF("EnableMoveINput");
}

void AUSB_PlayerPawn::DisableUSBMove(float dur)
{
	m_bBlockChargeClick = true;
	//m_bBlockInputMove = true;
	m_bBlockJump = true;
	m_fBlockMoveTimeWhenEjectTimer = dur;
	PRINTF("DisableMoveINput");
}

void AUSB_PlayerPawn::AddBattery(UBattery * batt)
{
	m_AryBatteries.Add(batt);
}

void AUSB_PlayerPawn::RemoveBattery(UBattery * batt)
{
	m_AryBatteries.Remove(batt);
}

void AUSB_PlayerPawn::MoveForward(float v)
{
	if ((Controller != NULL) && (v != 0.0f) && !m_bBlockInputMove)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, v);
	}
}

void AUSB_PlayerPawn::MoveRight(float v)
{
	if ((Controller != NULL) && (v != 0.0f) && !m_bBlockInputMove)
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


	DisableUSBInput();

	ConnectChargingStart();
}


void AUSB_PlayerPawn::TryConnect(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	PRINTF("TryConnect - Called : %s",*m_CurrentHead->GetOwner()->GetName());

	auto* PortCompo = Cast<UPortSkMeshComponent>(OtherComp);

	PRINTF("Compo1 : %s", *OverlappedComponent->GetName());
	PRINTF("Compo2 : %s", *OtherComp->GetName());

	if (!PortCompo)
	{
		FailConnection(PortCompo, &SweepResult, EFailConnectionReason::PortNotFoundCast);
		return;
	}

	if (!PortCompo->CheckConnectTransform(GetHead(), m_UsbMovement->IsMovingOnGround()))
	{
		FailConnection(PortCompo, &SweepResult, EFailConnectionReason::RotationNotMatch);
		return;
	}

	if (!m_CurrentHeadPin->CheckTypeMatch(PortCompo->GetPortType()))
	{
		FailConnection(PortCompo, &SweepResult, EFailConnectionReason::PinTypeNotMatch);
		return;
	}

	SuccessConnection(PortCompo);
}

void AUSB_PlayerPawn::ConnectChargingStart()
{

	PRINTF("Charging Start");

	m_UsbMovement->m_bUseSweep = false;

	FVector For = m_CurrentHead->GetForwardVector();

	//딱붙어있을때 스윕 필요
	FHitResult Hit;
	FVector TraceStart = m_CurrentHeadPin->GetComponentLocation();
	FVector TraceEnd = TraceStart+For * 3.f;
	
	FCollisionQueryParams Param;

	AddIgnoreActorsToQuery(Param);

	TArray<TEnumAsByte<EObjectTypeQuery> >  ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery8);




	//sweep이니 잘못됨
	if (UKismetSystemLibrary::BoxTraceSingleForObjects(GetWorld(),
		TraceStart, TraceEnd, m_UsbMovement->m_Shape.GetBox(),
		m_CurrentHead->GetComponentRotation(), ObjectTypes, true, m_AryTraceIgnoreActors,
		EDrawDebugTrace::ForOneFrame, Hit, true, FLinearColor::Green, FLinearColor::Red, 1.f) && Hit.GetActor())
	{
		PRINTF("Forward Overlap Called");
		TryConnect(m_CurrentHead, Hit.GetActor(), Hit.GetComponent(), 0, false, Hit);
		return;
	}

	m_CurrentHead->OnComponentBeginOverlap.AddDynamic(this, &AUSB_PlayerPawn::TryConnect);

	if (!m_UsbMovement->IsMovingOnGround())//sky connect
	{
		PRINTF("AirCharging Start");
		m_UsbMovement->RequestAirConnectChargeMove(m_CurrentFocusedPort->GetComponentRotation(), For, 3.f,2.f);
		return;
	}
	m_UsbMovement->RequestConnectChargeMove(For, 1.7f,2.f);
}

void AUSB_PlayerPawn::SuccessConnection(UPortSkMeshComponent* portConnect)
{
	m_CurrentHead->OnComponentBeginOverlap.RemoveDynamic(this, &AUSB_PlayerPawn::TryConnect);
	//PRINTF("SuccessConnection");
	//m_OnConnectedBP.Broadcast(m_CurrentHead->GetSocketLocation("PinPoint"));

	//m_CurrentHead->SetGenerateOverlapEvents(false);
	//EnableUSBInput();
	m_UsbMovement->StopUSBMove();
	m_UsbMovement->m_bUseSweep = true;

	portConnect->DisableColl();

	auto* ReadyAction = UCActionFactory::MakeMoveComponentToFollow(m_CurrentHeadPin, portConnect, 0.3f, "ReadyPoint", true, ETimingFunction::EaseInCube);
	ReadyAction->m_OnComplete.AddLambda(
		[=]()
		{
		for (auto* pp : GetPhysicsAry())
		{
			pp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		}

			AdjustPinTransform(portConnect);
		});

	m_ReadyActionManager->RunAction(ReadyAction);
	
	//m_CurrentHeadPin->Connect(portConnect);
	//portConnect->Connect(m_CurrentHeadPin);

	//if (!portConnect->m_bCantMoveOnConnected)
	//{
	//	SetHeadTail(portConnect->GetParentSkMesh(), m_CurrentTail, portConnect, m_PortTailPrev);
	//	AddTraceIgnoreActor(portConnect->GetOwner());
	//	AddPhysicsBody(portConnect->GetParentSkMesh());
	//}
	//else
	//{
	//	//cantMove;
	//	m_PortHeadPrev = portConnect;

	//	DisableUSBMove();
	//}
}

void AUSB_PlayerPawn::AdjustPinTransform(UPortSkMeshComponent * portConnect)
{
	//레디포인트가 핀에 있으면 각도 오차만큼 이동 거리에 오차가 생긴다, 대각선으로
	//레디포인트를 포트가 갖게해야한다.
	
	auto* RotateAction = UCActionFactory::MakeRotateComponentToFollowAction(m_CurrentHeadPin, portConnect, 0.2f);
	auto* MoveAction = UCActionFactory::MakeMoveComponentToFollow(m_CurrentHeadPin, portConnect, 0.8f, "PinPoint", false , ETimingFunction::EaseInCube);
	//값이 바뀌니까
	MoveAction->m_OnComplete.AddLambda(
	[=]()
	{
		PRINTF("SuccessConnection");
		m_OnConnectedBP.Broadcast(m_CurrentHead->GetSocketLocation("PinPoint"));
		m_CurrentHead->SetGenerateOverlapEvents(false);
		EnableUSBInput();
		//
		 FVector PortPoint = portConnect->GetComponentLocation();
		 FVector ConnectPoint = PortPoint + (m_CurrentHeadPin->GetComponentLocation() - m_CurrentHeadPin->GetSocketLocation("PinPoint"));
		 FRotator PortRot = portConnect->GetParentSkMesh()->GetComponentRotation();
		 m_CurrentHeadPin->SetWorldLocationAndRotationNoPhysics(ConnectPoint, PortRot);
		//
		m_CurrentHeadPin->Connect(portConnect);
		portConnect->Connect(m_CurrentHeadPin);
		//
		if (!portConnect->m_bCantMoveOnConnected)
		{
			SetHeadTail(portConnect->GetParentSkMesh(), m_CurrentTail, portConnect, m_PortTailPrev);
			AddTraceIgnoreActor(portConnect->GetOwner());
			AddPhysicsBody(portConnect->GetParentSkMesh());
		}
		else
		{
			//cantMove;
			m_PortHeadPrev = portConnect;
			DisableUSBMove();
		}

		for (auto* pp : GetPhysicsAry())
		{
			pp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		}

	}
	);
	
	m_ConnectActionManager->RunAction(RotateAction);
	m_ConnectActionManager->RunAction(MoveAction);
}

void AUSB_PlayerPawn::FailConnection(UPortSkMeshComponent* portConnect, const FHitResult * hitResult, EFailConnectionReason reason)
{
	switch (reason)
	{
	case EFailConnectionReason::PinTypeNotMatch:
		PRINTF("FailConnect - PinType");
		portConnect->FailConnection(*hitResult);
		m_CurrentHeadPin->FailConnection(*hitResult);
		break;
	case EFailConnectionReason::RotationNotMatch:
		PRINTF("FailConnect - Rotation");
		portConnect->FailConnection(*hitResult);
		m_CurrentHeadPin->FailConnection(*hitResult);
		break;
	case EFailConnectionReason::PortNotFoundCast:
		PRINTF("FailConnect - Port Is Different");
		return;
		m_CurrentHeadPin->FailConnection(*hitResult);
		break;
	case EFailConnectionReason::PortNotFoundTimeEnd:
		PRINTF("FailConnect - TimeEnd");
		break;
	}
	PRINTF("FailConnection");
	m_CurrentHead->OnComponentBeginOverlap.RemoveDynamic(this, &AUSB_PlayerPawn::TryConnect);
	EnableUSBInput();
	m_UsbMovement->m_bUseSweep = true;
	m_UsbMovement->StopUSBMove();
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

	SetHeadTail(m_CurrentTail, m_CurrentHead, m_PortTailPrev, m_PortHeadPrev);
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
	if (!m_PortTailPrev)//usb를 제외하곤 결국 달려있는것 모두 Port들이다
	{
		return false;
	}

	float EjectPowerFromPort = m_PortTailPrev->m_fEjectPowerToPin;

	if (RemoveTraceIgnoreActor(m_PortTailPrev->GetOwner()))
	{
		RemovePhysicsBody(m_CurrentTail);//여기가 안나와준다면,해당 포트는 움직일수 없던 포트
	}
	
	auto* TailPrev = m_PortTailPrev->GetPinConnected();

	m_OnDisconnectedBP.Broadcast(TailPrev->GetSocketLocation("PinPoint"));

	TailPrev->SetGenerateOverlapEvents(true);

	m_PortTailPrev->Disconnect();
	SetHeadTail(m_CurrentHead, TailPrev, m_PortHeadPrev, TailPrev->GetMyPort(), true);

	EnableUSBMove();

	DisableUSBInput(m_fBlockMoveTimeWhenEject);//이부분을 바꿔서 임펄스에 비례해 오래 못건드리게
	FVector ImpulseDir = GetTail()->GetForwardVector()*-1.f * EjectPowerFromPort *GetTotalMass();
	m_UsbMovement->AddImpulse(ImpulseDir);
	
	m_PlayerCon->PlayerCameraManager->PlayCameraShake(UEjectionCamShake::StaticClass(), 1.0f);
	return true;
}

void AUSB_PlayerPawn::AddTraceIgnoreActor(AActor * actorWant)
{
	m_AryTraceIgnoreActors.Emplace(actorWant);
	m_UsbMovement->AddIgnoreTraceActor(actorWant);
}

bool AUSB_PlayerPawn::RemoveTraceIgnoreActor(AActor * actorWant)
{
	m_UsbMovement->RemoveIgnoreTraceActor(actorWant);
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
		PRINTF("Disconnect Success");
	}
}

void AUSB_PlayerPawn::Jump()
{
	if (!m_bBlockJump)
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

	if (!m_CurrentHeadPin)
	{//꽂힌 머리가 핀기능이 없음
		return;
	}

	if (m_CurrentFocusedPort)
	{
		m_CurrentFocusedPort->OnFocusEnd(m_CurrentHeadPin);
	}


	if (GetWorld()->SweepSingleByChannel(HitResult, StartTrace, EndTrace, FQuat::Identity, ECC_GameTraceChannel9, FCollisionShape::MakeSphere(10.f), QueryParams))
	{
		if (!HitResult.GetActor())
		{
			return;
		}

		UPortSkMeshComponent* PortableCompo = Cast<UPortSkMeshComponent>(HitResult.GetComponent());

		if (PortableCompo && !PortableCompo->GetPinConnected())// PortableCompo->CheckYawOnly(m_CurrentHeadPin)
		{
			m_CurrentFocusedPort = PortableCompo;
			m_CurrentFocusedPort->OnFocus(m_CurrentHeadPin, IsMovingOnGround());
			return;
		}
	}

	m_CurrentFocusedPort = nullptr;
}

bool AUSB_PlayerPawn::IsImpulseVelocityLower()//임펄스중 속력이 인풋보다 작아지면 움직일수 있게됨
{
	float CurrentSize= m_CurrentHead->GetPhysicsLinearVelocity().SizeSquared();

	return CurrentSize < m_fMaxSpeedSqr * m_UsbMovement->m_fAirControl;
}


