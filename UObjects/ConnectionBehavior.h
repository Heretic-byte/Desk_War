

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ConnectionBehavior.generated.h"

/**
 * 
 */
class AUSB_PlayerPawn;
class IConnectable;
UCLASS()
class DESK_WAR_API UConnectionBehavior : public UObject
{
	GENERATED_BODY()
public:
	virtual void ExecuteConnection(AUSB_PlayerPawn* playerPawn, IConnectable* connectPawn) {};
	virtual void ExecuteDisconnection(AUSB_PlayerPawn* playerPawn, IConnectable* connectPawn) {};
};
