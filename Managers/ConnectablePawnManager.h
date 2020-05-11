

#pragma once

#include "CoreMinimal.h"

#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Datas/ConnectablePawnData.h"

/**
 * 
 */
class DESK_WAR_API ConnectablePawnManager
{
public:
	~ConnectablePawnManager();
protected:
	UPROPERTY()
	UWorld* m_CurrentWorld;
	UPROPERTY()
	UDataTable* m_ConnectPawnTable;
public:
	void Init(UWorld* worldHere,UDataTable* dataTable);
	// Spawn Pawn In Level
	// load pawn

	const FConnectablePawn_Data & GetConnectPawnData(FName id);
	//
};
