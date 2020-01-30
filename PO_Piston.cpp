// Fill out your copyright notice in the Description page of Project Settings.


#include "PO_Piston.h"
#include "Math/UnrealMathUtility.h"

APO_Piston::APO_Piston()
{
	RecoverySpeed = 450;
}

// Called every frame
void APO_Piston::Tick(float DeltaTime)
{
	
	if (!bToDestination || IsNeutral()) {	// ToHome
		MoveMeshToHome(DeltaTime);
	}
	else {
		if (AffectedForce.Equals(FVector::ZeroVector)) {
			MoveMeshToHome(DeltaTime);
		}
	}
	
	
	Super::Tick(DeltaTime);
}

void APO_Piston::MoveMeshToHome(float DeltaTime)
{
	if (Alpha <= 0) {
		return;
	}

	float RecoveryRate = FMath::Clamp((Alpha / MaxAlpha), 0.3f, 1.0f);	// Home에 가까울 수록 감속
	FVector NetForce = Direction * (-1 * RecoverySpeed) * RecoveryRate;

	bToDestination = false;
	MoveMesh(NetForce, DeltaTime);
}