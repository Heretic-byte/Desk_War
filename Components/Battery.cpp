#include "Battery.h"
#include "Managers/USB_GameManager.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/USB_PlayerPawn.h"

// Sets default values for this component's properties
UBattery::UBattery()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBattery::BeginPlay()
{
	Super::BeginPlay();

	// ...
	m_fCurrentBattery = m_fMaxBattery;
	m_Player = Cast<AUSB_PlayerPawn>(UGameplayStatics::GetPlayerController(this, 0)->GetPawn());
}

float UBattery::GetBattery(float use)
{
	m_fCurrentBattery -= use;

	if (m_fCurrentBattery <= 0)
	{
		return 0.f;
	}

	float Ratio = GetBatteryCurrentPercentOne();//percent return
	m_OnBatteryGave.Broadcast(Ratio);

	return Ratio;
}

float UBattery::GetBatteryCurrentPercentOne()
{
	return m_fCurrentBattery / m_fMaxBattery;
}

void UBattery::AddBatteryToPlayer()
{
	m_Player->AddBattery(this);
}

void UBattery::RemoveBatteryToPlayer()
{
	m_Player->RemoveBattery(this);
}



