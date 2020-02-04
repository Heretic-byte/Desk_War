// Fill out your copyright notice in the Description page of Project Settings.


#include "USB_Player_Pawn.h"

// Sets default values
AUSB_Player_Pawn::AUSB_Player_Pawn(const FObjectInitializer& obj)
	:Super(obj)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//

	m_fMovingForce = 4000.f;
	m_fOrientRotSpeed = 1.f;
	//
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root00"));




	m_MeshUsb = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkelMesh01"));
	m_MeshUsb->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundMeshPortUSB(TEXT("SkeletalMesh'/Game/01_FinalUSB/Mesh/Head/NewHead0120/USB_Head_Mesh_06.USB_Head_Mesh_06'"));
	check(FoundMeshPortUSB.Object);
	m_MeshUsb->SkeletalMesh = FoundMeshPortUSB.Object;
	m_MeshUsb->SetCollisionProfileName(FName(TEXT("USBMesh")));
	m_MeshUsb->RelativeScale3D = FVector(3.000000, 3.000000, 3.000000);
	m_MeshUsb->SetSimulatePhysics(true);


	m_SceneCamRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene02"));
	m_SceneCamRoot->SetupAttachment(m_MeshUsb);

	m_SpringMain = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring03"));
	m_SpringMain->SetupAttachment(m_SceneCamRoot);
	m_SpringMain->SocketOffset = FVector(50.f, 0, 0);
	m_SpringMain->TargetOffset = FVector(0.f, 0.f, 400.f);
	m_SpringMain->TargetArmLength = 600.f;
	m_SpringMain->bUsePawnControlRotation = true;
	m_SpringMain->bInheritRoll = false;
	m_SpringMain->bEnableCameraLag = true;
	m_SpringMain->bEnableCameraRotationLag = true;
	m_SpringMain->CameraLagSpeed = 10.f;
	m_SpringMain->CameraRotationLagSpeed = 10.f;
	m_SpringMain->m_fMinimumArmLength = 75.f;

	m_CamMain = CreateDefaultSubobject<UCameraComponent>(TEXT("Cam04"));
	m_CamMain->SetupAttachment(m_SpringMain);
	m_CamMain->FieldOfView = 100.f;
	m_CamMain->RelativeRotation = FRotator(-30.f, 0.f, 0.f);

	
	m_fLimitLinearVelocity = 500.f;
	m_fLimitAngularVelocity = 100.f;

	
	
}

// Called when the game starts or when spawned
void AUSB_Player_Pawn::BeginPlay()
{
	Super::BeginPlay();
	SetHead(m_MeshUsb);
	m_fMaxLinearSpeedSqr = FMath::Square<float>(m_fLimitLinearVelocity);
	m_MeshUsb->SetPhysicsMaxAngularVelocityInDegrees(m_fLimitAngularVelocity);
}

// Called every frame
void AUSB_Player_Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickForceMove(DeltaTime);
	TickLimitVelocity();
	TickHeadYawRotate(DeltaTime);
	TickHeadRollRotate(DeltaTime);
	TickHeadPitch(DeltaTime);
	//
}

// Called to bind functionality to input
void AUSB_Player_Pawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AUSB_Player_Pawn::ForceForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUSB_Player_Pawn::ForceRight);

	PlayerInputComponent->BindAxis("Turn", this, &AUSB_Player_Pawn::RotateYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &AUSB_Player_Pawn::RotatePitch);
}

UPrimitiveComponent * AUSB_Player_Pawn::GetHead() const
{
	return m_PrimHead;
}

void AUSB_Player_Pawn::SetHead(UPrimitiveComponent * headWantPhysics)
{
	m_PrimHead = headWantPhysics;
}

FVector AUSB_Player_Pawn::GetHeadVelocityDir()
{
	FVector Velo = GetHead()->GetPhysicsAngularVelocity();
	
	if (Velo.Normalize())
	{
		return Velo;
	}

	return FVector(0,0,0);
}


void AUSB_Player_Pawn::ForceForward(float v)
{
	m_fVertical = v;

}

void AUSB_Player_Pawn::ForceRight(float v)
{
	m_fHorizontal = v;
}

void AUSB_Player_Pawn::RotateYaw(float v)
{
	AddControllerYawInput(v);
}

void AUSB_Player_Pawn::RotatePitch(float v)
{
	AddControllerPitchInput(v);
}

void AUSB_Player_Pawn::TickForceMove(float delta)
{
	if (Controller != NULL)
	{
		if (m_fHorizontal != 0 || m_fVertical != 0)
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			const FVector XDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) * m_fVertical;
			const FVector YDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) * m_fHorizontal;

			FVector ForceDir = XDirection + YDirection;
			if (!ForceDir.Normalize())
			{
				return;
			}

			ForceDir *= m_fMovingForce;
			GetHead()->AddForce(ForceDir);

			
		}
	}
}

void AUSB_Player_Pawn::TickHeadYawRotate(float delta)
{
	FVector wantDirection = GetHeadVelocityDir();

	if (wantDirection.IsNearlyZero(0.1f))
	{
		return;
	}
	FRotator VelocityRot = wantDirection.Rotation();
	FRotator HeadRot = GetHead()->GetComponentRotation();

	float YawDist = VelocityRot.Yaw - HeadRot.Yaw;
	PRINTF("DD %.1f", YawDist);//언리얼 특유의 각도 보정때문에
	//360나왔다가 작은수가 나왔다가 한다.

	//-300도 와 60도는 같다.
	//가장 큰 갭은 180도이다.
	//절대값이 180보다 클때만
	//360 - 절대값

	YawDist = FMath::Abs(YawDist);

	if (YawDist > 180)
	{
		YawDist = 360 - YawDist;
	}

	PRINTF("Now Yaw Gap %.1f", YawDist);

	FRotator SlerpRotation = UKismetMathLibrary::RInterpTo(HeadRot, VelocityRot,delta,m_fOrientRotSpeed *(180.f /  YawDist));

	GetHead()->SetWorldRotation(SlerpRotation,true,nullptr,ETeleportType::TeleportPhysics);

}

void AUSB_Player_Pawn::TickHeadRollRotate(float delta)
{
}

void AUSB_Player_Pawn::TickHeadPitchRotate(float delta)
{
}

void AUSB_Player_Pawn::TickLimitVelocity()
{
	FVector Velo = GetHead()->GetPhysicsLinearVelocity();

	if (Velo.SizeSquared() > m_fMaxLinearSpeedSqr)
	{
		GetHead()->SetPhysicsLinearVelocity(Velo.GetClampedToMaxSize(m_fLimitLinearVelocity));
	}

	GetHead()->SetPhysicsAngularVelocity(FVector());//벽에 닿을때 회전이 멈추질 않아서 이럼
}

