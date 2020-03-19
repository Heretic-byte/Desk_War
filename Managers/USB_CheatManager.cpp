// Fill out your copyright notice in the Description page of Project Settings.


#include "USB_CheatManager.h"
#include "Datas/USB_Macros.h"
void UUSB_CheatManager::InitCheatManager()
{
	Super::InitCheatManager();
	PRINTF("Cheat Manager Init !");
	m_Manager = GetWorld()->GetGameInstance<UUSB_GameManager>();
	m_PlayerController = Cast<AUSB_PlayerController>(GetWorld()->GetFirstPlayerController());
	m_Player = Cast<AUSB_PlayerPawn>( m_PlayerController->Player);
}

void UUSB_CheatManager::EnablePlayerInputMove()
{
	m_Player->EnableInputMove();
}

void UUSB_CheatManager::DisablePlayerInputMove(float timer)
{
	m_Player->DisableInputMove(timer);
}

void UUSB_CheatManager::DisablePlayerAutoMove()
{
	m_Player->DisableAutoMove();
}

void UUSB_CheatManager::EnablePlayerAutoMove(float x, float y, float z,float timer)
{
	m_Player->EnableAutoMove(FVector(x, y, z), timer);
}

void UUSB_CheatManager::DisablePlayerAutoRot()
{
	m_Player->DisableAutoRotate();
}

void UUSB_CheatManager::EnablePlayerAutoRot(float pitch, float yaw, float roll, float timer)
{
	m_Player->EnableAutoRotate(FRotator(pitch,yaw,roll),timer);
}

