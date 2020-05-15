#pragma once


#include "CoreMinimal.h"
#include "UObjects/SawPlayerBehavior.h"
#include "DetectChaseBehavior.generated.h"



UCLASS()
class DESK_WAR_API UDetectChaseBehavior : public USawPlayerBehavior
{
	GENERATED_BODY()

public:
	virtual void Execute(AConnectablePawn* connectPawn, float deltaTime) override;
	
};
