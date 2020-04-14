#include "USB_SpringArm.h"
#include "Datas/USB_Macros.h"
#include "GameFramework/Pawn.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/MaterialControl.h"

const FName UUSB_SpringArm::SocketName(TEXT("SpringEndpoint"));

// Sets default values for this component's properties
UUSB_SpringArm::UUSB_SpringArm(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	m_BlockedMatControl = nullptr;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	m_fWheelZoomSpeed = 10.f;
	bAutoActivate = true;
	bTickInEditor = true;
	bUsePawnControlRotation = false;
	bDoCollisionTest = true;

	bInheritPitch = true;
	bInheritYaw = true;
	bInheritRoll = true;

	TargetArmLength = 300.0f;
	m_fMinimumArmLength = 10.f;
	m_fCamZoomInSpeed = 10.f;
	ProbeSize = 12.0f;
	ProbeChannel = ECC_Camera;

	RelativeSocketRotation = FQuat::Identity;

	bUseCameraLagSubstepping = true;
	CameraLagSpeed = 10.f;
	CameraRotationLagSpeed = 10.f;
	CameraLagMaxTimeStep = 1.f / 60.f;
	CameraLagMaxDistance = 0.f;
	UnfixedCameraPosition = FVector::ZeroVector;
}

void UUSB_SpringArm::BeginPlay()
{
	Super::BeginPlay();
	m_fMaximumArmLength = TargetArmLength;
	
}

FRotator UUSB_SpringArm::GetTargetRotation() const
{
	FRotator DesiredRot = GetComponentRotation();

	if (bUsePawnControlRotation)
	{
		if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
		{
			const FRotator PawnViewRotation = OwningPawn->GetViewRotation();
			if (DesiredRot != PawnViewRotation)
			{
				DesiredRot = PawnViewRotation;
			}
		}
	}

	// If inheriting rotation, check options for which components to inherit
	if (!bAbsoluteRotation)
	{
		if (!bInheritPitch)
		{
			DesiredRot.Pitch = RelativeRotation.Pitch;
		}

		if (!bInheritYaw)
		{
			DesiredRot.Yaw = RelativeRotation.Yaw;
		}

		if (!bInheritRoll)
		{
			DesiredRot.Roll = RelativeRotation.Roll;
		}
	}

	return DesiredRot + m_RotOffset;
}

void UUSB_SpringArm::UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime)
{
	FRotator DesiredRot = GetTargetRotation();

	// Apply 'lag' to rotation if desired
	if (bDoRotationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraRotationLagSpeed > 0.f)
		{
			const FRotator ArmRotStep = (DesiredRot - PreviousDesiredRot).GetNormalized() * (1.f / DeltaTime);
			FRotator LerpTarget = PreviousDesiredRot;
			float RemainingTime = DeltaTime;
			while (RemainingTime > KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmRotStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(LerpTarget), LerpAmount, CameraRotationLagSpeed));
				PreviousDesiredRot = DesiredRot;
			}
		}
		else
		{
			DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(DesiredRot), DeltaTime, CameraRotationLagSpeed));
		}
	}
	PreviousDesiredRot = DesiredRot;

	// Get the spring arm 'origin', the target we want to look at
	FVector ArmOrigin = GetComponentLocation() + TargetOffset;
	// We lag the target, not the actual camera position, so rotating the camera around does not have lag
	FVector DesiredLoc = ArmOrigin;
	if (bDoLocationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraLagSpeed > 0.f)
		{
			const FVector ArmMovementStep = (DesiredLoc - PreviousDesiredLoc) * (1.f / DeltaTime);
			FVector LerpTarget = PreviousDesiredLoc;

			float RemainingTime = DeltaTime;
			while (RemainingTime > KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmMovementStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, CameraLagSpeed);
				PreviousDesiredLoc = DesiredLoc;
			}
		}
		else
		{
			DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, CameraLagSpeed);
		}

		// Clamp distance if requested
		bool bClampedDist = false;
		if (CameraLagMaxDistance > 0.f)
		{
			const FVector FromOrigin = DesiredLoc - ArmOrigin;
			if (FromOrigin.SizeSquared() > FMath::Square(CameraLagMaxDistance))
			{
				DesiredLoc = ArmOrigin + FromOrigin.GetClampedToMaxSize(CameraLagMaxDistance);
				bClampedDist = true;
			}
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (bDrawDebugLagMarkers)
		{
			DrawDebugSphere(GetWorld(), ArmOrigin, 5.f, 8, FColor::Green);
			DrawDebugSphere(GetWorld(), DesiredLoc, 5.f, 8, FColor::Yellow);

			const FVector ToOrigin = ArmOrigin - DesiredLoc;
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc, DesiredLoc + ToOrigin * 0.5f, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc + ToOrigin * 0.5f, ArmOrigin, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
		}
#endif
	}

	PreviousArmOrigin = ArmOrigin;
	PreviousDesiredLoc = DesiredLoc;

	DesiredLoc -= DesiredRot.Vector() * TargetArmLength;
	DesiredLoc += FRotationMatrix(DesiredRot).TransformVector(SocketOffset);

	FVector ResultLoc;

	if (bDoTrace && (TargetArmLength != 0.0f))
	{
		ResultLoc= CollisionCameraFix(ArmOrigin, DesiredLoc, DeltaTime);
	}
	else
	{
		ResultLoc = DesiredLoc;
		bIsCameraFixed = false;
		UnfixedCameraPosition = ResultLoc;
	}

	FTransform WorldCamTM(DesiredRot, ResultLoc);
	FTransform RelCamTM = WorldCamTM.GetRelativeTransform(GetComponentTransform());
	
	m_LastTarget = ResultLoc;

	RelativeSocketLocation = RelCamTM.GetLocation();
	RelativeSocketRotation = RelCamTM.GetRotation();

	UpdateChildTransforms();
}

