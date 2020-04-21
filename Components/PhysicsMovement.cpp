// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsMovement.h"
#include "Datas/USB_Macros.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "PhysicsPublic.h"
#include "Engine/World.h"

UPhysicsMovement::UPhysicsMovement(const FObjectInitializer& objInit)
{
	m_NameSweepProfileName = "PhysicsActor";
	m_bShowDebug = false;
	m_fMaxTimeStep = 33.f;
	m_fGroundFriction = 2.f;
	m_fMaxSpeed = 370.f;
	m_fMaxBrakingDeceleration = 400.f;
	m_fMinAnalogSpeed = 10.f;
	m_MovingTarget = nullptr;
	m_fJumpZVelocity = 540.f;
	m_fMovingForce = 600.f;
	m_bOnGround = false;
	m_bPressedJump = false;
	m_RotationRate = FRotator(180.f, 180.f, 500.f);
	m_fAirControl = 0.5f;
	m_fWalkableSlopeAngle = 65.f;
	bUseAccelerationForPaths = true;
	GetNavAgentPropertiesRef().bCanJump = true;
	GetNavAgentPropertiesRef().bCanWalk = true;
	m_bIsFalling = false;
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

	SetWalkableFloorAngle(m_fWalkableSlopeAngle);

	//TickCastGround();
	m_bIsFalling = !m_bOnGround;

	
}


void UPhysicsMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CheckJumpInput(DeltaTime);
	TickCastGround();
	SetAccel(DeltaTime);
	m_fAnalogInputModifier = ComputeAnalogInputModifier();
	TickRotate(SelectTargetRotation(DeltaTime), DeltaTime);//얘다

	ClearJumpInput(DeltaTime);
}

void UPhysicsMovement::PhysSceneStep(FPhysScene * PhysScene, float DeltaTime)
{
	if (!PawnOwner || !UpdatedComponent || !GetWorld())
	{
		return;
	}
	CalcVelocity(DeltaTime, m_fGroundFriction);

	if (m_Acceleration.SizeSquared2D() < 1)//maybe square better?
	{
		return;
	}
	TickMovement(DeltaTime);

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



	UMovementComponent::SetUpdatedComponent(NewUpdatedComponent);
	PawnOwner = NewUpdatedComponent ? CastChecked<APawn>(NewUpdatedComponent->GetOwner()) : NULL;

	if (!m_MovingTarget->IsSimulatingPhysics())
	{
		m_MovingTarget->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		m_MovingTarget->SetSimulatePhysics(true);
	}


}

void UPhysicsMovement::SetMovingComponent(USceneComponent* NewUpdatedComponent, bool bRemoveIgnoreActorOld)
{
	auto* OldTarget = PawnOwner;
	if (!OldTarget)
	{
		SetUpdatedComponent(NewUpdatedComponent);
		return;
	}

	if (!PawnOwner)
	{
		//set fail
		return;
	}

	if (OldTarget != PawnOwner)
	{

		if (bRemoveIgnoreActorOld)
		{
			RemoveIgnoreTraceActor(OldTarget);
		}
		AddIgnoreTraceActor(PawnOwner);
	}
	SetUpdatedComponent(NewUpdatedComponent);

	m_Shape = MakeMovingTargetBox();
}

FRotator UPhysicsMovement::SelectTargetRotation(float delta)
{
	if (m_Acceleration.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		return m_MovingTarget->GetComponentRotation();
	}

	FRotator AccelRot = m_Acceleration.GetSafeNormal().Rotation();

	if (!IsMovingOnGround())
	{
		m_OnAirTargetRot.Yaw = AccelRot.Yaw;
		return m_OnAirTargetRot;
	}
	m_OnGroundRampRot.Yaw = AccelRot.Yaw;
	return m_OnGroundRampRot;
}

