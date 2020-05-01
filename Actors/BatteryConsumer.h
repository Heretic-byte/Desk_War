

#pragma once

#include "CoreMinimal.h"
#include "Actors/ConnectableActor.h"
#include "Actors/USB_PlayerPawn.h"
#include "Datas/USB_Macros.h"
#include "BatteryConsumer.generated.h"

UCLASS()
class DESK_WAR_API ABatteryConsumer : public AConnectableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABatteryConsumer();
public:
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, Category = "Battery")
	FVoidFloatBP m_OnGivingBP;
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, Category = "Battery")
	FVoidVoidBP m_OnFullBP;
protected:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Battery")
	float m_fMaxBattery;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battery")
	float m_fChargingTime;
	UPROPERTY()
	AUSB_PlayerPawn* m_PlayerPawn;
protected:
	float m_fCollectedBattery;
	float m_fSuckBatteryPerSec;
public:	
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnConnected(UPinSkMeshComponent* skComp) override;
	virtual void OnDisconnected(UPinSkMeshComponent* skComp) override;
	bool CheckNeedBattery();
};
