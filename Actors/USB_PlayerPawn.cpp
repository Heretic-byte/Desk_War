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


	m_bBlockHeadChange = false;
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
	m_Movement->SetInitHeadMass(m_CurrentHead->GetBodyInstance()->GetBodyMass());
	m_Movement->m_OnAutoMoveEnd.AddUObject(this, &AUSB_PlayerPawn::ConnectChargingEnd);
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
	m_fPortTraceRange = 77.f;
	m_fHeadChangeCD = 0.5f;
	m_fHeadChangeCDTimer = 0.f;
	
	m_fConnectReadyDuration = 5.f;
	m_fConnectPushDuration = 0.3f;
}

void AUSB_PlayerPawn::CreatePhysicMovement()
{
	m_Movement = CreateDefaultSubobject<UPhysicsMovement>(TEXT("Movement00"));

	m_Movement->SetUpdatePhysicsMovement(m_CurrentHead, m_CurrentTail);

	m_Movement->m_fMovingForce=38000.f;
	m_Movement->m_fGroundCastBoxSize = 10.f;
	m_Movement->m_fGroundCastOffset = -20.f;
	m_Movement->m_WalkableSlopeAngle = 55.f;
	m_Movement->m_fJumpZVelocity = 2000.f;
	m_Movement->m_nJumpMaxCount = 2;
	m_Movement->m_fAirControl = 0.6f;

	m_Movement->m_bDebugShowForwardCast = false;
	
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

	m_Movement->SetUpdatePhysicsMovement(m_CurrentHead, m_CurrentTail);
	
	m_CamRoot->AttachToComponent(m_CurrentHead,FAttachmentTransformRules::KeepRelativeTransform);

	m_CurrentHead->OnComponentBeginOverlap.AddDynamic(this,&AUSB_PlayerPawn::TryConnect);
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
	m_Movement->SetTraceIgnoreActorAry(&m_AryTraceIgnoreActors);
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

	if (m_CurrentTryConnectingPin)
	{
		PRINTF("Pin Here");
	}
	else
	{
		PRINTF("Pin No");
	}
	if (m_CurrentTryConnectingPort)
	{
		PRINTF("Port Here");
	}
	else
	{
		PRINTF("Port No");
	}
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
	if (m_bBlockHeadChange)
	{
		return;
	}

	if (!m_CurrentFocusedPort)
	{
		return;
	}

	if (m_CurrentFocusedPort->GetPinConnected())
	{
		return;
	}


	auto* PinHead = Cast<UPinSkMeshComponent>(GetHead());

	if (!PinHead)
	{
		return;
	}

	if (PinHead->GetPortConnected())
	{
		return;
	}

	m_bBlockHeadChange = true;
	m_CurrentTryConnectingPin = PinHead;
	if (m_CurrentTryConnectingPin)
	{
		PRINTF("Pin Here");
	}
	m_CurrentTryConnectingPort = m_CurrentFocusedPort;
	if (m_CurrentTryConnectingPort)
	{
		PRINTF("Port Here");
	}

	if (!m_Movement->IsGround())
	{ 
		m_Movement->m_OnAutoRotateEnd.RemoveAll(this);

		float Distance = FVector::Dist(PinHead->GetComponentLocation(), m_CurrentFocusedPort->GetComponentLocation());
		PRINTF("Distance is : %f", Distance);
		//77 일때 3
		float Rate = Distance / m_fPortTraceRange;
		Rate *= m_fMaxConnectRotTime;

		if (Rate < m_fMinConnectRotTime)
		{
			Rate = m_fMinConnectRotTime;
		}
		PRINTF("RateTime IS : %f", Rate);
		auto* Pin = m_CurrentTryConnectingPin;
		auto* Port = m_CurrentTryConnectingPort;

		EnableAutoRotate(m_CurrentFocusedPort->GetComponentRotation(), Rate);
		m_Movement->m_OnAutoRotateEnd.AddLambda([=]
		{//실패했는데 얘는 비동기로 계속 불려버리네
			
			Pin->SetGenerateOverlapEvents(true);
			
			Port->DisblePhysicsCollision();
			EnableAutoMove(Port->GetForwardVector(), 3.f);
		});
		return;
	}
	m_CurrentTryConnectingPin->SetGenerateOverlapEvents(true);
	m_CurrentTryConnectingPort->DisblePhysicsCollision();
	EnableAutoMove(m_CurrentTryConnectingPin->GetForwardVector(), 3.f);
	
}


