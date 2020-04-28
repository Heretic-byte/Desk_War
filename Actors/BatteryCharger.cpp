


#include "BatteryCharger.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Battery.h"
#include "FuncLib/USBFunctionLib.h"
// Sets default values
ABatteryCharger::ABatteryCharger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_fMaxBattery = 100.f;
	m_fGiveBatteryPerSec = 20.f;
	m_fCurrentBattery = 0.f;
}

// Called when the game starts or when spawned
void ABatteryCharger::BeginPlay()
{
	Super::BeginPlay();
	m_fCurrentBattery = m_fMaxBattery;
}

// Called every frame
void ABatteryCharger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto* Battery = m_PlayerPawn->GetBattery();

	if (!Battery)
	{
		SetActorTickEnabled(false);
		return;
	}

	float BatterySuck = DeltaTime * m_fGiveBatteryPerSec;

	m_fCurrentBattery -= BatterySuck;

	if (m_fCurrentBattery <= 0)
	{
		m_fCurrentBattery = 0.f;
		m_OnGiving.Broadcast(0.f);
		m_OnFull.Broadcast();
		SetActorTickEnabled(false);
		return;
	}
	m_OnGiving.Broadcast(GetRemainBatteyPercentOne());
	Battery->ChargeBattery(BatterySuck);
}

void ABatteryCharger::OnConnected(UPinSkMeshComponent * skComp)
{
	if (GetRemainBatteyPercentOne() <= 0.f)
	{
		return;
	}
	m_PlayerPawn = Cast<AUSB_PlayerPawn>(UGameplayStatics::GetPlayerPawn(this, 0));

	if (!m_PlayerPawn)
	{
		return;
	}


	auto* Battery = m_PlayerPawn->GetBattery();

	if (!Battery)
	{
		return;
	}

	if (Battery->GetBatteryCurrentPercentOne()>=1.f)
	{
		return;
	}

	UUSBFunctionLib::SetAudioPlay(m_Audio, 1, 0.2f);

	SetActorTickEnabled(true);
}

void ABatteryCharger::OnDisconnected(UPinSkMeshComponent * skComp)
{
	SetActorTickEnabled(false);
	m_PlayerPawn = nullptr;
}

float ABatteryCharger::GetRemainBatteyPercentOne()
{
	return m_fCurrentBattery / m_fMaxBattery;
}

