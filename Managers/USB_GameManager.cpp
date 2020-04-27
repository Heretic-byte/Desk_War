// Fill out your copyright notice in the Description page of Project Settings.


#include "USB_GameManager.h"


void UUSB_GameManager::GameOver()
{
	PRINTF("GameOver");
}

void UUSB_GameManager::Init()
{
	Super::Init();
	m_LevelManager = new LevelManager();
	m_LevelManager->Init(this);
}

void UUSB_GameManager::Shutdown()
{
	Super::Shutdown();
	delete m_LevelManager;
}

void UUSB_GameManager::UnloadCurrent()
{
	m_LevelManager->UnloadCurrent();
}

void UUSB_GameManager::UnloadLevel(FName wantLevel)
{
	m_LevelManager->UnloadLevel(wantLevel);
}

void UUSB_GameManager::LoadLevel(FName newLevelName)
{
	m_LevelManager->LoadLevel(newLevelName);
}
