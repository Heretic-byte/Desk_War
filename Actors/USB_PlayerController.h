// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "USB_PlayerController.generated.h"

/**
 * 
 */

class UPhysicsPathFollowingComponent;
UCLASS()
class DESK_WAR_API AUSB_PlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AUSB_PlayerController();

protected:
	UPROPERTY(VisibleAnywhere,Category="Navigation",BlueprintReadOnly)
	UPhysicsPathFollowingComponent* m_PathFollowComp;
	UPROPERTY(EditDefaultsOnly, Category = "Debug", BlueprintReadOnly)
	bool m_bUseNavMove;
public:
	bool bMoveToMouseCursor;

	FVector DestWant;

	FRotator RotWant;

public:
	virtual void BeginPlay() override;

	void PlayerTick(float DeltaTime);

	virtual void SetupInputComponent() override;

	void MoveToMouseCursor();

	void SetNewMoveDestination(const FVector DestLocation);

	void OnSetDestinationPressed();

	void OnSetDestinationReleased();

	virtual FVector GetNavAgentLocation() const override;

};