bool UPhysicsMovement::SetAccel(float DeltaTime)
{
	FVector InputDir;

	InputDir = ConsumeInputVector();

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


void UPhysicsMovement::TickMovement(float delta)
{
	const FVector Delta = FVector(Velocity.X, Velocity.Y, 0.f);// delta;
	FHitResult Hit(1.f);
	FVector RampVector = ComputeGroundMovementDelta(Delta, m_GroundHitResult);
	FVector ResultVector;
	if (SweepCanMove(RampVector, delta, Hit))
	{
		SetVelocity(RampVector, Hit);
		m_OnGroundRampRot = RampVector.GetSafeNormal().Rotation();
		return;
	}

	if (m_bTwoWallHit)
	{
		return;
	}

	if (Hit.bStartPenetrating)
	{
		FVector Loc = m_MovingTarget->GetComponentLocation();
		FVector HitPoint = Hit.ImpactPoint;
		HitPoint.Z = Loc.Z;
		FVector HitNormal = (HitPoint - Loc).GetSafeNormal();

		float Dot = m_InputNormal | HitNormal;

		ResultVector = Dot >= 0 ? SlideAlongOnSurface(Delta, delta, 1.f, Hit.Normal, Hit, true) : Delta.Size() * m_InputNormal;//delta?
	}
	else if (Hit.IsValidBlockingHit())	//일반적인 BlockingHit일 때
	{
		float LastMoveTimeSlice = delta;
		float PercentTimeApplied = Hit.Time;//시작에서 끝까지 어디서 부딫혔는지의 퍼센트
		if ((Hit.Time > 0.f) && (Hit.Normal.Z > KINDA_SMALL_NUMBER) && IsWalkable(Hit))	//Hit된 지면이 경사면으로 추정된다면 경사면을 따라 걸음
		{
			const float InitialPercentRemaining = 1.f - PercentTimeApplied;
			RampVector = ComputeGroundMovementDelta(Delta * InitialPercentRemaining, Hit);
			LastMoveTimeSlice = InitialPercentRemaining * LastMoveTimeSlice;

			ResultVector = RampVector;

			const float SecondHitPercent = Hit.Time * InitialPercentRemaining;
			PercentTimeApplied = FMath::Clamp(PercentTimeApplied + SecondHitPercent, 0.f, 1.f);
		}
		if (Hit.IsValidBlockingHit())
		{
			ResultVector = SlideAlongOnSurface(Delta, delta, 1.f - PercentTimeApplied, Hit.Normal, Hit, true);
		}
	}
	m_OnGroundRampRot = ResultVector.GetSafeNormal().Rotation();
	SetVelocity(ResultVector, Hit);

}
void UPhysicsMovement::TickRotate(const FRotator rotateWant, float delta)
{
	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
	CurrentRotation.DiagnosticCheckNaN(TEXT("UPhysicsMovement::Rotate(): CurrentRotation"));

	FRotator DeltaRot = GetDeltaRotation(delta);//돌릴 값
	DeltaRot.DiagnosticCheckNaN(TEXT("UPhysicsMovement::Rotate(): GetDeltaRotation"));

	FRotator DesiredRotation = rotateWant;

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
	queryParam.AddIgnoredActors(m_AryTraceIgnoreActors);
}

bool UPhysicsMovement::IsFalling() const
{
	return !IsMovingOnGround();
}


void UPhysicsMovement::TickCastGround()
{
	m_GroundHitResult.Reset(1.f, false);
	FCollisionShape BoxShape = m_Shape; //MakeMovingTargetBox();
	FVector TraceStart = m_MovingTarget->GetComponentLocation();

	FVector TraceEnd = TraceStart;
	TraceEnd.Z -= (BoxShape.GetExtent().Z)*2.f;

	FCollisionQueryParams QueryParam;
	AddIgnoreActorsToQuery(QueryParam);

	ECollisionChannel Channel = UpdatedComponent->GetCollisionObjectType();

	FVector Ex = BoxShape.GetExtent();
	Ex *= 0.3f;
	BoxShape.SetBox(Ex);

	m_bOnGround = GetWorld()->SweepSingleByChannel(m_GroundHitResult, TraceStart, TraceEnd, FQuat(FVector(0.f, 0.f, -1.f), PI * 0.25f), Channel, BoxShape, QueryParam);
	m_fGroundDist = (TraceStart.Z - m_GroundHitResult.Location.Z);

	if (!m_bOnGround)//간혹 실패할경우 각도차이인지 확인
	{
		m_GroundHitResult.Reset(1.f, false);

		m_bOnGround = GetWorld()->SweepSingleByChannel(m_GroundHitResult, TraceStart, TraceEnd, FQuat::Identity, Channel, BoxShape, QueryParam);

		m_fGroundDist = (TraceStart.Z - m_GroundHitResult.Location.Z);
	}

	if (m_bOnGround && -100 > m_GroundHitResult.Component.Get()->GetPhysicsLinearVelocity().Z)//공중에서 떨어지고 있는 오브젝트는 디딤체크안함
	{
		m_bOnGround = false;
		m_GroundHitResult.Reset(1.f, false);
	}

	if (m_bIsFalling == m_bOnGround)
	{
		m_bIsFalling = !m_bOnGround;

		if (!m_bIsFalling)
		{
			Landing();
		}
	}
}


void UPhysicsMovement::SetWalkableFloorAngle(float InWalkableFloorAngle)
{
	m_fWalkableSlopeAngle = FMath::Clamp(InWalkableFloorAngle, 0.f, 90.0f);
	m_fWalkableSlopeHeight = FMath::Cos(FMath::DegreesToRadians(m_fWalkableSlopeAngle));
}

void UPhysicsMovement::SetAccelerationByDir(const FVector inputPure)
{
	m_InputNormal = inputPure.GetSafeNormal2D();

	m_Acceleration = GetMaxForce() *m_InputNormal;
}


void UPhysicsMovement::Jump()
{
	m_bPressedJump = true;
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
}

FVector UPhysicsMovement::GetVelocity()
{
	if (!m_MovingTarget)
	{
		return FVector::ZeroVector;
	}

	FVector Normal= m_MovingTarget->GetPhysicsLinearVelocity().GetSafeNormal2D();

	return UKismetMathLibrary::InverseTransformDirection(m_MovingTarget->GetComponentTransform(),Normal);
}

void UPhysicsMovement::CheckJumpInput(float DeltaTime)
{
	if (m_bPressedJump)
	{
		const bool bDidJump = DoJump();
		if (bDidJump)
		{
			m_bPressedJump = false;
		}
	}
}

bool UPhysicsMovement::DoJump()
{
	if (CanJump())
	{
		FVector CurrentV = m_MovingTarget->GetPhysicsLinearVelocity();
		CurrentV.Z = FMath::Max(m_fJumpZVelocity, CurrentV.Z);

		m_MovingTarget->SetPhysicsLinearVelocity(CurrentV);

		m_nJumpCurrentCount++;
		PRINTF("JumpCount:%d", m_nJumpCurrentCount);

		return true;
	}

	return false;
}

bool UPhysicsMovement::CanJump()
{
	if (m_nJumpCurrentCount >= m_nJumpMaxCount)
	{
		return false;
	}
	return true;
}

void UPhysicsMovement::ClearJumpInput(float delta)
{
	if (m_bPressedJump)
	{
		m_bPressedJump = false;
	}

	bool Walkable = IsWalkable(m_GroundHitResult);
	if (m_WasWalkable != Walkable)
	{
		m_WasWalkable = Walkable;

		if (m_WasWalkable)
		{
			ResetJumpState();
		}
	}
	else if (Walkable&&!IsFalling() && m_MovingTarget->GetPhysicsLinearVelocity().Z <10.f)
	{
		ResetJumpState();
	}
}

void UPhysicsMovement::ResetJumpState()
{
	m_bPressedJump = false;
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
	if (!Hit.bBlockingHit)
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
		if (HitComponent->IsSimulatingPhysics())
		{
			return true;
		}

		const FWalkableSlopeOverride& SlopeOverride = HitComponent->GetWalkableSlopeOverride();
		TestWalkableZ = SlopeOverride.ModifyWalkableFloorZ(TestWalkableZ);
	}
	// Can't walk on this surface if it is too steep.
	if (Hit.ImpactNormal.Z < TestWalkableZ)
	{
		return false;
	}
	return true;
}

