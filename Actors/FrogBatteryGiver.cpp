#include "FrogBatteryGiver.h"
#include "Datas/USB_Macros.h"

AFrogBatteryGiver::AFrogBatteryGiver(const FObjectInitializer& objInit):Super(objInit)
{
	m_MeshFrog = CreateDefaultSubobject<USkeletalMeshComponent>("FrogMesh00");
}

void AFrogBatteryGiver::SpawnBattery()
{
	ShowActor(m_MeshFrog->GetSocketLocation(m_NameSpawnSocket));
	PRINTF("SpawnBatt");
}
