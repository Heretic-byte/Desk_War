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

	m_fAngularDamping = .1f;
	m_fTorqueSpeedWeight = .1f;
	m_fJumpZVelocity = 540.f;
	m_fAirControlWeight = 1.f;
	m_bIsGround = true;


	m_ForwardBoneName = FName(TEXT("joint2"));
}

// Called when the game starts or when spawned
void AUSB_Player_Pawn::BeginPlay()
{
	Super::BeginPlay();
	SetHead(m_MeshUsb);
	m_fMaxLinearSpeedSqr = FMath::Square<float>(m_fLimitLinearVelocity);
	GetHead()->SetAngularDamping(m_fAngularDamping);
	GetHead()->GetBodyInstance()->UpdateDampingProperties();

	PRINTF("Walkable angle %f", GetHead()->GetWalkableSlopeOverride().GetWalkableSlopeAngle());

}

// Called every frame
void AUSB_Player_Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Controller == NULL)
	{
		return;
	}

	SetInputForceDir();

	TickForceMove(DeltaTime);

	TickYawRotate(m_ForceDir, DeltaTime);

	TickPitchRollRotate(DeltaTime);

	TickLimitVelocity();
}

void AUSB_Player_Pawn::SetInputForceDir()
{
	if (m_fHorizontal != 0 || m_fVertical != 0)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector XDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) * m_fVertical;
		const FVector YDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) * m_fHorizontal;

		m_ForceDir = XDirection + YDirection;


		m_ForceDir.Normalize(0.01f);


		DrawDebugLine(
			GetWorld(),
			GetHead()->GetComponentLocation(),
			GetHead()->GetComponentLocation() + 50.f * m_ForceDir,
			FColor(0, 0, 255),
			false, -1, 0,
			4.333
		);
	}
	else
	{
		m_ForceDir = FVector::ZeroVector;
	}
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

bool AUSB_Player_Pawn::GetIsGround() const
{
	return m_bIsGround;
}

void AUSB_Player_Pawn::SetHead(UPrimitiveComponent * headWantPhysics)
{
	m_PrimHead = headWantPhysics;
}

FVector AUSB_Player_Pawn::GetHeadVelocity()
{
	FVector Velo = GetHead()->GetComponentVelocity();
	

		DrawDebugLine(
			GetWorld(),
			GetHead()->GetComponentLocation(),
			GetHead()->GetComponentLocation()+  40.f * Velo,
			FColor(255, 0, 0),
			false, -1, 0,
			4.333
		);

		return Velo;
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
	float AirW = (GetIsGround() ? 1.f : m_fAirControlWeight);



	GetHead()->SetPhysicsLinearVelocity(delta*m_ForceDir*m_fMovingForce * AirW, true, m_ForwardBoneName);
}

void AUSB_Player_Pawn::TickYawRotate(FVector &ForceDir, float delta)
{
	float DotYaw = FVector::DotProduct(ForceDir, GetHead()->GetRightVector());

	auto VYaw = UKismetMathLibrary::GetUpVector(GetHead()->GetComponentRotation());

	FVector VeloDir = GetHeadVelocity();

	if (!VeloDir.IsNearlyZero(0.1f))
	{
		GetHead()->SetPhysicsAngularVelocityInDegrees(delta*VYaw * DotYaw *m_YawW, true, m_ForwardBoneName);
	}
}

void AUSB_Player_Pawn::TickPitchRollRotate(float delta)
{
	FHitResult HitResult;
	//피직스 각도로 해보자
	CastGround(HitResult);

	float PitchWant = UKismetMathLibrary::MakeRotFromYZ(GetHead()->GetRightVector(), HitResult.ImpactNormal).Pitch;

	float YawWant = GetHead()->GetComponentRotation().Yaw;

	float RollWant = UKismetMathLibrary::MakeRotFromXZ(GetHead()->GetForwardVector(), HitResult.ImpactNormal).Roll;

	FRotator CurrentHeadRot = GetHead()->GetComponentRotation();

	FRotator LerpedRotation = FMath::RInterpTo(CurrentHeadRot, FRotator(PitchWant, YawWant, RollWant), delta, m_LerpRotateTime);

	//GetHead()->SetWorldRotation(LerpedRotation,true,nullptr,ETeleportType::TeleportPhysics);

	//GetHead()->SetPhysicsAngularVelocity(delta*VYaw * DotYaw *m_YawW, true);
}

void AUSB_Player_Pawn::TickLimitVelocity()
{

	FVector Velo = GetHead()->GetPhysicsLinearVelocity();

	if (Velo.SizeSquared() > m_fMaxLinearSpeedSqr)
	{
		FVector ClampedGroundSpeed = Velo.GetClampedToMaxSize(m_fLimitLinearVelocity);
		GetHead()->SetPhysicsLinearVelocity(FVector(ClampedGroundSpeed.X,ClampedGroundSpeed.Y,Velo.Z));
	}

}

void AUSB_Player_Pawn::CastGround(FHitResult & hitResult)
{

	FVector TraceStart = GetHead()->GetComponentLocation();
	TraceStart.Z += 15.f;
	float Offset = -45.f;

	FVector TraceEnd = TraceStart;
	TraceEnd.Z += Offset;

	
	FCollisionQueryParams QueryParam;
	QueryParam.AddIgnoredActor(this);


	if (GetWorld()->LineTraceSingleByChannel(hitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParam))
	{
		m_bIsGround = true;
	}
	else
	{
		m_bIsGround = false;
	}
}

void AUSB_Player_Pawn::DoJump()
{
	FVector HeadLinearVelo = GetHead()->GetPhysicsLinearVelocity();
	HeadLinearVelo.Z = FMath::Max(HeadLinearVelo.Z, m_fJumpZVelocity);
	GetHead()->SetPhysicsLinearVelocity(HeadLinearVelo);
}