void UPhysicsMovement::SetVelocity(FVector& velocity, FHitResult & sweep)
{
	if (!m_MovingTarget)
	{
		return;
	}
	FVector CurrentV = m_MovingTarget->GetPhysicsLinearVelocity();

	if (!IsMovingOnGround())
	{
		velocity *= m_fAirControl;
	}

	velocity.Z = CurrentV.Z;
	m_MovingTarget->SetPhysicsLinearVelocity(velocity);
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
		FVector Location = UpdatedComponent->GetComponentLocation();
		SweepCanMove(SlideDelta, deltaTime, Hit);
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
		return SlideDelta;
	}
	return SlideDelta;
}

bool UPhysicsMovement::SweepCanMove(FVector  delta, float deltaTime, FHitResult& OutHit)
{
	const float MinMovementDistSq = FMath::Square(4.f*KINDA_SMALL_NUMBER);

	const FQuat InitialRotationQuat = m_MovingTarget->GetComponentTransform().GetRotation();

	FRotator InitRot = m_MovingTarget->GetComponentRotation();


	m_bTwoWallHit = false;

	FCollisionShape Shape = m_Shape;
	FVector Ex = Shape.GetExtent();

	float ExHeight = Ex.Z;
	Ex.Z *= 1.2f;
	Ex.X *= 1.2f;
	Ex.Y *= 1.2f;

	Shape.SetBox(Ex);

	FVector TraceStart = m_MovingTarget->GetComponentLocation();
	TraceStart.Z += ExHeight * 0.3f;
	FVector TraceEnd = TraceStart +(m_InputNormal* delta.Size()*deltaTime);

	float DeltaSizeSq = (TraceEnd - TraceStart).SizeSquared();

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
	bool IsSimul = false;
	TArray<FHitResult> Hits;
	FVector NewLocation = TraceStart;
	TArray<TEnumAsByte<	EObjectTypeQuery> >  ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery2);
	if (DeltaSizeSq > 0.f)//여기서 현재 컴플렉스 콜리전 false 상태이다,
	{
		//m_AryTraceIgnoreActors.Add(m_GroundHitResult.GetActor());
		bool const bHadBlockingHit=UKismetSystemLibrary::BoxTraceMultiForObjects(GetWorld(),
			TraceStart,TraceEnd,Shape.GetBox(),
			InitRot, ObjectTypes,false,m_AryTraceIgnoreActors,
			m_bShowDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,Hits,true,FLinearColor::Green,FLinearColor::Red,1.f);
		//m_AryTraceIgnoreActors.Remove(m_GroundHitResult.GetActor());
		if (Hits.Num() > 0)
		{
			const float DeltaSize = FMath::Sqrt(DeltaSizeSq);

			for (int32 HitIdx = 0; HitIdx < Hits.Num(); HitIdx++)
			{
				PullBackHit(Hits[HitIdx], TraceStart, TraceEnd, DeltaSize);
			}

			if (Hits.Num() > 1)
			{
				m_bTwoWallHit = true;
			}
		}
		else
		{
			return true;
		}

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
						if ( NormalDotDelta < 300 && NormalDotDelta>100)//Hits.Num() == 1 &&
						{
							//코너 끼임 방지임 이게
							return true;
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
				IsSimul = BlockingHit.GetComponent()->IsSimulatingPhysics();

				if (IsSimul)
				{
					return true;
				}

				bFilledHitResult = true;
			}
		}
		else
		{
			return true;
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
			}
		}
	}

	OutHit = BlockingHit;

	if (IsSimul)
	{
		return true;
	}

	if (BlockingHit.bBlockingHit)
	{
		m_MovingTarget->SetWorldLocationAndRotationNoPhysics(NewLocation, SelectTargetRotation(deltaTime));
		//m_MovingTarget->SetWorldLocation(NewLocation,false,nullptr,ETeleportType::TeleportPhysics);
		if (!IsPendingKill())
		{
			check(bFilledHitResult);
			m_MovingTarget->DispatchBlockingHit(*PawnOwner, BlockingHit);
		}
	}

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
	if (!m_bShowDebug)
	{
		return;
	}
	FVector Start = UpdatedComponent->GetComponentLocation();
	DrawDebugLine(GetWorld(), Start, Start + wantVector.GetSafeNormal()*length, color, false, -1.f, 1, 1.3f);
}

