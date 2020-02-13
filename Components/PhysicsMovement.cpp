// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsMovement.h"
#include "Datas/USB_Macros.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UPhysicsMovement::UPhysicsMovement(const FObjectInitializer& objInit)
{
	m_MovingTarget = nullptr;
	m_fJumpZVelocity = 540.f;
	m_fMovingForce = 5000.f;
	m_bOnGround = false;
	m_bPressedJump = false;
	m_fAngularDampingForPhysicsAsset = 1.f;
	m_fLinearDampingForPhysicsAsset = 1.f;
	m_fGroundCastOffset = -45.f;
	m_RotationRate = FRotator(180.f,180.f,500.f);
	m_bDebugShowForwardCast = false;
	m_fForwardCastOffset = 100.f;
	m_fAirControl = 0.05f;
	m_fGroundCastBoxSize = 15.f;
	m_WalkableSlopeAngle = 65.f;
}

void UPhysicsMovement::SetTraceIgnoreActorAry(TArray<AActor*>* aryWant)
{
	m_ptrAryTraceIgnoreActors = aryWant;
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

	CheckJumpInput(DeltaTime);

	TickCastGround();
	FVector InputDir = ConsumeInputVector();
	m_Acceleration = ScaleInputAccel(InputDir);
	if (!TickCheckCanMoveForward())
	{
		return;
	}
	TickMovement(DeltaTime);
}
void UPhysicsMovement::TickMovement(float delta)
{
	if (m_Acceleration.IsNearlyZero(0.1f))
	{
		return;
	}
	TickRotate(delta);

	
	Velocity = m_Acceleration * delta;

	if(!IsGround())
	{
		Velocity*= m_fAirControl;
	}

	UpdateComponentVelocity();

	ClearJumpInput(delta);
}
void UPhysicsMovement::TickRotate(float delta)
{
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

		//m_MovingTarget->SetPhysicsAngularVelocityInDegrees(DesiredRotation.Euler(),false);
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

void UPhysicsMovement::AddIgnoreActorsToQuery(FCollisionQueryParams & queryParam)
{
	queryParam.AddIgnoredActors(*m_ptrAryTraceIgnoreActors);
}

bool UPhysicsMovement::IsFalling() const
{
	return !IsGround();
}

void UPhysicsMovement::BeginPlay()
{
	Super::BeginPlay();
}

bool UPhysicsMovement::TickCheckCanMoveForward()
{
	FVector InputDirStart = m_MovingTarget->GetComponentLocation();
	FVector InputDirEnd = InputDirStart;
	InputDirEnd += m_InputNormal*m_fForwardCastOffset;
	FHitResult InputHit;
#if WITH_EDITOR
	if (m_bDebugShowForwardCast)
	{
		DrawDebugLine(
			GetWorld(),
			InputDirStart,
			InputDirEnd,
			FColor(255, 0, 0),
			false, -1, 0,
			12.333
		);
	}
#endif
	FCollisionQueryParams QueryParam;
	AddIgnoreActorsToQuery(QueryParam);

	bool bCanGoForward = true;

	if (GetWorld()->LineTraceSingleByChannel(InputHit, InputDirStart, InputDirEnd, ECollisionChannel::ECC_GameTraceChannel8, QueryParam))//막혀도 각도가 낮으면 통과시켜줘
	{
		bCanGoForward = m_WalkableSlopeAngle < InputHit.ImpactNormal.Rotation().Pitch;
	}

	return bCanGoForward;
}

void UPhysicsMovement::TickCastGround()
{
	FVector TraceStart = m_MovingTarget->GetComponentLocation();
	TraceStart.Z -= m_fGroundCastBoxSize;

	FVector TraceEnd = TraceStart;
	TraceEnd.Z += m_fGroundCastOffset;

	FCollisionQueryParams QueryParam;
	AddIgnoreActorsToQuery(QueryParam);

	const FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(m_fGroundCastBoxSize, m_fGroundCastBoxSize, m_fGroundCastBoxSize));

#if WITH_EDITOR
	if (m_bDebugShowForwardCast)
	{
		DrawDebugBox(GetWorld(), TraceStart, FVector(m_fGroundCastBoxSize, m_fGroundCastBoxSize, m_fGroundCastBoxSize), FColor(120, 0, 120), false, -1.f, 0.1f);
		DrawDebugBox(GetWorld(), TraceEnd, FVector(m_fGroundCastBoxSize, m_fGroundCastBoxSize, m_fGroundCastBoxSize), FColor(120, 0, 120), false, -1.f, 0.1f);
	}
#endif

	m_bOnGround=GetWorld()->SweepSingleByChannel(m_GroundHitResult,TraceStart,TraceEnd,FQuat(FVector(0.f, 0.f, -1.f), PI * 0.25f),ECollisionChannel::ECC_Visibility,BoxShape,QueryParam);

	if (!m_bOnGround)
	{
		m_GroundHitResult.Reset(1.f, false);

		m_bOnGround =GetWorld()->SweepSingleByChannel(m_GroundHitResult,TraceStart,TraceEnd,FQuat::Identity,ECollisionChannel::ECC_Visibility,BoxShape,QueryParam);
	}
}

