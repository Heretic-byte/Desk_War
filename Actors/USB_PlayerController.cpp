// Fill out your copyright notice in the Description page of Project Settings.


#include "USB_PlayerController.h"
#include "Managers/USB_CameraManager.h"
#include "Managers/USB_CheatManager.h"

AUSB_PlayerController::AUSB_PlayerController()
{
	PlayerCameraManagerClass = AUSB_CameraManager::StaticClass();
	CheatClass = UUSB_CheatManager::StaticClass();
}


void AUSB_PlayerController::BeginPlay()
{
	Super::BeginPlay();

}
