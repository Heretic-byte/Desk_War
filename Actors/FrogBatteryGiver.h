

#pragma once

#include "CoreMinimal.h"
#include "Actors/ObjectGiver.h"
#include "Components/SkeletalMeshComponent.h"
#include "FrogBatteryGiver.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API AFrogBatteryGiver : public AObjectGiver
{
	GENERATED_BODY()
	
public:
	AFrogBatteryGiver(const FObjectInitializer& objInit);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Frog")
	FName m_NameSpawnSocket;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Frog")
	USkeletalMeshComponent* m_MeshFrog;
public:
	UFUNCTION(BlueprintCallable,Category="Frog")
	void SpawnBattery();
};
