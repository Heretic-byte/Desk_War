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
	m_fMaxTimeStep = 33.f;
	m_fGroundFriction = 8.f;
	m_fMaxSpeed = 10000.f;
	m_fMaxBrakingDeceleration = 4500.f;
	m_fMinAnalogSpeed = 100.f;
	m_bIsWallBlocking = false;
	m_bAutoRot = false;
	m_bAutoMove = false;
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
	m_fWalkableSlopeAngle = 65.f;
	m_fBlockMoveTime = 0.f;
	m_fBlockMoveTimer = 0.f;
	m_fAutoRotTime = 0.f;
	m_fAutoRotTimer = 0.f;
	m_fAdditionalTraceMultipleLength = 1.f;
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
	m_fInitDesiredRotRollDelta = m_RotationRate.Roll;

	SetWalkableFloorAngle(m_fWalkableSlopeAngle);
}


void UPhysicsMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickCastGround();
	//TickCastFlipCheck();
	SetAccel(DeltaTime);
	m_fAnalogInputModifier = ComputeAnalogInputModifier();
	TickRotate(SelectTargetRotation(DeltaTime), DeltaTime);
	CheckJumpInput(DeltaTime);
	ClearJumpInput(DeltaTime);
}

void UPhysicsMovement::PhysSceneStep(FPhysScene * PhysScene, float DeltaTime)
{
	if (!PawnOwner || !UpdatedComponent || !GetWorld())
	{
		return;
	}

	
	if (!m_bIsWallBlocking)
	{
		CalcVelocity(DeltaTime, m_fGroundFriction);
		if (m_bBlockMove && !m_bAutoMove)
		{
			//only block move
			return;
		}

		if (m_Acceleration.SizeSquared2D() < 1)//maybe square better?
		{
			return;
		}
		TickMovement(DeltaTime);
	}

	UpdateComponentVelocity();
}

void UPhysicsMovement::CalcVelocity(float DeltaTime, float Friction)
{

	float MaxSpeed = GetMaxSpeed();
	const float MaxInputSpeed = FMath::Max(MaxSpeed * m_fAnalogInputModifier, GetMinAnalogSpeed());
	MaxSpeed = MaxInputSpeed;

	const bool bZeroAcceleration = m_Acceleration.IsZero();
	const bool bVelocityOverMax = IsExceedingMaxSpeed(MaxSpeed);

	if (bZeroAcceleration || bVelocityOverMax)
	{
	
		const FVector OldVelocity = Velocity;
		const float ActualBrakingFriction = Friction;
		ApplyVelocityBraking(DeltaTime, ActualBrakingFriction, m_fMaxBrakingDeceleration);

		if (bVelocityOverMax && Velocity.SizeSquared() < FMath::Square(MaxSpeed) && FVector::DotProduct(m_Acceleration, OldVelocity) > 0.0f)
		{
			
			Velocity = OldVelocity.GetSafeNormal() * MaxSpeed;
		}
	}
	else if (!bZeroAcceleration)
	{
		
		const FVector AccelDir = (m_Acceleration.Size() == 0) ? FVector::ZeroVector : m_Acceleration.GetSafeNormal();
		const float VelSize = Velocity.Size();
		Velocity = Velocity - (Velocity - AccelDir * VelSize) * FMath::Min(DeltaTime * Friction, 1.f);
		const float NewMaxInputSpeed = IsExceedingMaxSpeed(MaxInputSpeed) ? Velocity.Size() : MaxInputSpeed;
		Velocity += m_Acceleration * DeltaTime;
		Velocity = Velocity.GetClampedToMaxSize(NewMaxInputSpeed);
	}

	
}

float UPhysicsMovement::GetMinAnalogSpeed() const
{
	return m_fMinAnalogSpeed;
}

