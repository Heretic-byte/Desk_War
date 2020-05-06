

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Actors/ObjectGiver.h"
#include "IPoolingObj.generated.h"

//class AObjectGiver;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIPoolingObj : public UInterface
{
	GENERATED_BODY()
};

class DESK_WAR_API IIPoolingObj
{
	GENERATED_BODY()
public:
	virtual void OnInit(AObjectGiver* objGiver)
	{

	}
	virtual void OnPullEnque()
	{

	}
};
