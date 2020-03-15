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

	m_AryPhysicsBody.Reserve(10);



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
	m_Movement->m_Head = m_CurrentHead;
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

	m_CurrentHead = m_PinUSB;
	m_CurrentTail = m_Pin5Pin;
	m_CurrentHeadPin = m_PinUSB;
	m_CurrentTailPin = m_Pin5Pin;
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
		AddPhysicsBody(portWant->GetParentSkMesh());
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
	RemovePhysicsBody(Tail->GetPortConnected());
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
		m_Movement->SetBlockMoveTimer(0.f);
	}
	else
	{
		
		DisableInput(m_PlayerCon);
		m_Movement->SetBlockMoveTimer(-1.f);
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
	//되고 안되고 조건문은 여기있어야함
	//몇개는 핀이가져야 잘못된 삽입에 패널티를 줌


	//BlockInput(true);

	//SetPhysicsVelocityAllBody(FVector(0, 0, 0));

	auto* Port = m_CurrentFocusedPort;
	Port->DisblePhysicsCollision();

	m_ActionManager->RemoveAllActions();

	auto* HeadMovingSequence = UCActionFactory::MakeSequenceAction();

	auto* HeadMoveAction = HeadMoveForReadyConnect();

	HeadMovingSequence->AddAction(HeadMoveAction);

	auto* PushAction = HeadMoveForPushConnection();

	HeadMovingSequence->AddAction(PushAction);

	PushAction->m_OnActionComplete.BindLambda(
		[=]()
	{
		SetPhysicsVelocityAllBody(FVector(0, 0, 0));

		FRotator ConnectRot = Port->GetComponentRotation();

		GetHead()->SetWorldRotation(ConnectRot,false,nullptr,ETeleportType::TeleportPhysics);

		TryConnect(Port);

		//일단은 이거문제로 추정
		//Port->EnablePhysicsCollision();

		BlockInput(false);

		if (Port->GetBlockMoveOnConnnect())
		{
			BlockMovement();
		}
	});

	HeadMovingSequence->m_OnActionKilled.BindLambda(
		[=]()
	{
		//Port->EnablePhysicsCollision();
		BlockInput(false);
	});

	m_ActionManager->RunAction(HeadMovingSequence);
	m_ActionManager->RunAction(RotateForConnect());

}

bool AUSB_PlayerPawn::CheckConnectTransform()
{
	if (!m_bCanConnectDist)
	{
		return false;
	}

	FRotator PortRot = m_CurrentFocusedPort->GetComponentRotation();
	FRotator PinRot = GetHead()->GetComponentRotation();
	FRotator& ConnectRot= Cast<UPinSkMeshComponent>(GetHead())->GetConnectableRot();

	float PitchDiff = FMath::Abs(PortRot.Pitch-PinRot.Pitch);
	bool PitchCheck = PitchDiff <= ConnectRot.Pitch;

	float RollDiff = FMath::Abs(PortRot.Roll - PinRot.Roll);
	bool RollCheck = RollDiff <= ConnectRot.Roll;

	float YawDiff = FMath::Abs(PortRot.Yaw - PinRot.Yaw);
	bool YawCheck = YawDiff <= ConnectRot.Yaw;

	return PitchCheck&&RollCheck&&YawCheck;
}

UCActionBaseInterface* AUSB_PlayerPawn::HeadMoveForReadyConnect()
{
	auto* MoveAction = UCActionFactory::MakeFollowMoveComponentToAction(GetHead(), m_CurrentFocusedPort, m_fConnectReadyDuration,NAME_None, "ConnectStart", ETimingFunction::Linear,ETeleportType::TeleportPhysics);
	MoveAction->m_OnActionTick.BindLambda(
		[=](float delta)
	{
		

	});
	return MoveAction;
}

UCActionBaseInterface* AUSB_PlayerPawn::RotateForConnect()
{
	auto* Action = UCActionFactory::MakeFollowRotateComponentToAction(GetHead(), m_CurrentFocusedPort, m_fConnectReadyDuration, ETimingFunction::Linear,ETeleportType::TeleportPhysics);

	return Action;
}

UCActionBaseInterface* AUSB_PlayerPawn::HeadMoveForPushConnection()
{
	auto* MoveAction = UCActionFactory::MakeFollowMoveComponentToAction(GetHead(), m_CurrentFocusedPort, m_fConnectPushDuration,"PinPoint","PortPoint", ETimingFunction::EaseInCube, ETeleportType::TeleportPhysics);

	return MoveAction;
}

UCActionBaseInterface * AUSB_PlayerPawn::TailMoveForReadyConnect()
{
	FVector ForceWant = m_CurrentFocusedPort->GetComponentLocation() - GetTail()->GetComponentLocation();
	ForceWant.Normalize();

	auto* MoveAction = UCActionFactory::MakeAddForceMoveComponentToAction(GetTail(), ForceWant*3800.f, m_fConnectReadyDuration);
	return MoveAction;
}

UCActionBaseInterface * AUSB_PlayerPawn::TailMoveForPushConnection()
{
	auto* MoveAction = UCActionFactory::MakeFollowMoveComponentToAction(GetTail(), m_CurrentFocusedPort, m_fConnectReadyDuration, NAME_None, "ConnectStart", ETimingFunction::Linear, ETeleportType::TeleportPhysics);

	return MoveAction;
}

void AUSB_PlayerPawn::DisconnectShot()
{
	if (TryDisconnect())
	{
		FVector ImpulseDir = _inline_GetTailPin()->GetForwardVector() * m_fEjectionPower;
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
	FVector Forward = GetHead()->GetForwardVector();
	FVector EndTrace = (GetHead()->GetForwardVector()* m_fPortTraceRange) + StartTrace;

	FCollisionQueryParams QueryParams;
	AddIgnoreActorsToQuery(QueryParams);

	//DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor(255, 0, 120), false, -1.f, 0.1f);
	
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
			m_bCanConnectDist=m_CurrentFocusedPort->SetAimTracePoint(HitResult.ImpactPoint);
			return;
		}
	}
	m_bCanConnectDist = false;
	m_CurrentFocusedPort = nullptr;
}