float UPhysicsMovement::GetMaxSpeed() const
{
	return m_fMaxSpeed;
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

void UPhysicsMovement::EnableInputMove()
{
	m_bBlockMove = false;
	m_fBlockMoveTime = 0.f;
	m_fBlockMoveTimer = 0.f;
}

void UPhysicsMovement::DisableInputMove(float timeWant)
{
	m_bBlockMove = true;
	m_fBlockMoveTime = timeWant;
	m_fBlockMoveTimer = 0.f;
}

void UPhysicsMovement::EnableAutoMove(FVector dirWant, float timeWant)
{
	m_bAutoMove = true;
	m_AutoMoveDir = dirWant;
	DisableInputMove(timeWant);
}

void UPhysicsMovement::DisableAutoMove()
{
	m_bAutoMove = false;
	m_AutoMoveDir = FVector::ZeroVector;
	m_OnAutoMoveEnd.Broadcast();
	EnableInputMove();
}

void UPhysicsMovement::EnableAutoRotate(FRotator rotWant, float timeWant)
{
	m_AutoRotateRot = rotWant;
	m_fAutoRotTime = timeWant;
	m_fAutoRotTimer = 0.f;
	m_bAutoRot = true;
}

void UPhysicsMovement::DisableAutoRotate()
{
	m_fAutoRotTime = 0.f;
	m_fAutoRotTimer = 0.f;
	m_bAutoRot = false;
	m_OnAutoRotateEnd.Broadcast();
}

void UPhysicsMovement::SetUpdatePhysicsMovement(UPhysicsSkMeshComponent * headUpdatedCompo, UPhysicsSkMeshComponent * tailUpdatedCompo)
{
	SetCastingLength(headUpdatedCompo);;
	SetUpdatedComponent(headUpdatedCompo);
	m_MovingTargetTail = tailUpdatedCompo;
}

void UPhysicsMovement::SetCastingLength(UPhysicsSkMeshComponent * headUpdatedCompo)
{
	//m_fGroundCastOffset /= m_fAddTraceMultipleLength;
	m_fGroundCastBoxSize /= m_fAdditionalTraceMultipleLength;
	m_fAdditionalTraceMultipleLength = headUpdatedCompo->GetMeshRadiusMultiple();
	//m_fGroundCastOffset *= m_fAddTraceMultipleLength;
	m_fGroundCastBoxSize *= m_fAdditionalTraceMultipleLength;
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

FRotator UPhysicsMovement::SelectTargetRotation(float delta)
{
	if (m_bAutoRot)
	{
		if (m_fAutoRotTime > 0)
		{
			m_fAutoRotTimer += delta;

			if (m_fAutoRotTime <= m_fAutoRotTimer)
			{
				DisableAutoRotate();
			}
		}
		//TargetRot = m_AutoRotateRot;
		return m_AutoRotateRot;
	}

	if (m_Acceleration.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		//TargetRot = m_MovingTarget->GetComponentRotation();
		return m_MovingTarget->GetComponentRotation();
	}

	FRotator ROt = m_Acceleration.GetSafeNormal().Rotation();
	TargetRot.Yaw = ROt.Yaw;
	//TargetRot.Roll = ROt.Roll;

	return TargetRot;
}

bool UPhysicsMovement::SetAccel(float DeltaTime)
{
	FVector InputDir;
	if (m_bBlockMove)
	{
		if (m_fBlockMoveTime > 0.f)
		{
			m_fBlockMoveTimer += DeltaTime;

			if (m_fBlockMoveTime <= m_fBlockMoveTimer)//end
			{
				if (m_bAutoMove)
				{
					DisableAutoMove();
				}
				else
				{
					EnableInputMove();
				}
			}
		}
		//inf
		if (m_bAutoMove)
		{
			InputDir = m_AutoMoveDir;
		}
		else
		{
			return false;
		}
	}
	else
	{
		InputDir = ConsumeInputVector();
	}
	SetAccelerationByDir(InputDir);
	
	return true;
}

float UPhysicsMovement::ComputeAnalogInputModifier() const
{
	const float MaxAccel = GetMaxForce();
	if (m_Acceleration.SizeSquared() > 0.f && MaxAccel > SMALL_NUMBER)
	{
		return FMath::Clamp(m_Acceleration.Size() / MaxAccel, 0.f, 1.f);
	}

	return 0.f;
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

void UPhysicsMovement::TickMovement(float delta)
{	
	const FVector Delta = FVector(Velocity.X, Velocity.Y, 0.f);// delta;
	FHitResult Hit(1.f);
	FVector RampVector = ComputeGroundMovementDelta(Delta, m_GroundHitResult);
	FVector ResultVector;
	if (SweepCanMove(RampVector, delta, Hit))
	{
		SetVelocity(RampVector, Hit, delta);
		TargetRot = RampVector.GetSafeNormal().Rotation();
		return;
	}

	if (Hit.bStartPenetrating)
	{
		ResultVector= SlideAlongOnSurface(Delta, delta,1.f, Hit.Normal, Hit, true);
	}
	else if (Hit.IsValidBlockingHit())	//일반적인 BlockingHit일 때
	{
		float LastMoveTimeSlice = delta;
		float PercentTimeApplied = Hit.Time;//시작에서 끝까지 어디서 부딫혔는지의 퍼센트
		if ((Hit.Time > 0.f) && (Hit.Normal.Z > KINDA_SMALL_NUMBER) && IsWalkable(Hit))	//Hit된 지면이 경사면으로 추정된다면 경사면을 따라 걸음
		{
			PRINTF("ClimbRamp");
			//Another walkable ramp.
			const float InitialPercentRemaining = 1.f - PercentTimeApplied;
			RampVector = ComputeGroundMovementDelta(Delta * InitialPercentRemaining, Hit);
			LastMoveTimeSlice = InitialPercentRemaining * LastMoveTimeSlice;
			
			ResultVector = RampVector;

			const float SecondHitPercent = Hit.Time * InitialPercentRemaining;
			PercentTimeApplied = FMath::Clamp(PercentTimeApplied + SecondHitPercent, 0.f, 1.f);
		}
		if (Hit.IsValidBlockingHit())
		{
			PRINTF("Blocked3");
			ResultVector= SlideAlongOnSurface(Delta,delta, 1.f - PercentTimeApplied, Hit.Normal, Hit, true);
		}
	}
	TargetRot = ResultVector.GetSafeNormal().Rotation();
	SetVelocity(ResultVector, Hit, delta);

}
void UPhysicsMovement::TickRotate(const FRotator rotateWant,float delta)
{
	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
	CurrentRotation.DiagnosticCheckNaN(TEXT("UPhysicsMovement::Rotate(): CurrentRotation"));

	FRotator DeltaRot = GetDeltaRotation(delta);//돌릴 값
	DeltaRot.DiagnosticCheckNaN(TEXT("UPhysicsMovement::Rotate(): GetDeltaRotation"));

	FRotator DesiredRotation= rotateWant;

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

void UPhysicsMovement::AddIgnoreActorsToQuery(FCollisionQueryParams & queryParam)
{
	queryParam.AddIgnoredActors(*m_ptrAryTraceIgnoreActors);
}

bool UPhysicsMovement::IsFalling() const
{
	return !IsMovingOnGround();
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
	m_fGroundDist = (TraceStart.Z - m_GroundHitResult.Location.Z);
	if (!m_bOnGround)//간혹 실패할경우 각도차이인지 확인
	{
		m_GroundHitResult.Reset(1.f, false);

		m_bOnGround = GetWorld()->SweepSingleByChannel(m_GroundHitResult,TraceStart,TraceEnd,FQuat::Identity,ECollisionChannel::ECC_Visibility,BoxShape,QueryParam);

		m_fGroundDist = (TraceStart.Z - m_GroundHitResult.Location.Z);
	}
}

void UPhysicsMovement::TickCastFlipCheck()
{
	FVector TraceStart = m_MovingTarget->GetComponentLocation();
	FVector TraceEnd = TraceStart +( m_fGroundCastOffset*m_MovingTarget->GetUpVector());//groundoffset is minus

#if WITH_EDITOR
	if (m_bDebugShowForwardCast)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor(120, 255, 120), false, -1.f, 0.1f);
	}
#endif
	FCollisionQueryParams QueryParam;
	AddIgnoreActorsToQuery(QueryParam);
	FHitResult HitResult;

	if (GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, ECollisionChannel::ECC_Visibility, FCollisionShape::MakeSphere(3.f), QueryParam))
	{
		m_RotationRate.Roll = 0.f;
	}
	else
	{
		m_RotationRate.Roll = m_fInitDesiredRotRollDelta;
	}

}

void UPhysicsMovement::SetWalkableFloorAngle(float InWalkableFloorAngle)
{
	m_fWalkableSlopeAngle = FMath::Clamp(InWalkableFloorAngle, 0.f, 90.0f);
	m_fWalkableSlopeHeight = FMath::Cos(FMath::DegreesToRadians(m_fWalkableSlopeAngle));
}

void UPhysicsMovement::SetAccelerationByDir(const FVector inputPure)
{
	m_vInputNormal = inputPure.GetSafeNormal2D();

	m_Acceleration= GetMaxForce() *m_vInputNormal;
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

bool UPhysicsMovement::IsMovingOnGround() const
{
	return m_bOnGround;
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
	m_MovingTargetTail->AddImpulse(impulseWant);
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

		//Velocity.Z = FMath::Max(Velocity.Z, m_fJumpZVelocity);

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

	if(IsMovingOnGround())
		ResetJumpState();
}

bool UPhysicsMovement::CanJump() const
{
	bool bCanJump = true;

	if (m_nJumpCurrentCount == 0)
	{
		bCanJump = IsMovingOnGround();
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

void UPhysicsMovement::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration)
{
	const float MIN_TICK_TIME = 1e-6f;
	if (Velocity.IsZero() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	const float FrictionFactor = FMath::Max(0.f, 2.f);
	Friction = FMath::Max(0.f, Friction * FrictionFactor);
	BrakingDeceleration = FMath::Max(0.f, BrakingDeceleration);
	const bool bZeroFriction = (Friction == 0.f);
	const bool bZeroBraking = (BrakingDeceleration == 0.f);

	if (bZeroFriction && bZeroBraking)
	{
		return;
	}

	const FVector OldVel = Velocity;
	float RemainingTime = DeltaTime;
	const float MaxTimeStep = FMath::Clamp(1.0f / m_fMaxTimeStep, 1.0f / 75.0f, 1.0f / 20.0f);

	const FVector RevAccel = (bZeroBraking ? FVector::ZeroVector : (-BrakingDeceleration * Velocity.GetSafeNormal()));
	while (RemainingTime >= MIN_TICK_TIME)
	{
		const float dt = ((RemainingTime > MaxTimeStep && !bZeroFriction) ? FMath::Min(MaxTimeStep, RemainingTime * 0.5f) : RemainingTime);
		RemainingTime -= dt;
		Velocity = Velocity + ((-Friction) * Velocity + RevAccel) * dt;
		if ((Velocity | OldVel) <= 0.f)
		{
			Velocity = FVector::ZeroVector;
			return;
		}
	}

	const float VSizeSq = Velocity.SizeSquared();
	if (VSizeSq <= KINDA_SMALL_NUMBER || (!bZeroBraking && VSizeSq <= FMath::Square(10.f)))
	{
		Velocity = FVector::ZeroVector;
	}
}

FVector UPhysicsMovement::ComputeGroundMovementDelta(const FVector& Delta, const FHitResult& RampHit) const
{
	const FVector FloorNormal = RampHit.ImpactNormal;
	const FVector ContactNormal = RampHit.Normal;

	//직각이 아닐때,//수평도 아닐때
	if (FloorNormal.Z < (1.f - KINDA_SMALL_NUMBER) && FloorNormal.Z > KINDA_SMALL_NUMBER&& ContactNormal.Z > KINDA_SMALL_NUMBER  && IsWalkable(RampHit))
	{
		// Compute a vector that moves parallel to the surface, by projecting the horizontal movement direction onto the ramp.
		const float FloorDotDelta = (FloorNormal | Delta);
		FVector RampMovement(Delta.X, Delta.Y, -FloorDotDelta / FloorNormal.Z);

		return RampMovement.GetSafeNormal() * Delta.Size();
	}

	return Delta;
}

bool UPhysicsMovement::IsWalkable(const FHitResult & Hit) const
{
	if (!Hit.IsValidBlockingHit())
	{
		// No hit, or starting in penetration
		return false;
	}

	// Never walk up vertical surfaces.
	if (Hit.ImpactNormal.Z < KINDA_SMALL_NUMBER)
	{
		return false;
	}
	float TestWalkableZ = m_fWalkableSlopeHeight;

	// See if this component overrides the walkable floor z.
	const UPrimitiveComponent* HitComponent = Hit.Component.Get();
	if (HitComponent)
	{
		const FWalkableSlopeOverride& SlopeOverride = HitComponent->GetWalkableSlopeOverride();
		TestWalkableZ = SlopeOverride.ModifyWalkableFloorZ(TestWalkableZ);
	}
	// Can't walk on this surface if it is too steep.
	if (Hit.ImpactNormal.Z < TestWalkableZ)
	{
		PRINTF("CantWalkable");
		return false;
	}
	PRINTF("Walkable");
	return true;
}

void UPhysicsMovement::SetVelocity(FVector& velocity,FHitResult & sweep, float delta)
{
	if (!m_MovingTarget)
	{
		return;
	}
	FVector CurrentV = m_MovingTarget->GetPhysicsLinearVelocity();

	velocity.Z = CurrentV.Z;
	m_MovingTarget->SetPhysicsLinearVelocity(velocity);
	DrawVectorFromHead(m_MovingTarget->GetPhysicsLinearVelocity(), 40, FColor::Emerald);
}

FVector UPhysicsMovement::SlideAlongOnSurface(const FVector& velocity, float deltaTime, float Time, const FVector & InNormal, FHitResult & Hit, bool bHandleImpact)
{
	if (!Hit.bBlockingHit)
	{
		return velocity;
	}

	FVector Normal(InNormal);
	if (IsMovingOnGround())
	{
		// We don't want to be pushed up an unwalkable surface.
		if (Normal.Z > 0.f)
		{
			if (!IsWalkable(Hit))
			{
				Normal = Normal.GetSafeNormal2D();
			}
		}
		else if (Normal.Z < -KINDA_SMALL_NUMBER)
		{
			// Don't push down into the floor when the impact is on the upper portion of the capsule.

			if (m_fGroundDist < 1.9f && m_GroundHitResult.bBlockingHit)
			{
				const FVector FloorNormal = m_GroundHitResult.Normal;
				const bool bFloorOpposedToMovement = (velocity | FloorNormal) < 0.f && (FloorNormal.Z < 1.f - DELTA);
				if (bFloorOpposedToMovement)
				{
					Normal = FloorNormal;
				}

				Normal = Normal.GetSafeNormal2D();
			}
		}
	}

	//base
	float PercentTimeApplied = 0.f;
	const FVector OldHitNormal = Normal;

	FVector SlideDelta = ComputeSlideVector(velocity, Time, Normal, Hit);

	if ((SlideDelta | velocity) > 0.f)
	{
		const FQuat Rotation = UpdatedComponent->GetComponentQuat();
		FVector Location= UpdatedComponent->GetComponentLocation();
		//SetVelocity(SlideDelta, Hit, Time);
		SweepCanMove(SlideDelta, deltaTime,Hit);
		const float FirstHitPercent = Hit.Time;
		PercentTimeApplied = FirstHitPercent;
		if (Hit.IsValidBlockingHit())
		{
			TwoWallAdjust(SlideDelta, Hit, OldHitNormal);
			if (!SlideDelta.IsNearlyZero(1e-3f) && (SlideDelta | velocity) > 0.f)
			{
				SweepCanMove(SlideDelta, deltaTime, Hit);
				const float SecondHitPercent = Hit.Time * (1.f - FirstHitPercent);
				PercentTimeApplied += SecondHitPercent;
			}
		}
		PRINTF("Return1");
		return SlideDelta;
	}
	PRINTF("Return2");
	return SlideDelta;
}

bool UPhysicsMovement::SweepCanMove ( FVector  delta,float deltaTime,FHitResult& OutHit,float offset)
{
	 FVector TraceStart = m_MovingTarget->GetComponentLocation();
	TraceStart.Z += 10.f;
	const FVector TraceEnd = TraceStart + delta*(deltaTime+offset);
	float DeltaSizeSq = (TraceEnd - TraceStart).SizeSquared();				// Recalc here to account for precision loss of float addition
	const FQuat InitialRotationQuat = m_MovingTarget->GetComponentTransform().GetRotation();

	const float MinMovementDistSq = FMath::Square(4.f*KINDA_SMALL_NUMBER);

	if (DeltaSizeSq <= MinMovementDistSq)//너무작으면 스윕 안한다
	{
		if (UpdatedComponent->GetComponentQuat().Equals(InitialRotationQuat, SCENECOMPONENT_QUAT_TOLERANCE))
		{
			return true;
		}
		DeltaSizeSq = 0.f;
	}

	FHitResult BlockingHit(NoInit);
	BlockingHit.bBlockingHit = false;
	BlockingHit.Time = 1.f;
	bool bFilledHitResult = false;
	bool bIncludesOverlapsAtEnd = false;
	bool bRotationOnly = false;
	TArray<FOverlapInfo> PendingOverlaps;
	AActor* const Actor = GetOwner();

	{
		TArray<FHitResult> Hits;
		FVector NewLocation = TraceStart;
		const bool bCollisionEnabled = m_MovingTarget->IsQueryCollisionEnabled();
		if (bCollisionEnabled && (DeltaSizeSq > 0.f))
		{
			UWorld* const MyWorld = GetWorld();

			FComponentQueryParams Params(SCENE_QUERY_STAT(MoveComponent), Actor);
			FCollisionResponseParams ResponseParam;
			m_MovingTarget->InitSweepCollisionParams(Params, ResponseParam);
			bool const bHadBlockingHit = MyWorld->ComponentSweepMulti(Hits, m_MovingTarget, TraceStart, TraceEnd, InitialRotationQuat, Params);
			//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, -1.f, 0.1f, 3.f);
			if (Hits.Num() > 0)
			{
				const float DeltaSize = FMath::Sqrt(DeltaSizeSq);
				for (int32 HitIdx = 0; HitIdx < Hits.Num(); HitIdx++)
				{
					PullBackHit(Hits[HitIdx], TraceStart, TraceEnd, DeltaSize);
				}
			}
			else
			{
				return true;
			}

			int32 FirstNonInitialOverlapIdx = INDEX_NONE;
			if (bHadBlockingHit)
			{
				int32 BlockingHitIndex = INDEX_NONE;
				float BlockingHitNormalDotDelta = BIG_NUMBER;
				for (int32 HitIdx = 0; HitIdx < Hits.Num(); HitIdx++)
				{
					const FHitResult& TestHit = Hits[HitIdx];

					if (TestHit.bBlockingHit)
					{
						if (TestHit.Time == 0.f)
						{
							const float NormalDotDelta = (TestHit.ImpactNormal | delta);
							if (NormalDotDelta < BlockingHitNormalDotDelta)
							{
								BlockingHitNormalDotDelta = NormalDotDelta;
								BlockingHitIndex = HitIdx;
								//각도차가 클수록 내적이 작아진다.
							}
						}
						else if (BlockingHitIndex == INDEX_NONE)
						{
							BlockingHitIndex = HitIdx;
							break;
						}
					}
				}

				if (BlockingHitIndex >= 0)
				{
					BlockingHit = Hits[BlockingHitIndex];
					
					bFilledHitResult = true;
				}
			}

			if (!BlockingHit.bBlockingHit)
			{
				NewLocation = TraceEnd;
			}
			else
			{
				check(bFilledHitResult);
				NewLocation = TraceStart + (BlockingHit.Time * (TraceEnd - TraceStart));

				const FVector ToNewLocation = (NewLocation - TraceStart);
				if (ToNewLocation.SizeSquared() <= MinMovementDistSq)
				{
					NewLocation = TraceStart;
					BlockingHit.Time = 0.f;

					if (FirstNonInitialOverlapIdx != INDEX_NONE)
					{
						const bool bAllowShrinking = false;
						PendingOverlaps.SetNum(FirstNonInitialOverlapIdx, bAllowShrinking);
					}
				}
			}
		}
		else if (DeltaSizeSq > 0.f)
		{
			NewLocation += delta;
			bIncludesOverlapsAtEnd = false;
		}
		m_MovingTarget->SetWorldLocationAndRotationNoPhysics(NewLocation,SelectTargetRotation(deltaTime));
	}

	if (BlockingHit.bBlockingHit && !IsPendingKill())
	{
		check(bFilledHitResult);
		m_MovingTarget->DispatchBlockingHit(*Actor, BlockingHit);
	}
	OutHit = BlockingHit;
	return IsWalkable(BlockingHit);
}

void UPhysicsMovement::PullBackHit(FHitResult & Hit, const FVector & Start, const FVector & End, const float Dist)
{
	const float DesiredTimeBack = FMath::Clamp(0.1f, 0.1f / Dist, 1.f / Dist) + 0.001f;
	Hit.Time = FMath::Clamp(Hit.Time - DesiredTimeBack, 0.f, 1.f);
}

void UPhysicsMovement::UpdateComponentVelocity()
{
}

FVector UPhysicsMovement::ComputeSlideVector(const FVector & Delta, const float Time, const FVector & Normal, const FHitResult & Hit) const
{
	//DrawVectorFromHead(Delta, 100.f, FColor::Cyan);
	FVector Result = Super::ComputeSlideVector(Delta, Time, Normal, Hit);
	DrawVectorFromHead(Result, 100.f, FColor::Red);
	// prevent boosting up slopes
	if (IsFalling())
	{
		Result = HandleSlopeBoosting(Result, Delta, Time, Normal, Hit);
	}

	return Result;
}

FVector UPhysicsMovement::HandleSlopeBoosting(const FVector & SlideResult, const FVector & Delta, const float Time, const FVector & Normal, const FHitResult & Hit) const
{
	FVector Result = SlideResult;

	if (Result.Z > 0.f)
	{
		// Don't move any higher than we originally intended.
		const float ZLimit = Delta.Z * Time;
		if (Result.Z - ZLimit > KINDA_SMALL_NUMBER)
		{
			if (ZLimit > 0.f)
			{
				// Rescale the entire vector (not just the Z component) otherwise we change the direction and likely head right back into the impact.
				const float UpPercent = ZLimit / Result.Z;
				Result *= UpPercent;
			}
			else
			{
				// We were heading down but were going to deflect upwards. Just make the deflection horizontal.
				Result = FVector::ZeroVector;
			}

			// Make remaining portion of original result horizontal and parallel to impact normal.
			const FVector RemainderXY = (SlideResult - Result) * FVector(1.f, 1.f, 0.f);
			const FVector NormalXY = Normal.GetSafeNormal2D();
			const FVector Adjust = Super::ComputeSlideVector(RemainderXY, 1.f, NormalXY, Hit);
			Result += Adjust;
		}
	}

	return Result;;
}

void UPhysicsMovement::DrawVectorFromHead(FVector wantVector, float length, FColor color) const
{
	static int b = 1;
	FVector Start = UpdatedComponent->GetComponentLocation();
	DrawDebugLine(GetWorld(), Start, Start + wantVector.GetSafeNormal()*length, color, false, -1.f, 1, 1.3f);
}

