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
	TickCheckCanMoveForward();
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

	
	if(!m_bIsWallBlocking)
		TickMovement(DeltaTime);
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
	if (m_bBlockMove && !m_bAutoMove)
	{
		//only block move
		return;
	}

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
		PRINTF("NoInput");
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
		PRINTF("Blocked");
		//m_bIsWallBlocking = true;
		m_Acceleration = FVector::ZeroVector;
		return ;
	}
	else
	{
		PRINTF("NotBlocked");
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
