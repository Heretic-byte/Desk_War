// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsMovement.h"
#include "Datas/USB_Macros.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "PhysicsPublic.h"
#include "Engine/World.h"

UPhysicsMovement::UPhysicsMovement(const FObjectInitializer& objInit)
{
	m_bUsingAutoMove = false;
	m_fInitHeadMass = 1.f;
	m_bBlockMove = false;
	m_MovingTarget = nullptr;
	m_fJumpZVelocity = 540.f;
	m_fMovingForce = 5000.f;
	m_bOnGround = false;
	m_bPressedJump = false;
	m_fGroundCastOffset = -45.f;
	m_RotationRate = FRotator(180.f,180.f,500.f);
	m_bDebugShowForwardCast = false;
	m_fAirControl = 0.05f;
	m_fGroundCastBoxSize = 15.f;
	m_WalkableSlopeAngle = 65.f;
	m_fBlockMoveTime = 0.f;
	m_fBlockMoveTimer = 0.f;
	m_fAddTraceMultipleLength = 1.f;
}

void UPhysicsMovement::BeginPlay()
{
	Super::BeginPlay();
	UWorld* World = GetWorld();
	if (World)
	{
		FPhysScene* PScene = World->GetPhysicsScene();
		if (PScene)
		{
			OnPhysSceneStepHandle = PScene->OnPhysSceneStep.AddUObject(this, &UPhysicsMovement::PhysSceneStep);
		}
	}
}

