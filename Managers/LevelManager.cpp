


#include "LevelManager.h"
#include "Datas/USB_Macros.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"



void LevelManager::Init(UObject * worldContext)
{
	m_WorldObj = worldContext;
}

void LevelManager::LoadLevel(FName newLevelName)
{
	FLatentActionInfo LatenInfo;
	UGameplayStatics::LoadStreamLevel(m_WorldObj, newLevelName, true, true, LatenInfo);
}

void LevelManager::UnloadCurrent()
{
	if (m_NameCurrentActiveLevel == "")
	{
		return;
	}

	FLatentActionInfo LatentInfo;
	UGameplayStatics::UnloadStreamLevel(m_WorldObj, m_NameCurrentActiveLevel, LatentInfo,true);
}

void LevelManager::UnloadLevel(FName wantLevel)
{
	FLatentActionInfo LatentInfo;
	UGameplayStatics::UnloadStreamLevel(m_WorldObj, wantLevel, LatentInfo, true);
}
