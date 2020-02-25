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
	InitPlayerPawn();
	CreatePhysicMovement();
	CreateCameraFamily();
	CreateSkFaceMesh();

	m_ActionManager = CreateDefaultSubobject<UActionManagerComponent>("ActionManager00");
}

void AUSB_PlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	m_PlayerCon = Cast<APlayerController>(GetController());
	SetHeadTail(m_CurrentHead, m_CurrentTail);
	InitTraceIgnoreAry();
}

void AUSB_PlayerPawn::InitPlayerPawn()
{
	m_bCanConnectDist = false;
	m_fConnectHorizontalAngle = 0.06f;
	m_fEjectionPower = 4200.f;
	m_fSpineAngularDamping = 1.f;
	m_fSpineLinearDamping = 0.01f;
	m_fCollMass = 1.f;
	m_fMaxAngularVelocity = 150.f;
	m_fBlockMoveTimeWhenEject = 1.5f;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	m_NamePinConnectSocket = "ConnectPoint";
	m_NamePinConnectStartSocket = "ConnectStart";
	m_NamePinConnectPushPointSocket = "PushPoint";

	m_CurrentHead = m_PinUSB;
	m_CurrentTail = m_Pin5Pin;
	m_CurrentHeadPin = m_PinUSB;
	m_CurrentTailPin = m_Pin5Pin;
	m_fPortTraceRange = 77.f;
	m_fHeadChangeCD = 0.5f;
	m_fHeadChangeCDTimer = 0.f;
}

void AUSB_PlayerPawn::CreatePhysicMovement()
{
	m_Movement = CreateDefaultSubobject<UPhysicsMovement>(TEXT("Movement00"));

	m_Movement->SetUpdatePhysicsMovement(m_CurrentHead, m_CurrentTail);

	m_Movement->SetDamping(0.01f, 1.f);

	m_Movement->m_fMovingForce=38000.f;
	m_Movement->m_fGroundCastBoxSize = 10.f;
	m_Movement->m_fGroundCastOffset = -20.f;
	m_Movement->m_WalkableSlopeAngle = 55.f;
	m_Movement->m_fJumpZVelocity = 2000.f;
	m_Movement->m_nJumpMaxCount = 2;
	m_Movement->m_fAirControl = 0.6f;

	m_Movement->m_bDebugShowForwardCast = false;

	m_Movement->m_NameLinearVeloHeadBone = "PinPoint";
	m_Movement->m_NameLinearVeloTailBone = "PinPoint";
}

void AUSB_PlayerPawn::CreateCameraFamily()
{
	m_CamRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CamRoot"));
	m_CamRoot->SetupAttachment(m_PinUSB);
	m_CamOffset = FVector(3.5f, 0.f, 0.f);
	m_CamRoot->SetRelativeLocation(m_CamOffset);

	m_MainSpringArm = CreateDefaultSubobject<UUSB_SpringArm>(TEXT("Spring00"));
	m_MainSpringArm->SetupAttachment(m_CamRoot);
	m_MainSpringArm->TargetArmLength = 600.f;
	m_MainSpringArm->SocketOffset = FVector(50.f, 0, 0);
	m_MainSpringArm->TargetOffset = FVector(0.f, 0.f, 400.f);
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
	m_MainCam->RelativeRotation = FRotator(-30.f, 0.f, 0.f);
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

	m_MeshFaceSk->SetRelativeLocation(FVector(-0.165f,0, 0.93f));
	m_MeshFaceSk->SetRelativeScale3D(FVector(2.64f, 2.64f, 2.64f));
}

