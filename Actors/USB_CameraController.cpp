// Fill out your copyright notice in the Description page of Project Settings.


#include "USB_CameraController.h"

// Sets default values
AUSB_CameraController::AUSB_CameraController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUSB_CameraController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUSB_CameraController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

