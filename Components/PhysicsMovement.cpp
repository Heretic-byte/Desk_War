// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsMovement.h"
#include "Datas/USB_Macros.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"


UPhysicsMovement::UPhysicsMovement(const FObjectInitializer& objInit)
{
	m_MovingTarget = nullptr;
	m_DeferredUpdatedMoveComponent = nullptr;
	m_bMovementInProgress = false;
	m_bDeferUpdateMoveComponent = false;
	m_fJumpZVelocity = 540.f;
	m_fMovingForce = 5000.f;
	m_bOnGround = false;
	m_bPressedJump = false;
	m_fAngularDampingForPhysicsAsset = 1.f;
	m_fLinearDampingForPhysicsAsset = 1.f;
	m_fGroundCastOffset = -45.f;
	m_RotationRate = FRotator(500.f,180.f,180.f);
}

void UPhysicsMovement::SetUpdatedComponent(USceneComponent * NewUpdatedComponent)
{
	if (NewUpdatedComponent)
	{
		m_MovingTarget = Cast<UPrimitiveComponent>(NewUpdatedComponent);
		if (!m_MovingTarget)
		{
			PRINTF("Target is not Primitive");
			return;
		}
	}

	if (m_bMovementInProgress)
	{
		m_bDeferUpdateMoveComponent = true;
		m_DeferredUpdatedMoveComponent = NewUpdatedComponent;
		return;
	}
	m_bDeferUpdateMoveComponent = false;
	m_DeferredUpdatedMoveComponent = nullptr;
	USceneComponent* OldUpdatedComponent = UpdatedComponent;

	Super::SetUpdatedComponent(NewUpdatedComponent);

	if (UpdatedComponent == NULL)
	{
		StopActiveMovement();
	}

	if (!m_MovingTarget->IsSimulatingPhysics())
	{
		m_MovingTarget->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		m_MovingTarget->SetSimulatePhysics(true);
	}

	if (m_MovingTarget->GetBodyInstance())
	{
		m_MovingTarget->SetAngularDamping(m_fAngularDampingForPhysicsAsset);
		m_MovingTarget->SetLinearDamping(m_fLinearDampingForPhysicsAsset);
		m_MovingTarget->GetBodyInstance()->UpdateDampingProperties();
	}
}

void UPhysicsMovement::SetVelocityBone(FName boneName)
{
	m_NameLinearVelocityBone = boneName;
}

void UPhysicsMovement::SetMovingForce(float fForce)
{
	m_fMovingForce = fForce;
}

void UPhysicsMovement::SetJumpZVelocity(float zVelo)
{
	m_fJumpZVelocity = zVelo;
}

void UPhysicsMovement::SetAngularDamping(float fAngDamp)
{
	m_fAngularDampingForPhysicsAsset = fAngDamp;
}

void UPhysicsMovement::SetLinearDamping(float fLinDamp)
{
	m_fLinearDampingForPhysicsAsset = fLinDamp;
}

void UPhysicsMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime) || !GetWorld())
	{
		return;
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickCastGround();
	FVector InputDir = ConsumeInputVector();
	m_Acceleration = ScaleInputAccel(InputDir);
	TickMovement(DeltaTime);
}
void UPhysicsMovement::TickMovement(float delta)
{
	TickRotate(delta);
	Velocity += m_Acceleration * delta;
	UpdateComponentVelocity();
}
void UPhysicsMovement::TickRotate(float delta)
{
	if (m_Acceleration.IsNearlyZero(0.1f))
	{
		return;
	}

	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
	CurrentRotation.DiagnosticCheckNaN(TEXT("UPhysicsMovement::Rotate(): CurrentRotation"));

	FRotator DeltaRot = GetDeltaRotation(delta);
	DeltaRot.DiagnosticCheckNaN(TEXT("UPhysicsMovement::Rotate(): GetDeltaRotation"));

	FRotator DesiredRotation = CurrentRotation;

	DesiredRotation = ComputeOrientToMovementRotation(CurrentRotation,  DeltaRot);

	DesiredRotation.Normalize();

	const float AngleTolerance = 1e-1f;


	if (!CurrentRotation.Equals(DesiredRotation, AngleTolerance))
	{
		if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
		{
			DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
		}

		if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
		{
			DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
		}

		if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
		{
			DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
		}

		DesiredRotation.DiagnosticCheckNaN(TEXT("UPhysicsMovement::Rotate(): DesiredRotation"));

		FVector WantDegree = DesiredRotation.UnrotateVector(m_MovingTarget->GetForwardVector());

		//m_MovingTarget->SetPhysicsAngularVelocityInDegrees(DesiredRotation.Euler());
		MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, true, nullptr, ETeleportType::TeleportPhysics);
	}
}

