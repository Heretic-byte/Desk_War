// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsMovement.h"

#include "Kismet/KismetMathLibrary.h"

#include "Components/PrimitiveComponent.h"

#include "DrawDebugHelpers.h"

#include "Datas/USB_Macros.h"


UPhysicsMovement::UPhysicsMovement()
{
	PrimaryComponentTick.bCanEverTick = true;
	m_bShowDebug = false;
}
void UPhysicsMovement::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (!m_UpdateComponent)
	{
		PRINTF("Physics Move Dosnt have a UpdateCompon");
		SetComponentTickEnabled(false);
		return;
	}
}

bool UPhysicsMovement::GetIsGround() const
{
	return m_bIsGround;
}

void UPhysicsMovement::DoJump()
{

}

FVector UPhysicsMovement::GetVelocity() const
{
	FVector Velo = m_UpdateComponent->GetComponentVelocity();

	ShowDebugVector(Velo,35.f,FColor::Blue);

	return Velo;

}

void UPhysicsMovement::SetUpdatedComponent(USceneComponent * NewUpdatedComponent)
{

	m_UpdateComponent = Cast<UPrimitiveComponent>(NewUpdatedComponent);
	if (!m_UpdateComponent)
	{
		PRINTF("UpdateComponent SHould be Primirive !");
		return;
	}
	Super::SetUpdatedComponent(NewUpdatedComponent);
	m_UpdateComponent->SetSimulatePhysics(true);
	//m_UpdateComponent->BodyInstance.bLockXRotation = true;
	//m_UpdateComponent->BodyInstance.bLockYRotation = true;
	//m_UpdateComponent->BodyInstance.bLockZRotation = false;

	m_UpdateComponent->SetLinearDamping(m_fLinearDamping);
	m_UpdateComponent->SetAngularDamping(m_fAngularDamping);
	m_UpdateComponent->GetBodyInstance()->UpdateDampingProperties();
}

// Called when the game starts



// Called every frame
void UPhysicsMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	m_CurrentUpdateTargetForward = m_UpdateComponent->GetForwardVector();
	m_Velocity = GetVelocity();

	const FVector InputVector = ConsumeInputVector().GetSafeNormal2D();






}




void UPhysicsMovement::TickForceMove(float delta)
{
	
}

void UPhysicsMovement::TickHeadYawTorque()
{
	FVector TargetDir = m_Velocity.GetSafeNormal2D();

	if (TargetDir.IsNearlyZero(0.1f))
	{
		return;
	}

	float Dot = FVector::DotProduct(TargetDir, m_CurrentUpdateTargetForward);

	float RadiAngle = FMath::Acos(Dot);

	FVector InversedVector = UKismetMathLibrary::InverseTransformDirection(m_UpdateComponent->GetComponentTransform(), TargetDir);

	float DirPlusMinus;

	if (InversedVector.Y < 0.f)
	{
		DirPlusMinus = -1.f;
	}
	else
	{
		DirPlusMinus = 1.f;
	}
	m_UpdateComponent->AddTorqueInRadians(FVector(0, 0, RadiAngle*m_fMovingForce * DirPlusMinus * m_fTorqueSpeedWeight));

}

void UPhysicsMovement::TickHeadPitchRotate(const FVector & velocity, float deltaTime)
{

}

void UPhysicsMovement::TickLimitVelocity()
{
}

void UPhysicsMovement::CastGround(FHitResult & hitResult)
{



}

void UPhysicsMovement::ShowDebugVector( FVector  dir, float lineLength, FColor colorWant,float width) const
{
#if WITH_EDITOR
	if (m_bShowDebug)
	{
		if (!dir.Normalize(0.01f))
		{
			return;
		}

		DrawDebugLine(
			GetWorld(),
			m_UpdateComponent->GetComponentLocation(),
			m_UpdateComponent->GetComponentLocation() + lineLength * dir,
			colorWant,
			false, -1, 0,
			width
		);
	}
#endif
}



