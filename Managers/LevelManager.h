

#pragma once

#include "CoreMinimal.h"
#include "Engine/ObjectLibrary.h"
/**
 * 
 */
class DESK_WAR_API LevelManager
{
protected:
	FName m_NameCurrentActiveLevel;
	UPROPERTY()
	UObject* m_WorldObj;
public:
	void Init(UObject* worldContext);
	void LoadLevel(FName newLevelName);
	void UnloadCurrent();
	void UnloadLevel(FName wantLevel);
};