FVector UUSB_SpringArm::CollisionCameraFix(FVector &ArmOrigin, FVector &DesiredLoc, float DeltaTime)
{
	FVector ResultLoc;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());
	FHitResult Result;

	bIsCameraFixed = true;

	GetWorld()->SweepSingleByChannel(Result, ArmOrigin, DesiredLoc, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeSize), QueryParams);

	UnfixedCameraPosition = DesiredLoc;

	ResultLoc = BlendLocations(DesiredLoc, Result.Location, Result.bBlockingHit, DeltaTime, Result);

	if (ResultLoc == DesiredLoc)
	{
		bIsCameraFixed = false;
	}

	return ResultLoc;
}

FVector UUSB_SpringArm::BlendLocations(const FVector& DesiredArmLocation, const FVector& TraceHitLocation, bool bHitSomething, float DeltaTime, const FHitResult& hit)
{
	if (m_BlockedMatControl)
	{
		m_BlockedMatControl->SetInitAlpha();
		m_BlockedMatControl = nullptr;
	}

	if (!bHitSomething)
	{
		return DesiredArmLocation;
	}
	
	UMaterialControl* MatCon = Cast<UMaterialControl>( hit.GetActor()->GetComponentByClass(UMaterialControl::StaticClass()));

	if (MatCon)
	{
		m_BlockedMatControl = MatCon;
		m_BlockedMatControl->SetAlpha();
	}

	auto LerpedLocation= UKismetMathLibrary::VInterpTo(m_LastTarget, TraceHitLocation, DeltaTime, CameraLagSpeed);
	return LerpedLocation;
}

FVector UUSB_SpringArm::ClampTargetLocation(const FVector& traceLoc,const FHitResult& hit)
{
	if (m_fMinimumArmLength > hit.Distance)//왜안되는지 모르겠다.
	{
		return FVector(traceLoc.X, traceLoc.Y, traceLoc.Z + m_fMinimumArmLength);
	}

	return traceLoc;
}


void UUSB_SpringArm::ApplyWorldOffset(const FVector & InOffset, bool bWorldShift)
{
	Super::ApplyWorldOffset(InOffset, bWorldShift);
	PreviousDesiredLoc += InOffset;
	PreviousArmOrigin += InOffset;
}

void UUSB_SpringArm::ZoomIn()
{
	TargetArmLength -= m_fWheelZoomSpeed;

	if (TargetArmLength < m_fMinimumArmLength)
	{
		TargetArmLength = m_fMinimumArmLength;
	}
}

void UUSB_SpringArm::ZoomOut()
{
	TargetArmLength += m_fWheelZoomSpeed;

	if (TargetArmLength > m_fMaximumArmLength)
	{
		TargetArmLength = m_fMaximumArmLength;
	}
}

void UUSB_SpringArm::OnRegister()
{
	Super::OnRegister();

	// enforce reasonable limits to avoid potential div-by-zero
	CameraLagMaxTimeStep = FMath::Max(CameraLagMaxTimeStep, 1.f / 200.f);
	CameraLagSpeed = FMath::Max(CameraLagSpeed, 0.f);

	// Set initial location (without lag).
	UpdateDesiredArmLocation(false, false, false, 0.f);
}


void UUSB_SpringArm::PostLoad()
{
	Super::PostLoad();
}

void UUSB_SpringArm::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateDesiredArmLocation(bDoCollisionTest, bEnableCameraLag, bEnableCameraRotationLag, DeltaTime);
}

FTransform UUSB_SpringArm::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const
{
	FTransform RelativeTransform(RelativeSocketRotation, RelativeSocketLocation);

	switch (TransformSpace)
	{
		case RTS_World:
		{
			return RelativeTransform * GetComponentTransform();
			break;
		}
		case RTS_Actor:
		{
			if (const AActor* Actor = GetOwner())
			{
				FTransform SocketTransform = RelativeTransform * GetComponentTransform();
				return SocketTransform.GetRelativeTransform(Actor->GetTransform());
			}
			break;
		}
		case RTS_Component:
		{
			return RelativeTransform;
		}
	}
	return RelativeTransform;
}

bool UUSB_SpringArm::HasAnySockets() const
{
	return true;
}

void UUSB_SpringArm::QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const
{
	new (OutSockets) FComponentSocketDescription(SocketName, EComponentSocketType::Socket);
}

FVector UUSB_SpringArm::GetUnfixedCameraPosition() const
{
	return UnfixedCameraPosition;
}

bool UUSB_SpringArm::IsCollisionFixApplied() const
{
	return bIsCameraFixed;
}