UPrimitiveComponent * UPhysicsMovement::GetMovingTargetComponent() const
{
	return m_MovingTarget;
}

void UPhysicsMovement::ShowVelocityAccel()
{
	PRINTF("Velo:%s,:%f, Accel:%s,:%f", *Velocity.ToString(), Velocity.Size(), *m_Acceleration.ToString(), m_Acceleration.Size());
	PRINTF("Physcs: %s,%f", *m_MovingTarget->GetPhysicsLinearVelocity().ToString(), m_MovingTarget->GetPhysicsLinearVelocity().Size())
}

void UPhysicsMovement::StopActiveMovement()
{
	Super::StopActiveMovement();

	if (!m_MovingTarget)
	{
		return;
	}

	Velocity = FVector::ZeroVector;
	m_Acceleration = FVector::ZeroVector;
	m_MovingTarget->SetPhysicsLinearVelocity(FVector::ZeroVector);
}

void UPhysicsMovement::AddIgnoreTraceActor(AActor * actorWant)
{
	m_AryTraceIgnoreActors.Add(actorWant);
}

void UPhysicsMovement::RemoveIgnoreTraceActor(AActor * actorWant)
{
	m_AryTraceIgnoreActors.Remove(actorWant);
}

void UPhysicsMovement::Landing()
{
	PRINTF("Landing");
	if (IsWalkable(m_GroundHitResult))
	{
		ResetJumpState();
	}
}

FCollisionShape UPhysicsMovement::MakeMovingTargetBox()
{
	FCollisionShape BoxShape = FCollisionShape::MakeBox(m_MovingTarget->GetBodyInstance()->GetBodyBounds().GetExtent());

	if (BoxShape.Box.HalfExtentX < BoxShape.Box.HalfExtentY)
	{
		float Swap = BoxShape.Box.HalfExtentX;

		BoxShape.Box.HalfExtentX = BoxShape.Box.HalfExtentY;

		BoxShape.Box.HalfExtentY = Swap;
	}

	return BoxShape;
}


