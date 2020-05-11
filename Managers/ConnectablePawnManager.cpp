
#include "ConnectablePawnManager.h"


ConnectablePawnManager::~ConnectablePawnManager()
{
	m_CurrentWorld = nullptr;
	m_ConnectPawnTable = nullptr;
}

void ConnectablePawnManager::Init(UWorld * worldHere, UDataTable * dataTable)
{
	m_CurrentWorld = worldHere;
	m_ConnectPawnTable = dataTable;
}

const FConnectablePawn_Data & ConnectablePawnManager::GetConnectPawnData(FName id)
{
	return *m_ConnectPawnTable->FindRow<FConnectablePawn_Data>(id, "");
}