void AUSB_PlayerPawn::SetHeadTail(UPhysicsSkMeshComponent * headWant, UPhysicsSkMeshComponent * tailWant)
{
	m_CurrentHead = headWant;
	m_CurrentTail = tailWant;

	auto* HeadPin = m_CurrentHeadPin;
	m_CurrentHeadPin = m_CurrentTailPin;
	m_CurrentTailPin = HeadPin;

	m_Movement->SetUpdatePhysicsMovement(m_CurrentHead, m_CurrentTail);
	
	m_CamRoot->AttachToComponent(m_CurrentHead,FAttachmentTransformRules::KeepRelativeTransform);
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
}

void AUSB_PlayerPawn::InitTraceIgnoreAry()
{
	m_AryTraceIgnoreActors.Reserve(10);
	AddTraceIgnoreActor(this);
	m_Movement->SetTraceIgnoreActorAry(&m_AryTraceIgnoreActors);
}

void AUSB_PlayerPawn::Tick(float DeltaTime)
{
	m_fHeadChangeCDTimer += DeltaTime;
	Super::Tick(DeltaTime);
	TickTracePortable();

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

void AUSB_PlayerPawn::ChangeHeadTail()
{
	if (m_fHeadChangeCDTimer < m_fHeadChangeCD)
	{
		return;
	}
	m_fHeadChangeCDTimer = 0.f;
	SetHeadTail(m_CurrentTail,m_CurrentHead);
}

bool AUSB_PlayerPawn::CheckPortVerticalAngle(UPortSkMeshComponent * port)
{
	return UKismetMathLibrary::Abs(port->GetUpVector().Z - GetHead()->GetUpVector().Z) <= 0.1f;
}

bool AUSB_PlayerPawn::CheckPortHorizontalAngle(UPortSkMeshComponent * port)
{
	FVector PlayerConnectDir = GetHead()->GetForwardVector();
	FVector ConnectPortDir = port->GetForwardVector();
	float Dot = UKismetMathLibrary::Dot_VectorVector(PlayerConnectDir, ConnectPortDir);
	return UKismetMathLibrary::Acos(Dot) <= m_fConnectHorizontalAngle;
}

bool AUSB_PlayerPawn::TryConnect(UPortSkMeshComponent* portWant)
{
	auto* Head = Cast<UPinSkMeshComponent>(GetHead());

	if (!Head)
	{
		return false;
	}

	if (Head->GetPortConnected())
	{
		return false;
	}

	bool Result= Head->Connect(portWant);

	if (Result)
	{
		AddTraceIgnoreActor(portWant->GetOwner());
		SetHeadTail(portWant->GetParentSkMesh(), m_CurrentTail);

		if (portWant->GetBlockMoveOnConnnect())
		{
			BlockMovement();
		}
	}

	return Result;
}

void AUSB_PlayerPawn::BlockMovement()
{
	m_Movement->SetBlockMoveTimer(-1.f);
}

void AUSB_PlayerPawn::UnblockMovement()
{
	m_Movement->SetBlockMoveTimer(0.f);
}

bool AUSB_PlayerPawn::TryDisconnect()
{
	auto* Tail = Cast<UPinSkMeshComponent>(GetTail());

	if (!Tail)
	{
		Tail = m_CurrentTailPin;
	}

	if (!Tail->GetPortConnected())
	{
		return false;
	}

	RemoveTraceIgnoreActor(Tail->GetPortConnected()->GetOwner());
	Tail->Disconnect();
	SetHeadTail(m_CurrentHead, Tail);
	UnblockMovement();
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

void AUSB_PlayerPawn::ConnectShot()
{
	if (!m_CurrentFocusedPort)
	{
		return;
	}

	if (!CheckConnectTransform())
	{
		return;
	}

	BlockInput(true);

	GetHead()->SetPhysicsLinearVelocity(FVector(0,0,0));
	for (auto* Sphere : m_ArySpineColls)
	{
		Sphere->SetPhysicsLinearVelocity(FVector(0, 0, 0));
	}
	GetTail()->SetPhysicsLinearVelocity(FVector(0, 0, 0));

	auto* Port = m_CurrentFocusedPort;
	Port->DisablePhysics();

	m_ActionManager->RemoveAllActions();

	auto* Sequence = UCActionFactory::MakeSequenceAction();

	Sequence->AddAction(MoveForReadyConnect(m_CurrentFocusedPort));

	Sequence->AddAction(RotateForConnect(m_CurrentFocusedPort));

	auto* PushAction = MoveForPushConnection(m_CurrentFocusedPort);

	Sequence->AddAction(PushAction);

	
	PushAction->m_OnActionComplete.BindLambda(
		[=]()
		{
			TryConnect(Port);
			GetHead()->ResetAllBodiesSimulatePhysics();
			BlockInput(false);
		});

	Sequence->m_OnActionKilled.BindLambda(
		[=]()
	{
		Port->EnablePhysics();
		BlockInput(false);
	});

	m_ActionManager->RunAction(Sequence);
}

bool AUSB_PlayerPawn::CheckConnectTransform()
{
	if (!m_bCanConnectDist)
	{
		return false;
	}

	bool ResultH = CheckPortHorizontalAngle(m_CurrentFocusedPort);
	bool ResultV = CheckPortVerticalAngle(m_CurrentFocusedPort);

	if (!ResultH || !ResultV)
	{
		return false;
	}

	return true;
}

UCActionBaseInterface* AUSB_PlayerPawn::MoveForReadyConnect(UPortSkMeshComponent * portWant)
{
	FVector Dest = portWant->GetSocketLocation(m_NamePinConnectStartSocket);

	auto* MoveAction = UCActionFactory::MakeMoveComponentToAction(GetHead(), Dest, 0.5f, ETimingFunction::EaseInCube);

	return MoveAction;
}

UCActionBaseInterface* AUSB_PlayerPawn::RotateForConnect(UPortSkMeshComponent * portWant)
{
	FRotator ConnectRot = portWant->GetComponentRotation();

	auto* Action = UCActionFactory::MakeRotateComponentToAction(GetHead(), ConnectRot, 0.2f, ETimingFunction::Linear);

	return Action;
}

UCActionBaseInterface* AUSB_PlayerPawn::MoveForPushConnection(UPortSkMeshComponent * portWant)
{
	FVector Dest = portWant->GetSocketLocation(m_NamePinConnectPushPointSocket);

	auto* MoveAction = UCActionFactory::MakeMoveComponentToAction(GetHead(), Dest, 0.3f, ETimingFunction::EaseInCube);

	return MoveAction;
}

void AUSB_PlayerPawn::DisconnectShot()
{
	if (TryDisconnect())
	{
		FVector ImpulseDir = _inline_GetTailPin()->GetForwardVector() * 1.f * m_fEjectionPower;
		m_Movement->AddImpulse(ImpulseDir);
		m_Movement->SetBlockMoveTimer(m_fBlockMoveTimeWhenEject);
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
	FVector EndTrace = (GetHead()->GetForwardVector()* m_fPortTraceRange) + StartTrace;

	FCollisionQueryParams QueryParams;
	AddIgnoreActorsToQuery(QueryParams);

	DrawDebugLine(
		GetWorld(),
		StartTrace,
		EndTrace,
		FColor(255, 0, 0),
		false, -1, 0,
		6.333
	);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_GameTraceChannel9, QueryParams))
	{
		if (!HitResult.GetActor())
		{
			return;
		}

		UPortSkMeshComponent* PortableCompo = Cast<UPortSkMeshComponent>(HitResult.GetActor()->GetComponentByClass(UPortSkMeshComponent::StaticClass())); 

		if (PortableCompo)
		{
			m_CurrentFocusedPort = PortableCompo;
			m_bCanConnectDist=m_CurrentFocusedPort->SetAimTracePoint(HitResult.ImpactPoint);
			return;
		}
	}
	m_bCanConnectDist = false;
	//m_ActionManager->RemoveAllActions();
	m_CurrentFocusedPort = nullptr;
}


