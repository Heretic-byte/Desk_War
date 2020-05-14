

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"


#include "Actors/ConnectablePawn.h"
#include "NavigationPath.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"
#include "Navigation/PathFollowingComponent.h"

#include "ConnectPawnFSM.generated.h"

//class AConnectablePawn;

UCLASS()
class DESK_WAR_API UConnectPawnFSM : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void Execute(AConnectablePawn* connectPawn,float deltaTime);
};
