

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsPublic.h"
#include "Components/PrimitiveComponent.h"
#include "TestPhysics.generated.h"

UCLASS()
class DESK_WAR_API ATestPhysics : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(BlueprintReadWrite)
	UPrimitiveComponent* m_Primi;
	// Sets default values for this actor's properties
	ATestPhysics();

	float m_fTimer;
	bool m_StopPrint = false;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void PrintStopWatch();
public:
	FDelegateHandle OnPhysSceneStepHandle;
	FCalculateCustomPhysics OnCalculateCustomPhysics;
	void SubstepTick(float DeltaTime, FBodyInstance* BodyInstance);
	void PhysSceneStep(FPhysScene * PhysScene, float DeltaTime);
};
