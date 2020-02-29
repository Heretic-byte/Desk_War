


#include "PhysicsAiController.h"
#include "Components/PhysicsPathFollowingComponent.h"


APhysicsAiController::APhysicsAiController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPhysicsPathFollowingComponent>(TEXT("PathFollowingComponent")))
{

}