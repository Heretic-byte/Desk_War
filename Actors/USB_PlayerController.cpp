// Fill out your copyright notice in the Description page of Project Settings.


#include "USB_PlayerController.h"
#include "Managers/USB_CameraManager.h"
#include "Managers/USB_CheatManager.h"
#include "DrawDebugHelpers.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Actors/USB_PlayerPawn.h"
#include "Engine/World.h"

#include "Components/PrimitiveComponent.h"
#include "Components/PhysicsMovement.h"

#include "Components/PhysicsPathFollowingComponent.h"


DEFINE_LOG_CATEGORY_STATIC(USB_PlayerController, Log, All);

AUSB_PlayerController::AUSB_PlayerController()
{
	PlayerCameraManagerClass = AUSB_CameraManager::StaticClass();
	CheatClass = UUSB_CheatManager::StaticClass();
	m_bUseNavMove = false;
	//
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	m_PathFollowComp = CreateDefaultSubobject<UPhysicsPathFollowingComponent>("PathFollowComp00");
	m_PathFollowComp->Initialize();
}


void AUSB_PlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	m_PathFollowComp->SetMovementComponent( GetPawn()->GetMovementComponent());

	if (m_bUseNavMove)
	{
		bShowMouseCursor = true;
	}
}


void AUSB_PlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (bMoveToMouseCursor&&m_bUseNavMove)
	{
		MoveToMouseCursor();
	}
}

void AUSB_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &AUSB_PlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &AUSB_PlayerController::OnSetDestinationReleased);
}


void AUSB_PlayerController::MoveToMouseCursor()
{
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		SetNewMoveDestination(Hit.ImpactPoint);
	}
}

void AUSB_PlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		float const Distance = FVector::Dist(DestLocation, MyPawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if ((Distance > 120.0f))
		{
			//DrawDebugLine(GetWorld(), MyPawn->GetActorLocation(), DestLocation, FColor::Cyan, false, -1.f, 1, 3.f);
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, DestLocation);
		}
	}
}

void AUSB_PlayerController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void AUSB_PlayerController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}

FVector AUSB_PlayerController::GetNavAgentLocation() const
{
	return  Cast<UPhysicsMovement>( GetPawn()->GetMovementComponent())->GetMovingTargetComponent()->GetComponentLocation();

	//return GetPawn()->GetActorLocation();
}
