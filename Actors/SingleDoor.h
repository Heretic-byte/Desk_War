

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SingleDoor.generated.h"

UCLASS()
class DESK_WAR_API ASingleDoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASingleDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
