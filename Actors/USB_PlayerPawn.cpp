// Fill out your copyright notice in the Description page of Project Settings.


#include "USB_PlayerPawn.h"
#include "ConstructorHelpers.h"
#include "Datas/USB_Macros.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Animation/AnimBlueprint.h"


AUSB_PlayerPawn::AUSB_PlayerPawn(const FObjectInitializer& objInit):Super(objInit)
{
	InitPlayerPawn();
	CreatePhysicMovement();
	CreateCameraFamily();
	CreateSkFaceMesh();
	m_CurrentHead = m_PinUSB;
	m_CurrentTail = m_Pin5Pin;
	//SetHeadTail(m_PinUSB, m_Pin5Pin);
}


void AUSB_PlayerPawn::InitPlayerPawn()
{
	m_fSpineAngularDamping = 20.f;
	m_fSpineLinearDamping = 10.f;
	m_fCollMass = 0.1f;
	m_fMaxAngularVelocity = 1000.f;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AUSB_PlayerPawn::CreatePhysicMovement()
{
	m_Movement = CreateDefaultSubobject<UPhysicsMovement>(TEXT("Movement00"));

	m_Movement->SetMovingForce(7000.f);
	m_Movement->SetAngularDamping(12.f);
	m_Movement->SetLinearDamping(8.f);
}

void AUSB_PlayerPawn::CreateCameraFamily()
{
	m_CamRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CamRoot"));
	m_CamRoot->SetupAttachment(m_PinUSB);

	m_MainSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring00"));
	m_MainSpringArm->SetupAttachment(m_CamRoot);
	m_MainSpringArm->TargetArmLength = 600.f;
	m_MainSpringArm->SocketOffset = FVector(50.f, 0, 0);
	m_MainSpringArm->TargetOffset = FVector(0.f, 0.f, 400.f);
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
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundSkMesh(TEXT("SkeletalMesh'/Game/01_FinalUSB/Mesh/Head/NewFace/Face_idle.Face_idle'"));

	if (FoundSkMesh.Succeeded())
	{
		m_MeshFaceSk->SetSkeletalMesh(FoundSkMesh.Object);
	}

	m_MeshFaceSk->SetRelativeLocation(FVector(-0.165f,0,0.825f));
	m_MeshFaceSk->SetRelativeScale3D(FVector(0.88f, 0.88f, 0.88f ));
}

void AUSB_PlayerPawn::SetHeadTail(UPinSkMeshComponent * headWant, UPinSkMeshComponent * tailWant)
{
	m_CurrentHead = headWant;
	m_CurrentTail = tailWant;

	m_Movement->SetVelocityBone(m_CurrentHead->GetBoneVelo());
	m_Movement->SetUpdatedComponent(m_CurrentHead);

	m_CamRoot->AttachToComponent(m_CurrentHead,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void AUSB_PlayerPawn::SetupPlayerInputComponent(UInputComponent * PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AUSB_PlayerPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUSB_PlayerPawn::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AUSB_PlayerPawn::RotateYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &AUSB_PlayerPawn::RotatePitch);

}
void AUSB_PlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	SetHeadTail(m_PinUSB,m_Pin5Pin);
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

UPinSkMeshComponent * AUSB_PlayerPawn::GetHead()
{
	return _inline_GetHead();
}
