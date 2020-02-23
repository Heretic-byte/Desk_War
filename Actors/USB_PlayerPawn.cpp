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
#include "Components/PortSkMeshComponent.h"
#include "Actors/PortPawn.h"
#include "GameFramework/PlayerController.h"




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
	m_fSpineAngularDamping = 1.f;
	m_fSpineLinearDamping = 0.01f;
	m_fCollMass = 1.f;
	m_fMaxAngularVelocity = 150.f;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	m_CurrentHead = m_PinUSB;
	m_CurrentTail = m_Pin5Pin;
	m_fPortTraceRange = 77.f;
}

void AUSB_PlayerPawn::CreatePhysicMovement()
{
	m_Movement = CreateDefaultSubobject<UPhysicsMovement>(TEXT("Movement00"));

	m_Movement->SetUpdatedComponent(m_CurrentHead);
	m_Movement->m_MovingTargetTail = m_CurrentTail;

	m_Movement->SetDamping(0.01f, 1.f);

	m_Movement->m_fMovingForce=38000.f;
	m_Movement->m_fGroundCastBoxSize = 10.f;
	m_Movement->m_fForwardCastOffset = 55.f;
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

void AUSB_PlayerPawn::SetHeadTail(USkeletalMeshComponent * headWant, USkeletalMeshComponent * tailWant)
{
	m_CurrentHead = headWant;
	m_CurrentTail = tailWant;

	m_Movement->SetUpdatedComponent(m_CurrentHead);
	m_Movement->m_MovingTargetTail = m_CurrentTail;
	
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
	SetHeadTail(m_CurrentTail,m_CurrentHead);
}

bool AUSB_PlayerPawn::TryConnect()
{
	if (!m_CurrentFocusedPort)
	{
		return false;
	}

	auto* Head = Cast<UPinSkMeshComponent>(GetHead());

	if (!Head)
	{
		return false;
	}

	if (Head->GetPortConnected())
	{
		return false;
	}
	
	BlockInput(true);

	bool Result= Head->Connect(m_CurrentFocusedPort);

	if (Result)
	{
		AddTraceIgnoreActor(m_CurrentFocusedPort->GetOwner());
		SetHeadTail(m_CurrentFocusedPort->GetParentSkMesh(), m_CurrentTail);
	}

	BlockInput(false);

	return Result;
}

bool AUSB_PlayerPawn::TryDisconnect()
{
	auto* Tail = Cast<UPinSkMeshComponent>(GetTail());

	if (!Tail)
	{
		return false;
	}

	if (!Tail->GetPortConnected())
	{
		return false;
	}
	PRINTF("CompletelyDisconnected");
	return Tail->Disconnect();
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

UPrimitiveComponent * AUSB_PlayerPawn::GetHead()
{
	return _inline_GetHead();
}

UPrimitiveComponent * AUSB_PlayerPawn::GetTail()
{
	return _inline_GetTail();
}

void AUSB_PlayerPawn::ConnectShot()
{
	TryConnect();
}

void AUSB_PlayerPawn::DisconnectShot()
{
	TryDisconnect();
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
	FVector EndTrace = (GetController()->GetRootComponent()->GetForwardVector() * m_fPortTraceRange) + StartTrace;

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

			return;
		}
	}

	m_CurrentFocusedPort = nullptr;
}


