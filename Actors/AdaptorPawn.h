

#pragma once

#include "CoreMinimal.h"
#include "Actors/PortPawn.h"
#include "Components/NavPawnMovement.h"
#include "Components/PinSkMeshComponent.h"
#include "AdaptorPawn.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API AAdaptorPawn : public APortPawn
{
	GENERATED_UCLASS_BODY()
	
protected:
	virtual void BeginPlay() override;

	virtual void PortConnected(UPinSkMeshComponent* pinConnect) override;
	virtual void PortDisConnected(UPinSkMeshComponent* pinConnect) override;
protected:
	UPROPERTY(VisibleAnywhere)
	UNavPawnMovement* m_Movement;


};
