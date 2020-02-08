// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsMovement.h"
#include "Datas/USB_Macros.h"
#include "Components/PrimitiveComponent.h"

UPhysicsMovement::UPhysicsMovement(const FObjectInitializer& objInit)
{
	m_MovingTarget = nullptr;
	m_DeferredUpdatedMoveComponent = nullptr;
	m_bMovementInProgress = false;
	m_bDeferUpdateMoveComponent = false;
	m_NameLinearVelocityBone = NAME_None;
	m_fJumpZVelocity = 540.f;
	m_fMovingForce = 5000.f;
	m_bOnGround = false;
	m_bPressedJump = false;
	m_bWasJumping = false;
	m_fJumpKeyHoldTime = 0.f;
	m_fJumpForceTimeRemaining = 0.f;
	m_fJumpMaxHoldTime = 0.f;
	m_nJumpMaxCount = 1;
	m_nJumpCurrentCount = 0;
}

void UPhysicsMovement::SetUpdatedComponent(USceneComponent * NewUpdatedComponent)
{
	if (NewUpdatedComponent)
	{
		const APawn* NewPawnOwner = Cast<APawn>(NewUpdatedComponent->GetOwner());
		if (NewPawnOwner == NULL)
		{
			PRINTF("Owner is not Pawn")
			return;
		}

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

	if (UpdatedComponent != OldUpdatedComponent)
	{
		ClearAccumulatedForces();
	}
	if (UpdatedComponent == NULL)
	{
		StopActiveMovement();
	}

	m_MovingTarget->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	m_MovingTarget->SetSimulatePhysics(true);
}

void UPhysicsMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime) || !GetWorld())
	{
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckJumpInput(DeltaTime);

	const FVector InputVector = ConsumeInputVector();
	PRINTF("InputVector : %s", *InputVector.ToString());
	m_Acceleration = ScaleInputAccel(InputVector);
	Velocity = m_Acceleration;
	PerformMovement(DeltaTime);
}
void UPhysicsMovement::PerformMovement(float delta)
{
	ApplyAccumulatedForces(delta);
	UpdateComponentVelocity();
}
void UPhysicsMovement::ApplyAccumulatedForces(float DeltaSeconds)
{
	Velocity += m_PendingImpulseToApply + (m_PendingForceToApply * DeltaSeconds);

	m_PendingImpulseToApply = FVector::ZeroVector;
	m_PendingForceToApply = FVector::ZeroVector;
}


void UPhysicsMovement::UpdateComponentVelocity()
{
	FVector FinalVelocity = Velocity;
	FinalVelocity.Z = m_MovingTarget->GetPhysicsLinearVelocity().Z;

	m_MovingTarget->SetPhysicsLinearVelocity(Velocity, false, m_NameLinearVelocityBone);
}

void UPhysicsMovement::ClearAccumulatedForces()
{
	m_PendingImpulseToApply = FVector::ZeroVector;
	m_PendingForceToApply = FVector::ZeroVector;
	m_PendingLaunchVelocity = FVector::ZeroVector;
}

bool UPhysicsMovement::DoJump()
{
	if (PawnOwner && CanJump())
	{
		// Don't jump if we can't move up/down.
		if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
		{
			Velocity.Z = FMath::Max(Velocity.Z, m_fJumpZVelocity);
			return true;
		}
	}

	return false;
}

bool UPhysicsMovement::CanJump()
{
	bool bCanJump = true;

	bCanJump = IsGround();

	if (bCanJump)
	{
		if (!m_bWasJumping || GetJumpMaxHoldTime() <= 0.0f)
		{
			if (m_nJumpCurrentCount == 0 && IsFalling())
			{
				bCanJump = m_nJumpCurrentCount + 1 < m_nJumpMaxCount;
			}
			else
			{
				bCanJump = m_nJumpCurrentCount < m_nJumpMaxCount;
			}
		}
		else
		{
			const bool bJumpKeyHeld = (m_bPressedJump && m_fJumpKeyHoldTime < GetJumpMaxHoldTime());

			bCanJump = bJumpKeyHeld &&
				((m_nJumpCurrentCount < m_nJumpMaxCount) || (m_bWasJumping && m_nJumpCurrentCount == m_nJumpMaxCount));
		}
	}

	return bCanJump;
}

