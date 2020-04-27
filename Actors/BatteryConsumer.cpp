


#include "BatteryConsumer.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Battery.h"
#include "FuncLib/USBFunctionLib.h"
// Sets default values
ABatteryConsumer::ABatteryConsumer()
{
	m_fMaxBattery = 100.f;
	m_fChargingTime = 5.f;
	m_fCollectedBattery = 0.f;
	m_fSuckBatteryPerSec = 0.f;
}

// Called when the game starts or when spawned
void ABatteryConsumer::BeginPlay()
{
	Super::BeginPlay();
	m_fSuckBatteryPerSec = m_fMaxBattery / m_fChargingTime;
}

// Called every frame
void ABatteryConsumer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto* Battery = m_PlayerPawn->GetBattery();

	if (!Battery || !CheckNeedBattery() || Battery->GetBatteryCurrentPercentOne()<=0.f)
	{
		SetActorTickEnabled(false);
		return;
	}
	float GainBattery = DeltaTime * m_fSuckBatteryPerSec;
	Battery->UseBattery(GainBattery);
	
	m_fCollectedBattery += GainBattery;
	m_OnGiving.Broadcast(m_fCollectedBattery / m_fMaxBattery);

	if (!CheckNeedBattery())
	{
		m_OnEmpty.Broadcast();
	}
}

void ABatteryConsumer::OnConnected(UPinSkMeshComponent * skComp)
{
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

	if (Battery->GetBatteryCurrentPercentOne() <=0.0f)
	{
		return;
	}

	UUSBFunctionLib::SetAudioPlay(m_Audio, 1, 0.2f);

	SetActorTickEnabled(true);
}

void ABatteryConsumer::OnDisconnected(UPinSkMeshComponent * skComp)
{
	SetActorTickEnabled(false);
	m_PlayerPawn = nullptr;
}

bool ABatteryConsumer::CheckNeedBattery()
{
	return m_fMaxBattery > m_fCollectedBattery;
}

