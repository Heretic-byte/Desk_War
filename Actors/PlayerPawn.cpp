


#include "PlayerPawn.h"
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

// Sets default values
APlayerPawn::APlayerPawn()
{
	m_SkMesh = CreateDefaultSubobject<UPhysicsSkMeshComponent>("SkMesh1");
	RootComponent = m_SkMesh;
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_UsbMovement = CreateDefaultSubobject<UPhysicsMovement>(TEXT("Movement00"));
	m_CamRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CamRoot"));
	m_CamRoot->SetupAttachment(m_SkMesh);
	m_CamRoot->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	//
	m_MainSpringArm = CreateDefaultSubobject<UUSB_SpringArm>(TEXT("Spring00"));
	m_MainSpringArm->SetupAttachment(m_CamRoot);
	m_MainSpringArm->TargetArmLength = 600.f;
	m_MainSpringArm->SocketOffset = FVector(0, 0, 0);
	m_MainSpringArm->WorldTargetOffset = FVector(0.f, 0.f, 0.f);
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

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	//m_UsbMovement->SetUpdatedComponent(m_SkMesh);
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerPawn::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APlayerPawn::RotateYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerPawn::RotatePitch);
	//Connect
	PlayerInputComponent->BindAction(FName("Jump"), EInputEvent::IE_Pressed, this, &APlayerPawn::Jump);
	PlayerInputComponent->BindAction(FName("Jump"), EInputEvent::IE_Released, this, &APlayerPawn::StopJumping);
	PlayerInputComponent->BindAction(FName("ZoomIn"), EInputEvent::IE_Pressed, this, &APlayerPawn::ZoomIn);
	PlayerInputComponent->BindAction(FName("ZoomOut"), EInputEvent::IE_Pressed, this, &APlayerPawn::ZoomOut);
}

void APlayerPawn::ZoomIn()
{
	m_MainSpringArm->ZoomIn();
}

void APlayerPawn::ZoomOut()
{
	m_MainSpringArm->ZoomOut();
}

void APlayerPawn::Jump()
{
	m_UsbMovement->Jump();
}

void APlayerPawn::StopJumping()
{
	m_UsbMovement->StopJumping();
}

void APlayerPawn::MoveForward(float v)
{
	if ((Controller != NULL) && (v != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, v);
	}
}

void APlayerPawn::MoveRight(float v)
{
	if ((Controller != NULL) && (v != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, v);
	}
}

void APlayerPawn::RotateYaw(float v)
{
	AddControllerYawInput(v);
}

void APlayerPawn::RotatePitch(float v)
{
	AddControllerPitchInput(v);
}
