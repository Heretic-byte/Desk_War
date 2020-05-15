

#pragma once

#include "CoreMinimal.h"
#include "UObjects/SawPlayerBehavior.h"
#include "DetectEscapeBehavior.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API UDetectEscapeBehavior : public USawPlayerBehavior
{
	GENERATED_BODY()
	
public:
	virtual void Execute(AConnectablePawn* connectPawn, float deltaTime) override;
	float m_fMoveTimer=0.f;
};
