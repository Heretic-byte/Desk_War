


#include "Battery.h"

// Sets default values for this component's properties
UBattery::UBattery()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UBattery::BeginPlay()
{
	Super::BeginPlay();

	// ...
	m_fCurrentBattery = m_fMaxBattery;
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