void AUSB_PlayerPawn::TryConnect(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	PRINTF("TryConnect - Called");

	if (m_CurrentTryConnectingPort != OtherComp)//이따보고 위에 생략 시키자
	{
		PRINTF("FailConnect - Port Is Different");
		FailConnection(SweepResult);
		return;
	}

	if (!m_CurrentTryConnectingPort->CheckConnectTransform(m_CurrentTryConnectingPin))
	{
		PRINTF("FailConnect - Rotation");
		FailConnection(SweepResult);
		return;
	}

	if (!m_CurrentTryConnectingPin->Connect(m_CurrentTryConnectingPort))
	{
		PRINTF("FailConnect - PinType");
		FailConnection(SweepResult);
		return;
	}

	SuccessConnection();
}
//움직임 끊기 존재해야함
void AUSB_PlayerPawn::SuccessConnection()
{
	PRINTF("SuccessConnection");
	m_bBlockHeadChange = false;
	DisableAutoMove();
	ConnectChargingEnd();
	FVector PortPoint = m_CurrentTryConnectingPort->GetSocketLocation("PortPoint");
	FVector ConnectPoint = PortPoint + (m_CurrentTryConnectingPin->GetComponentLocation() - m_CurrentTryConnectingPin->GetSocketLocation("PinPoint"));
	FRotator PortRot = m_CurrentTryConnectingPort->GetComponentRotation();
	m_CurrentTryConnectingPin->SetWorldLocationAndRotationNoPhysics(ConnectPoint, PortRot);

	m_CurrentTryConnectingPort->Connect(m_CurrentTryConnectingPin);
	AddTraceIgnoreActor(m_CurrentTryConnectingPort->GetOwner());
	SetHeadTail(m_CurrentTryConnectingPort->GetParentSkMesh(), m_CurrentTail);
	AddPhysicsBody(m_CurrentTryConnectingPort->GetParentSkMesh());
	m_CurrentTryConnectingPort = nullptr;
	m_CurrentTryConnectingPin = nullptr;
}

void AUSB_PlayerPawn::FailConnection(const FHitResult & hitResult)
{
	PRINTF("FailConnection");
	m_bBlockHeadChange = false;
	DisableAutoMove();
	ConnectChargingEnd();
	m_CurrentTryConnectingPort->FailConnection(hitResult);
	m_CurrentTryConnectingPin->FailConnection(hitResult);
	m_CurrentTryConnectingPort = nullptr;
	m_CurrentTryConnectingPin = nullptr;
}

void AUSB_PlayerPawn::ConnectChargingEnd()//사실상 실패랑 똑같은데 팅기는것만 없어야함
{
	m_bBlockHeadChange = false;
	m_CurrentTryConnectingPin->SetGenerateOverlapEvents(false);
	m_CurrentTryConnectingPort->EnablePhysicsCollision();
}

void AUSB_PlayerPawn::EnableInputMove()
{
	m_Movement->EnableInputMove();
}

void AUSB_PlayerPawn::DisableInputMove(float timer)
{
	m_Movement->DisableInputMove(timer);
}

void AUSB_PlayerPawn::EnableAutoMove(FVector wantDir, float timer)
{
	m_Movement->EnableAutoMove(wantDir, timer);
}

void AUSB_PlayerPawn::DisableAutoMove()
{
	m_Movement->DisableAutoMove();
}

void AUSB_PlayerPawn::EnableAutoRotate(FRotator wantRot, float timer)
{
	m_Movement->EnableAutoRotate(wantRot, timer);
}

void AUSB_PlayerPawn::DisableAutoRotate()
{
	m_Movement->DisableAutoRotate();
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

	/*if (!Tail)
	{
		Tail = m_CurrentTailPin;
	}*/

	if (!Tail->GetPortConnected())
	{
		return false;
	}

	RemoveTraceIgnoreActor(Tail->GetPortConnected()->GetOwner());
	RemovePhysicsBody(Tail->GetPortConnected());
	Tail->Disconnect();
	SetHeadTail(m_CurrentHead, Tail);
	EnableInputMove();
	return true;
}

void AUSB_PlayerPawn::BlockInput(bool tIsBlock)
{
	if (!tIsBlock)
	{
		EnableInput(m_PlayerCon);
	}
	else
	{
		DisableInput(m_PlayerCon);
	}
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
		FVector ImpulseDir = GetTail()->GetForwardVector() * m_fEjectionPower;
		m_Movement->AddImpulse(ImpulseDir);
		DisableInputMove(m_fBlockMoveTimeWhenEject);
		m_PlayerCon->PlayerCameraManager->PlayCameraShake(UEjectionCamShake::StaticClass(),1.0f);
	}
}

void AUSB_PlayerPawn::Jump()
{
	m_Movement->Jump();
}

void AUSB_PlayerPawn::StopJumping()
{
	m_Movement->StopJumping();
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

		if (PortableCompo)
		{
			m_CurrentFocusedPort = PortableCompo;
			return;
		}
	}
	m_CurrentFocusedPort = nullptr;
}