void UPhysicsMovement::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FPhysScene* PScene = World->GetPhysicsScene();
		if (PScene)
		{
			PScene->OnPhysSceneStep.Remove(OnPhysSceneStepHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UPhysicsMovement::SetAutoMoveTimer(FVector dirWant, float timeWant)
{
	m_AutoMoveDir = dirWant;

	SetBlockMoveTimer(timeWant);

	m_bUsingAutoMove = true;
}

void UPhysicsMovement::SetUpdatePhysicsMovement(UPhysicsSkMeshComponent * headUpdatedCompo, UPhysicsSkMeshComponent * tailUpdatedCompo)
{
	SetCastingLength(headUpdatedCompo);;
	SetUpdatedComponent(headUpdatedCompo);
	m_MovingTargetTail = tailUpdatedCompo;
}

void UPhysicsMovement::SetCastingLength(UPhysicsSkMeshComponent * headUpdatedCompo)
{
	m_fGroundCastOffset /= m_fAddTraceMultipleLength;
	m_fGroundCastBoxSize /= m_fAddTraceMultipleLength;
	m_fAddTraceMultipleLength = headUpdatedCompo->GetMeshRadiusMultiple();
	m_fGroundCastOffset *= m_fAddTraceMultipleLength;
	m_fGroundCastBoxSize *= m_fAddTraceMultipleLength;
}

void UPhysicsMovement::SetTraceIgnoreActorAry(TArray<AActor*>* aryWant)
{
	m_ptrAryTraceIgnoreActors = aryWant;
}

void UPhysicsMovement::SetUpdatedComponent(USceneComponent * NewUpdatedComponent)
{
	if (NewUpdatedComponent)
	{
		m_MovingTarget = Cast<UPhysicsSkMeshComponent>(NewUpdatedComponent);
		if (!m_MovingTarget)
		{
			PRINTF("Target is not USkeletalMesh");
			return;
		}
	}

	USceneComponent* OldUpdatedComponent = UpdatedComponent;

	UMovementComponent::SetUpdatedComponent(NewUpdatedComponent);

	PawnOwner = NewUpdatedComponent ? CastChecked<APawn>(NewUpdatedComponent->GetOwner()) : NULL;

	if (UpdatedComponent == NULL)
	{
		StopActiveMovement();
	}

	if (!m_MovingTarget->IsSimulatingPhysics())
	{
		m_MovingTarget->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		m_MovingTarget->SetSimulatePhysics(true);
	}
}

void UPhysicsMovement::SetDamping(float fLinDamp, float fAngDamp)
{
	if (!GetWorld())
	{
		return;
	}

	if (m_MovingTarget->GetBodyInstance())
	{
		m_MovingTarget->SetAngularDamping(fAngDamp);
		m_MovingTarget->SetLinearDamping(fLinDamp);
		m_MovingTarget->GetBodyInstance()->UpdateDampingProperties();
	}
}

void UPhysicsMovement::SetInitHeadMass(float massHead)
{
	m_fInitHeadMass = massHead;
}

void UPhysicsMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector InputDir;

	if (m_bBlockMove)
	{
		m_fBlockMoveTimer += DeltaTime;

		if (0<m_fBlockMoveTime&&m_fBlockMoveTimer > m_fBlockMoveTime)
		{
			m_fBlockMoveTimer = 0.f;
			m_bBlockMove = false;

			if (m_bUsingAutoMove)
			{
				m_bUsingAutoMove = false;
				m_AutoMoveDir = FVector::ZeroVector;
				m_OnAutoMoveEnd.Broadcast();
			}
		}

		if (m_bUsingAutoMove)
		{
			InputDir = m_AutoMoveDir;
		}
		else
		{
			return;
		}
	}
	else
	{
		InputDir= ConsumeInputVector();
	}
	SetAccelerationByDir(InputDir);
	TickRotate(DeltaTime);
	CheckJumpInput(DeltaTime);
	ClearJumpInput(DeltaTime);
}

void UPhysicsMovement::PhysSceneStep(FPhysScene * PhysScene, float DeltaTime)
{
	if (!PawnOwner || !UpdatedComponent || !GetWorld())
	{
		return;
	}

	TickCastGround();
	
	TickMovement(DeltaTime);
	
}

void UPhysicsMovement::TickMovement(float delta)
{
	if (m_Acceleration.SizeSquared2D() <1)//maybe square better?
	{
		return;
	}
	Velocity = m_Acceleration * delta;

	if(!IsGround())
	{
		Velocity*= m_fAirControl;
	}

	UpdateComponentVelocity();
}
void UPhysicsMovement::TickRotate(float delta)
{
	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
	CurrentRotation.DiagnosticCheckNaN(TEXT("UPhysicsMovement::Rotate(): CurrentRotation"));

	FRotator DeltaRot = GetDeltaRotation(delta);//µ¹¸± °ª
	DeltaRot.DiagnosticCheckNaN(TEXT("UPhysicsMovement::Rotate(): GetDeltaRotation"));

	FRotator DesiredRotation = CurrentRotation;
	//
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

bool UPhysicsMovement::TickCheckCanMoveForward()
{
	float PlayerAngle = m_MovingTarget->GetComponentRotation().Pitch;

	if (PlayerAngle <= 0)
	{
		return true;
	}

	float Angle = 90.f - m_GroundHitResult.ImpactNormal.Rotation().Pitch;

	/*if (m_WalkableSlopeAngle < Angle)
	{
		return false;
	}*/

	return true;
}

void UPhysicsMovement::TickCastGround()
{
	FVector TraceStart = m_MovingTarget->GetComponentLocation();
	TraceStart.Z -= m_fGroundCastBoxSize;

	FVector TraceEnd = TraceStart;
	TraceEnd.Z += m_fGroundCastOffset*m_fAddTraceMultipleLength;

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
	m_MovingTarget->SetPhysicsLinearVelocity(Velocity);
}

bool UPhysicsMovement::IsGround() const
{
	return m_bOnGround;
}

void UPhysicsMovement::SetAccelerationByDir(const FVector inputPure)
{
	m_InputNormal = inputPure.GetClampedToMaxSize(1.f);

	m_Acceleration= GetMaxForce() *m_InputNormal;
}

void UPhysicsMovement::Jump()
{
	m_bPressedJump = true;
	m_fJumpKeyHoldTime = 0.0f;
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

void UPhysicsMovement::SetBlockMoveTimer(float wantBlockTime)
{
	m_fBlockMoveTimer = 0.f;
	if (wantBlockTime == 0)
	{
		m_fBlockMoveTime = 0.f;
		m_bBlockMove = false;
		return;
	}
	m_fBlockMoveTime = wantBlockTime;
	m_bBlockMove = true;
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
		FVector CurrentV = m_MovingTarget->GetPhysicsLinearVelocity();
		CurrentV.Z = FMath::Max(m_fJumpZVelocity, CurrentV.Z);
		
		float CurrentTargetMass = m_MovingTarget->GetBodyInstance()->GetBodyMass();
		float TargetTailMassRate = CurrentTargetMass / m_fInitHeadMass;

		float CurrentTailMass = m_MovingTargetTail->GetBodyInstance()->GetBodyMass();
		float TargetMassRate = CurrentTailMass / m_fInitHeadMass;


		m_MovingTarget->SetPhysicsLinearVelocity(CurrentV * TargetMassRate);
		m_MovingTargetTail->SetPhysicsLinearVelocity(CurrentV * TargetTailMassRate);

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

	if(IsGround())
		ResetJumpState();
}

bool UPhysicsMovement::CanJump() const
{
	bool bCanJump = true;

	if (m_nJumpCurrentCount == 0)
	{
		bCanJump = IsGround();
	}
	bCanJump =  m_nJumpCurrentCount < m_nJumpMaxCount -1;

	return bCanJump;
}

void UPhysicsMovement::ResetJumpState()
{
	m_bPressedJump = false;
	m_bWasJumping = false;
	m_fJumpKeyHoldTime = 0.0f;
	m_fJumpForceTimeRemaining = 0.0f;
	m_nJumpCurrentCount = 0;
}
