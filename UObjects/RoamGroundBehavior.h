

#pragma once

#include "CoreMinimal.h"
#include "UObjects/IdleBehavior.h"
#include "RoamGroundBehavior.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API URoamGroundBehavior : public UIdleBehavior
{
	GENERATED_BODY()
	
public:
	float m_fRoamCooldown=0.f;

	virtual void Execute(AConnectablePawn* connectPawn, float deltaTime)override;
};
