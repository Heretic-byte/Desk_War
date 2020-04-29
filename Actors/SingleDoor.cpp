


#include "SingleDoor.h"

// Sets default values
ASingleDoor::ASingleDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASingleDoor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASingleDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

