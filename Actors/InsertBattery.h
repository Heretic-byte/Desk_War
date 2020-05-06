

#pragma once

#include "CoreMinimal.h"
#include "Actors/BatteryConsumer.h"
#include "Components/PuzzleKey.h"
#include "InsertBattery.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API AInsertBattery : public ABatteryConsumer
{
	GENERATED_BODY()
	
public:
	AInsertBattery();

protected:
	UPROPERTY()
	USoundBase* m_ChargingSound;
	UPROPERTY()
	USoundBase* m_PuzzleUnlockSound;
	UPROPERTY(VisibleAnywhere,Category="Puzzle")
	UPuzzleKey* m_PuzzleKey;
	UPROPERTY(EditDefaultsOnly, Category = "Puzzle")
	int m_nMatGaugeIndex;

protected:
	virtual void BeginPlay() override;

	virtual void OnConnected(UPinSkMeshComponent* skComp) override;
	virtual void OnDisconnected(UPinSkMeshComponent* skComp) override;
public:
	UFUNCTION()
	void SetGaugeToDoor(float gauge);
	UFUNCTION()
	void OnFull();
};
