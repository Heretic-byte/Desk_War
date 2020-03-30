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
	m_WalkableSlopeAngle = 65.f;
	m_fBlockMoveTime = 0.f;
	m_fBlockMoveTimer = 0.f;
	m_fAutoRotTime = 0.f;
	m_fAutoRotTimer = 0.f;
	m_fAdditionalTraceMultipleLength = 1.f;
	m_fForwardWallCheckCast = 50.f;
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
}


void UPhysicsMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickCastGround();
	TickCastFlipCheck();
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
		return m_AutoRotateRot;
	}

	if (m_Acceleration.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		return m_MovingTarget->GetComponentRotation();
	}
	return m_Acceleration.GetSafeNormal().Rotation();
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
	const FVector Delta = FVector(Velocity.X, Velocity.Y, 0.f);
	FHitResult Hit(1.f);
	FVector RampVector = ComputeGroundMovementDelta(Delta, m_GroundHitResult);
	SetVelocity(Velocity,Hit);

	//float LastMoveTimeSlice = delta;

	//if (Hit.bStartPenetrating)	// SafeMoveUpdatedComponent를 통해 이동한 결과가 다른 콜리젼이랑 겹친 상황이라면
	//{
	//	// Allow this hit to be used as an impact we can deflect off, otherwise we do nothing the rest of the update and appear to hitch.
	//	//SlideAlongSurface(Delta, 1.f, Hit.Normal, Hit, true);

	//	if (Hit.bStartPenetrating)
	//	{
	//		//OnCharacterStuckInGeometry(&Hit);	// 끼었을 때 bJustTeleported = true 아직 복사 안함
	//	}
	//}
	//else if (Hit.IsValidBlockingHit())	// 일반적인 BlockingHit일 때
	//{
	//	// We impacted something (most likely another ramp, but possibly a barrier).
	//	float PercentTimeApplied = Hit.Time;//시작에서 끝까지 어디서 부딫혔는지의 퍼센트
	//	if ((Hit.Time > 0.f) && (Hit.Normal.Z > KINDA_SMALL_NUMBER) && IsWalkable(Hit))	// Hit된 지면이 경사면으로 추정된다면 경사면을 따라 걸음
	//	{
	//		// Another walkable ramp.
	//		const float InitialPercentRemaining = 1.f - PercentTimeApplied;
	//		RampVector = ComputeGroundMovementDelta(Delta * InitialPercentRemaining, Hit);
	//		LastMoveTimeSlice = InitialPercentRemaining * LastMoveTimeSlice;
	//		SetVelocity(RampVector, Hit);
	//		const float SecondHitPercent = Hit.Time * InitialPercentRemaining;
	//		PercentTimeApplied = FMath::Clamp(PercentTimeApplied + SecondHitPercent, 0.f, 1.f);
	//	}

	//	//경사면을 오르고 또 부딫혔는지 확인

	//	if (Hit.IsValidBlockingHit())	// Hit 여부에 따라서 어떻게 이동할지 정함
	//	{
	//		SlideAlongSurface(Delta, 1.f - PercentTimeApplied, Hit.Normal, Hit, true);
	//	}
	//}
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
	return !IsGround();
}

void UPhysicsMovement::TickCheckCanMoveForward()
{
	if (m_vInputNormal.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		//m_bIsWallBlocking = false;
		return ;
	}
	FVector TraceStart = m_MovingTarget->GetComponentLocation();
	FVector TraceEnd = TraceStart + (m_fForwardWallCheckCast*m_fAdditionalTraceMultipleLength*m_vInputNormal);//groundoffset is minus

#if WITH_EDITOR
	if (m_bDebugShowForwardCast)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor(255, 255, 120), false, -1.f, 0.1f);
	}