void UPhysicsMovement::UpdateComponentVelocity()
{
	if (!m_MovingTarget)
	{
		return;
	}
	FVector CurrentV = m_MovingTarget->GetPhysicsLinearVelocity();
	
	Velocity.Z = CurrentV.Z;

	m_MovingTarget->SetPhysicsLinearVelocity(Velocity, false, m_NameLinearVelocityBone);
}

bool UPhysicsMovement::IsGround() const
{
	return m_bOnGround;
}

FVector UPhysicsMovement::ScaleInputAccel(const FVector inputPure)
{
	m_InputNormal = inputPure.GetClampedToMaxSize(1.f);
	return GetMaxForce() *m_InputNormal;
}


void UPhysicsMovement::Jump()
{
	m_bPressedJump = true;
	m_fJumpKeyHoldTime = 0.0f;
}

void UPhysicsMovement::StopJumping()
{
	m_bPressedJump = false;
	ResetJumpState();
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


void UPhysicsMovement::CheckJumpInput(float DeltaTime)
{
	if (m_bPressedJump)
	{
		const bool bDidJump = DoJump();
		if (bDidJump)
		{
			m_bPressedJump = false;
			if (!m_bWasJumping)
			{
				m_fJumpForceTimeRemaining = m_fMaxHoldTime;
			}
		}
		m_bWasJumping = bDidJump;
	}
}


bool UPhysicsMovement::DoJump()
{
	if (CanJump())
	{
		PRINTF("Did Jump");
		FVector CurrentV = m_MovingTarget->GetPhysicsLinearVelocity();

		CurrentV.Z = FMath::Max(CurrentV.Z, m_fJumpZVelocity);

		m_MovingTarget->SetPhysicsLinearVelocity(CurrentV);
		//TestTail->SetPhysicsLinearVelocity(CurrentV, false, Test);
		m_nJumpCurrentCount++;
		return true;
	}

	return false;
}

void UPhysicsMovement::ClearJumpInput(float delta)
{
	if (m_bPressedJump)
	{
		m_fJumpKeyHoldTime += delta;

		if (m_fJumpKeyHoldTime >= m_fMaxHoldTime)
		{
			m_bPressedJump = false;
			
		}
	}
	else
	{
		m_fJumpForceTimeRemaining = 0.0f;
		m_bWasJumping = false;
	}
}


bool UPhysicsMovement::CanJump() const
{
	bool bCanJump = true;
	PRINTF("Can Jump1");

	if (m_nJumpCurrentCount == 0)
	{
		PRINTF("Can Jump2");
		bCanJump = IsGround();
	}
	bCanJump =  m_nJumpCurrentCount < m_nJumpMaxCount;

	return bCanJump;
}

void UPhysicsMovement::ResetJumpState()
{
	m_bPressedJump = false;
	m_bWasJumping = false;
	m_fJumpKeyHoldTime = 0.0f;
	m_fJumpForceTimeRemaining = 0.0f;
	PRINTF("Reset Jump1");
	if (!IsFalling())
	{
		PRINTF("Reset Jump2");
		m_nJumpCurrentCount = 0;
	}
}
