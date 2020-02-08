// Fill out your copyright notice in the Description page of Project Settings.


#include "PinConnector.h"

// Sets default values
APinConnector::APinConnector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APinConnector::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APinConnector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