#endif

	FCollisionQueryParams QueryParam;
	AddIgnoreActorsToQuery(QueryParam);
	FHitResult HitResult;

	if (GetWorld()->SweepSingleByObjectType(HitResult, TraceStart, TraceEnd, FQuat::Identity, ECollisionChannel::ECC_WorldStatic, FCollisionShape::MakeSphere(1.f), QueryParam))
	{
		//m_bIsWallBlocking = true;
		m_Acceleration = FVector::ZeroVector;
		return ;
	}
	else
	{
		//m_bIsWallBlocking = false;
		return ;
	}
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

	if (!m_bOnGround)//간혹 실패할경우 각도차이인지 확인
	{
		m_GroundHitResult.Reset(1.f, false);

		m_bOnGround = GetWorld()->SweepSingleByChannel(m_GroundHitResult,TraceStart,TraceEnd,FQuat::Identity,ECollisionChannel::ECC_Visibility,BoxShape,QueryParam);
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


bool UPhysicsMovement::IsGround() const
{
	return m_bOnGround;
}

void UPhysicsMovement::SetAccelerationByDir(const FVector inputPure)
{
	m_vInputNormal = inputPure.GetClampedToMaxSize(1.f);

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
	/*	FVector CurrentV = m_MovingTarget->GetPhysicsLinearVelocity();
		CurrentV.Z = FMath::Max(m_fJumpZVelocity, CurrentV.Z);
		
		float CurrentTargetMass = m_MovingTarget->GetBodyInstance()->GetBodyMass();
		float TargetTailMassRate = CurrentTargetMass / m_fInitHeadMass;

		float CurrentTailMass = m_MovingTargetTail->GetBodyInstance()->GetBodyMass();
		float TargetMassRate = CurrentTailMass / m_fInitHeadMass;


		m_MovingTarget->SetPhysicsLinearVelocity(CurrentV * TargetMassRate);
		m_MovingTargetTail->SetPhysicsLinearVelocity(CurrentV * TargetTailMassRate);*/

		Velocity.Z = FMath::Max(Velocity.Z, m_fJumpZVelocity);

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
		PRINTF("HEre");
		return false;
	}
	PRINTF("HEre2");
	float TestWalkableZ = m_WalkableSlopeHeight;

	// See if this component overrides the walkable floor z.
	const UPrimitiveComponent* HitComponent = Hit.Component.Get();
	if (HitComponent)
	{
		const FWalkableSlopeOverride& SlopeOverride = HitComponent->GetWalkableSlopeOverride();
		TestWalkableZ = SlopeOverride.ModifyWalkableFloorZ(TestWalkableZ);
		PRINTF("HEre3");
	}

	PRINTF("Hit nZ : %f, Test Z : %f", Hit.ImpactNormal.Z, TestWalkableZ);
	if (Hit.ImpactNormal.Z < TestWalkableZ)
	{
		PRINTF("HEre4");
		return false;
	}
	PRINTF("HEre5");
	return true;
}

void UPhysicsMovement::SetVelocity(FVector& velocity,FHitResult & sweep)
{
	if (!m_MovingTarget)
	{
		return;
	}
	FVector CurrentV = m_MovingTarget->GetPhysicsLinearVelocity();

	velocity.Z = CurrentV.Z;
	m_MovingTarget->SetPhysicsLinearVelocity(velocity);
	//PRINTF("Velocity : %s", *m_MovingTarget->GetPhysicsLinearVelocity().ToString());


	//Velocity.Z = 0.f;
}

float UPhysicsMovement::SlideAlongSurface(const FVector & Delta, float Time, const FVector & InNormal, FHitResult & Hit, bool bHandleImpact)
{
	if (!Hit.bBlockingHit)
	{
		return 0.f;
	}
	PRINTF("SlideAlongSurface");

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
			/*if (CurrentFloor.FloorDist < MIN_FLOOR_DIST && CurrentFloor.bBlockingHit)
			{
				const FVector FloorNormal = CurrentFloor.HitResult.Normal;
				const bool bFloorOpposedToMovement = (Delta | FloorNormal) < 0.f && (FloorNormal.Z < 1.f - DELTA);
				if (bFloorOpposedToMovement)
				{
					Normal = FloorNormal;
				}

				Normal = Normal.GetSafeNormal2D();
			}*/
		}
	}

	//base
	if (!Hit.bBlockingHit)
	{
		return 0.f;
	}

	float PercentTimeApplied = 0.f;
	const FVector OldHitNormal = Normal;

	FVector SlideDelta = ComputeSlideVector(Delta, Time, Normal, Hit);

	if ((SlideDelta | Delta) > 0.f)
	{
		const FQuat Rotation = UpdatedComponent->GetComponentQuat();
		SafeMoveUpdatedComponent(SlideDelta, Rotation, true, Hit);

		const float FirstHitPercent = Hit.Time;
		PercentTimeApplied = FirstHitPercent;
		if (Hit.IsValidBlockingHit())
		{
			// Notify first impact
			if (bHandleImpact)
			{
				HandleImpact(Hit, FirstHitPercent * Time, SlideDelta);
			}

			// Compute new slide normal when hitting multiple surfaces.
			TwoWallAdjust(SlideDelta, Hit, OldHitNormal);

			// Only proceed if the new direction is of significant length and not in reverse of original attempted move.
			if (!SlideDelta.IsNearlyZero(1e-3f) && (SlideDelta | Delta) > 0.f)
			{
				// Perform second move
				SafeMoveUpdatedComponent(SlideDelta, Rotation, true, Hit);
				const float SecondHitPercent = Hit.Time * (1.f - FirstHitPercent);
				PercentTimeApplied += SecondHitPercent;

				// Notify second impact
				if (bHandleImpact && Hit.bBlockingHit)
				{
					HandleImpact(Hit, SecondHitPercent * Time, SlideDelta);
				}
			}
		}

		return FMath::Clamp(PercentTimeApplied, 0.f, 1.f);
	}

	return 0.f;
}