FRotator UPhysicsMovement::GetDeltaRotation(float DeltaTime) const
{
	return FRotator(GetAxisDeltaRotation(m_RotationRate.Pitch, DeltaTime), GetAxisDeltaRotation(m_RotationRate.Yaw, DeltaTime), GetAxisDeltaRotation(m_RotationRate.Roll, DeltaTime));
}

float UPhysicsMovement::GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime) const
{
	return (InAxisRotationRate >= 0.f) ? (InAxisRotationRate * DeltaTime) : 360.f;
}

FRotator UPhysicsMovement::ComputeOrientToMovementRotation(const FRotator& CurrentRotation, FRotator& DeltaRotation) const
{
	FVector WantRotate = m_Acceleration;

	if (IsGround())
	{
		WantRotate += m_GroundHitResult.ImpactNormal;
	}

	if (WantRotate.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		return CurrentRotation;
	}
	return WantRotate.GetSafeNormal().Rotation();
}

void UPhysicsMovement::TickCastGround()
{
	FVector TraceStart = m_MovingTarget->GetComponentLocation();
	TraceStart.Z += 12.f;

	FVector TraceEnd = TraceStart;
	TraceEnd.Z += m_fGroundCastOffset;

	FCollisionQueryParams QueryParam;
	QueryParam.AddIgnoredActor(this->GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(m_GroundHitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParam))
	{
		m_bOnGround = true;

		PRINTF("Ground : %s", *m_GroundHitResult.ImpactNormal.Rotation().ToString());
	}
	else
	{
		m_bOnGround = false;
	}
}

void UPhysicsMovement::UpdateComponentVelocity()
{
	if (!m_MovingTarget)
	{
		return;
	}

	m_MovingTarget->SetPhysicsLinearVelocity(Velocity, true, m_NameLinearVelocityBone);

	Velocity= FVector::ZeroVector;
}


bool UPhysicsMovement::DoJump()
{
	if (PawnOwner)
	{
		// Don't jump if we can't move up/down.
		if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
		{
			PRINTF("JumpDID");
			Velocity.Z = FMath::Max(Velocity.Z, m_fJumpZVelocity);
			return true;
		}
	}

	return false;
}


bool UPhysicsMovement::IsGround() const
{
	return m_bOnGround;
}

bool UPhysicsMovement::IsWalkable(const FHitResult & Hit) const
{
	if (!Hit.IsValidBlockingHit())
	{
		return false;
	}

	if (Hit.ImpactNormal.Z < KINDA_SMALL_NUMBER)
	{
		return false;
	}

	float TestWalkableZ = m_fWalkableFloorZ;

	const UPrimitiveComponent* HitComponent = Hit.Component.Get();
	if (HitComponent)
	{
		const FWalkableSlopeOverride& SlopeOverride = HitComponent->GetWalkableSlopeOverride();
		TestWalkableZ = SlopeOverride.ModifyWalkableFloorZ(TestWalkableZ);
	}

	if (Hit.ImpactNormal.Z < TestWalkableZ)
	{
		return false;
	}

	return true;
}


FVector UPhysicsMovement::ScaleInputAccel(const FVector inputPure) const
{
	return GetMaxForce() *inputPure.GetClampedToMaxSize(1.f);
}


void UPhysicsMovement::Jump()
{
}

void UPhysicsMovement::StopJumping()
{
	m_bPressedJump = false;
}


float UPhysicsMovement::GetMaxForce() const
{
	return m_fMovingForce;
}

void UPhysicsMovement::AddForce(FVector forceWant)
{
	if (forceWant.IsNearlyZero())
	{
		return;
	}

	m_MovingTarget->AddForce(forceWant);
}

void UPhysicsMovement::AddImpulse(FVector impulseWant)
{
	if (impulseWant.IsNearlyZero())
	{
		return;
	}

	m_MovingTarget->AddImpulse(impulseWant);
}
