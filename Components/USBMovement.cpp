#include "USBMovement.h"
#include "Actors/USB_PlayerPawn.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"



UUSBMovement::UUSBMovement(const FObjectInitializer& objInit):Super(objInit)
{
	m_fJumpHeight = 20.f;
	m_fAutoMoveTimeWant = -1.f;
	m_fAutoMoveTimer = -1.f;
	m_fInitHeadMass = 2.5f;
	m_PlayerPawn = nullptr;
	m_NameSweepProfileName = "USBActor";
}

void UUSBMovement::BeginPlay()
{
	Super::BeginPlay();
	m_fInitHeadMass = m_MovingTarget->GetBodyInstance()->GetBodyMass();
}

void UUSBMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (m_fAutoMoveTimer < m_fAutoMoveTimeWant)
	{
		m_fAutoMoveTimer += DeltaTime;
		RequestPathMove(m_fAutoMoveInput);

		if (m_fAutoMoveTimer >= m_fAutoMoveTimeWant)
		{
			m_PlayerPawn->FailConnection(nullptr,nullptr,EFailConnectionReason::PortNotFoundTimeEnd); 
		}
	}
}



void UUSBMovement::InitUSBUpdateComponent(AUSB_PlayerPawn* playerPawn, UPhysicsSkMeshComponent * head, UPhysicsSkMeshComponent * tail)
{
	SetUSBUpdateComponent(head, tail,false);
	m_PlayerPawn = playerPawn;
}

void UUSBMovement::SetUSBUpdateComponent(UPhysicsSkMeshComponent * head, UPhysicsSkMeshComponent * tail, bool bRemoveTraceOld)
{
	SetMovingComponent(head,bRemoveTraceOld);
	m_MovingTargetTail = tail;
	m_TailShape = MakeMovingTargetBox(m_MovingTargetTail);
}

void UUSBMovement::AddForce(FVector forceWant)
{
	Super::AddForce(forceWant);
	m_MovingTargetTail->AddForce(forceWant);
}

void UUSBMovement::AddImpulse(FVector impulseWant)
{
	Super::AddImpulse(impulseWant);
	m_MovingTargetTail->AddImpulse(impulseWant);
}

void UUSBMovement::RequestConnectChargeMove(const FVector & normalHorizon, float timeWant)
{
	m_fAutoMoveInput = normalHorizon;
	m_fAutoMoveTimeWant = timeWant;
	m_fAutoMoveTimer = 0;
}

void UUSBMovement::RequestAirConnectChargeMove(FRotator portRot, const FVector & normalHorizon, float timeWant)
{
	m_OnAirTargetRot = portRot;
	RequestConnectChargeMove(normalHorizon, timeWant);
}

void UUSBMovement::StopUSBMove()
{
	m_fAutoMoveTimeWant = 0.f;
	m_fAutoMoveTimer = 0.f;
	m_fAutoMoveInput = FVector::ZeroVector;
	PRINTF("StopAutoMove");

	StopMovementImmediately();
	ClearAirRotation();
}
void UUSBMovement::ClearAirRotation()
{
	m_OnAirTargetRot = FRotator::ZeroRotator;
}
bool UUSBMovement::DoJump()
{
 	if (CanJump())
	{
		m_nJumpCurrentCount++;
		PRINTF("JumpCount:%d", m_nJumpCurrentCount);

		if (m_nJumpCurrentCount == 1)
		{
			m_OnJumpBP.Broadcast();
		}
		else
		{
			m_OnJumpSeveralBP.Broadcast(m_nJumpCurrentCount);
		}

		FVector CurrentV = m_MovingTarget->GetPhysicsLinearVelocity();
		float ZVelo= FMath::Sqrt(-2.f *GetGravityZ() * m_fJumpHeight);
		CurrentV.Z = FMath::Max(ZVelo, CurrentV.Z);

		for (auto Phy : m_PlayerPawn->GetPhysicsAry())
		{
			if (Phy == m_PlayerPawn->GetHead())
			{
				float Weight = ((m_PlayerPawn->GetTail()->GetMass() / m_PlayerPawn->GetHead()->GetMass()) / 10.f + 1.f);

				Phy->SetPhysicsLinearVelocity(CurrentV*Weight);
			}
			else
			{
				Phy->SetPhysicsLinearVelocity(CurrentV);
			}
		}

		return true;
	}
	return false;
}

void UUSBMovement::TickCastGround()
{
	Super::TickCastGround();

	m_TailGroundHitResult.Reset(1.f, false);
	FCollisionShape BoxShape = m_TailShape; //MakeMovingTargetBox();
	FVector TraceStart = m_MovingTargetTail->GetComponentLocation();

	FVector TraceEnd = TraceStart;
	TraceEnd.Z -= (BoxShape.GetExtent().Z)*2.f;

	FCollisionQueryParams QueryParam;
	AddIgnoreActorsToQuery(QueryParam);

	ECollisionChannel Channel = m_MovingTargetTail->GetCollisionObjectType();

	FVector Ex = BoxShape.GetExtent();
	Ex *= 0.3f;
	BoxShape.SetBox(Ex);

	m_bTailOnGround = GetWorld()->SweepSingleByChannel(m_TailGroundHitResult, TraceStart, TraceEnd, FQuat(FVector(0.f, 0.f, -1.f), PI * 0.25f), Channel, BoxShape, QueryParam);
	m_fTailGroundDist = (TraceStart.Z - m_TailGroundHitResult.Location.Z);

	if (!m_bTailOnGround)//간혹 실패할경우 각도차이인지 확인
	{
		m_TailGroundHitResult.Reset(1.f, false);

		m_bTailOnGround = GetWorld()->SweepSingleByChannel(m_TailGroundHitResult, TraceStart, TraceEnd, FQuat::Identity, Channel, BoxShape, QueryParam);

		m_fTailGroundDist = (TraceStart.Z - m_TailGroundHitResult.Location.Z);
	}

	if (m_bTailOnGround && -100 > m_TailGroundHitResult.Component.Get()->GetPhysicsLinearVelocity().Z)//공중에서 떨어지고 있는 오브젝트는 디딤체크안함
	{
		m_bTailOnGround = false;
		m_TailGroundHitResult.Reset(1.f, false);
	}

	if (m_bTailIsFalling == m_bTailOnGround)
	{
		m_bTailIsFalling = !m_bTailOnGround;

		if (!m_bTailIsFalling)
		{
			TailLanding();
		}
		else
		{
			m_fTailFallStartZ = m_MovingTargetTail->GetComponentLocation().Z;
		}
	}
}

void UUSBMovement::TailLanding()
{
	if ((m_fTailFallStartZ - m_MovingTargetTail->GetComponentLocation().Z) >= m_fMinLandHeight)
	{
		PRINTF("TailLanding:max was :%f", m_fTailFallStartZ - m_MovingTargetTail->GetComponentLocation().Z);
		m_OnTailLandingBP.Broadcast(m_TailGroundHitResult.ImpactPoint);
	}

	m_fTailFallStartZ = 0.f;
}

void UUSBMovement::CollectHeight()
{
	if (IsFalling())
	{
		m_fFallStartZ = FMath::Max<float>(m_fFallStartZ, m_MovingTarget->GetComponentLocation().Z);
		m_fTailFallStartZ = FMath::Max<float>(m_fTailFallStartZ, m_MovingTargetTail->GetComponentLocation().Z);
	}
}

