


#include "USB_RenewalPawn.h"

// Fill out your copyright notice in the Description page of Project Settings.


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
AUSB_RenewalPawn::AUSB_RenewalPawn(const FObjectInitializer& objInit) :Super(objInit)
{
	m_fMaxConnectRotTime = 3.f;
	m_fMinConnectRotTime = 1.f;
	m_fDefaultFailImpulsePower = 1000.f;
	InitPlayerPawn();
	CreatePhysicMovement();
	CreateCameraFamily();

	m_ActionManager = CreateDefaultSubobject<UActionManagerComponent>("ActionManager00");

	m_AryPhysicsBody.Reserve(10);

	m_bBlockChargeClick = false;
	m_bBlockHeadChange = false;
	m_bBlockJump = false;

	m_PinUSB->SetMeshRadiusMultiple(0.6f);
	m_Pin5Pin->SetMeshRadiusMultiple(0.3f);
}

void AUSB_RenewalPawn::BeginPlay()
{
	Super::BeginPlay();
	m_PlayerCon = Cast<APlayerController>(GetController());
	SetHeadTail(m_CurrentHead, m_CurrentTail);
	Cast<UPinSkMeshComponent>(m_CurrentHead)->m_fFailImpulsePower = m_fDefaultFailImpulsePower;
	Cast<UPinSkMeshComponent>(m_CurrentTail)->m_fFailImpulsePower = m_fDefaultFailImpulsePower;
}

void AUSB_RenewalPawn::ZoomIn()
{
	m_MainSpringArm->ZoomIn();
}

void AUSB_RenewalPawn::ZoomOut()
{
	m_MainSpringArm->ZoomOut();
}

void AUSB_RenewalPawn::InitPlayerPawn()
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
	m_BaseHeadPin = Cast<UPinSkMeshComponent>(m_CurrentHead);
	m_BaseTailPin = Cast<UPinSkMeshComponent>(m_CurrentTail);
	m_fPortTraceRange = 77.f;
	m_fHeadChangeCD = 0.5f;
	m_fHeadChangeCDTimer = 0.f;

	m_fConnectReadyDuration = 5.f;
	m_fConnectPushDuration = 0.3f;
}

void AUSB_RenewalPawn::CreatePhysicMovement()
{
	m_UsbMovement = CreateDefaultSubobject<UUSBMovementComponent>(TEXT("USBMovement00"));

	m_UsbMovement->SetUpdatedComponent(m_CurrentHead);

}

void AUSB_RenewalPawn::CreateCameraFamily()
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

void AUSB_RenewalPawn::SetHeadTail(UPhysicsSkMeshComponent * headWant, UPhysicsSkMeshComponent * tailWant)
{
	m_CurrentHead->OnComponentBeginOverlap.RemoveAll(this);//remove from older

	m_CurrentHead = headWant;
	m_CurrentTail = tailWant;

	m_UsbMovement->SetUpdatedComponent(m_CurrentHead);

	m_CamRoot->AttachToComponent(m_CurrentHead, FAttachmentTransformRules::KeepRelativeTransform);

	m_CurrentHeadPin = Cast<UPinSkMeshComponent>(m_CurrentHead);
}

void AUSB_RenewalPawn::SetupPlayerInputComponent(UInputComponent * PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AUSB_RenewalPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUSB_RenewalPawn::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AUSB_RenewalPawn::RotateYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &AUSB_RenewalPawn::RotatePitch);
	//Connect
	PlayerInputComponent->BindAction(FName("Jump"), EInputEvent::IE_Pressed, this, &AUSB_RenewalPawn::Jump);
	PlayerInputComponent->BindAction(FName("Jump"), EInputEvent::IE_Released, this, &AUSB_RenewalPawn::StopJumping);
	PlayerInputComponent->BindAction(FName("HeadChange"), EInputEvent::IE_Pressed, this, &AUSB_RenewalPawn::ChangeHeadTail);
	PlayerInputComponent->BindAction(FName("ZoomIn"), EInputEvent::IE_Pressed, this, &AUSB_RenewalPawn::ZoomIn);
	PlayerInputComponent->BindAction(FName("ZoomOut"), EInputEvent::IE_Pressed, this, &AUSB_RenewalPawn::ZoomOut);
}

void AUSB_RenewalPawn::Tick(float DeltaTime)
{
	m_fHeadChangeCDTimer += DeltaTime;
	Super::Tick(DeltaTime);

	if (!m_CurrentFocusedPort)
	{
		return;
	}

	FVector HeadPos = GetHead()->GetComponentLocation();
	FVector PortPst = m_CurrentFocusedPort->GetComponentLocation();
	DrawDebugLine(GetWorld(), HeadPos, PortPst, FColor::Cyan, false, -1, 0.1f);
}

void AUSB_RenewalPawn::MoveForward(float v)
{
	if ((Controller != NULL) && (v != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, v);
	}
}

void AUSB_RenewalPawn::MoveRight(float v)
{
	if ((Controller != NULL) && (v != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, v);
	}
}

void AUSB_RenewalPawn::RotatePitch(float v)
{
	AddControllerPitchInput(v);
}
void AUSB_RenewalPawn::RotateYaw(float v)
{
	AddControllerYawInput(v);
}


void AUSB_RenewalPawn::ChangeHeadTail()
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

	SetHeadTail(m_CurrentTail, m_CurrentHead);
}

void AUSB_RenewalPawn::SetPhysicsVelocityAllBody(FVector linearV)
{
	for (auto* Phy : m_AryPhysicsBody)
	{
		Phy->SetPhysicsLinearVelocity(linearV);
		Phy->SetPhysicsAngularVelocityInDegrees(linearV);
	}
}

UPhysicsSkMeshComponent * AUSB_RenewalPawn::GetHead()
{
	return _inline_GetHead();
}

UPhysicsSkMeshComponent * AUSB_RenewalPawn::GetTail()
{
	return _inline_GetTail();
}

void AUSB_RenewalPawn::Jump()
{
	if (!m_bBlockJump)
		m_UsbMovement->Jump();
}

void AUSB_RenewalPawn::StopJumping()
{
	m_UsbMovement->StopJumping();
}