bool UPhysicsMovement::IsFalling() const
{
	return !IsGround();
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



void UPhysicsMovement::SetWalkableFloorAngle(float InWalkableFloorAngle)
{
	m_fWalkableFloorAngle = FMath::Clamp(InWalkableFloorAngle, 0.f, 90.0f);
	m_fWalkableFloorZ = FMath::Cos(FMath::DegreesToRadians(m_fWalkableFloorAngle));
}

void UPhysicsMovement::SetWalkableFloorZ(float InWalkableFloorZ)
{
	m_fWalkableFloorZ = FMath::Clamp(InWalkableFloorZ, 0.f, 1.f);
	m_fWalkableFloorAngle = FMath::RadiansToDegrees(FMath::Acos(m_fWalkableFloorZ));
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


void UPhysicsMovement::CheckJumpInput(float DeltaTime)
{
	if (m_bPressedJump)
	{
		const bool bFirstJump = m_nJumpCurrentCount == 0;
		if (bFirstJump && IsFalling())
		{
			m_nJumpCurrentCount++;
		}
		const bool bDidJump = CanJump() && DoJump();
		if (bDidJump)
		{
			if (!m_bWasJumping)
			{
				m_nJumpCurrentCount++;
				m_fJumpForceTimeRemaining = GetJumpMaxHoldTime();
				m_OnJumpBP.Broadcast();
				m_OnJump.Broadcast();
			}
		}
		m_bWasJumping = bDidJump;
	}
}

void UPhysicsMovement::ClearJumpInput(float delta)
{
	if (m_bPressedJump)
	{
		m_fJumpKeyHoldTime += delta;

		if (m_fJumpKeyHoldTime >= GetJumpMaxHoldTime())
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

bool UPhysicsMovement::HandlePendingLaunch()
{
	if (!m_PendingLaunchVelocity.IsZero())
	{
		Velocity = m_PendingLaunchVelocity;
		m_PendingLaunchVelocity = FVector::ZeroVector;
		return true;
	}

	return false;
}

float UPhysicsMovement::GetJumpMaxHoldTime() const
{
	return m_fJumpMaxHoldTime;
}


void UPhysicsMovement::ResetJumpState()
{
	m_bPressedJump = false;
	m_bWasJumping = false;
	m_fJumpKeyHoldTime = 0.0f;
	m_fJumpForceTimeRemaining = 0.0f;

	if (!IsFalling())
	{
		m_nJumpCurrentCount = 0;
	}
}

bool UPhysicsMovement::IsJumpProvidingForce() const
{
	if (m_fJumpForceTimeRemaining > 0.0f)
	{
		return true;
	}

	return false;
}


float UPhysicsMovement::GetMaxForce() const
{
	return m_fMovingForce;
}

void UPhysicsMovement::Launch(FVector const & LaunchVel)
{
	m_PendingLaunchVelocity = LaunchVel;
}

void UPhysicsMovement::AddForce(FVector forceWant)
{
	if (forceWant.IsNearlyZero())
	{
		return;
	}

	float Mass = m_MovingTarget->GetMass();
	FVector FinalForce = forceWant;

	if (Mass > SMALL_NUMBER)
	{
		FinalForce += FinalForce / Mass;
	}
	m_PendingForceToApply += FinalForce;
}

void UPhysicsMovement::AddImpulse(FVector impulseWant)
{
	if (impulseWant.IsNearlyZero())
	{
		return;
	}

	float Mass = m_MovingTarget->GetMass();
	FVector FinalImpulse = impulseWant;

	if (Mass > SMALL_NUMBER)
	{
		FinalImpulse = FinalImpulse / Mass;
	}
	m_PendingImpulseToApply += FinalImpulse;
}
