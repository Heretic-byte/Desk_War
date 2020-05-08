

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Containers/Queue.h"
#include "ObjectGiver.generated.h"

UCLASS()
class DESK_WAR_API AObjectGiver : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObjectGiver(const FObjectInitializer& objInit);
protected:
	UPROPERTY(EditAnywhere, Category = "Object")
	TSubclassOf<AActor> m_cActorWantSpawn;
	UPROPERTY(EditAnywhere,Category="Object")
	int m_nPoolCount;

	TQueue<AActor*> m_QPoolObj;
protected:
	int m_nCurrentIndex;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CreatePoolObject();
public:	
	UFUNCTION(BlueprintCallable,Category="Object")
	AActor* ShowActor(const FVector& pos);

	void PullBackActor(AActor* obj);
};
