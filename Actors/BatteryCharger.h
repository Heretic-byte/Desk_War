

#pragma once
#include "Actors/USB_PlayerPawn.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/ConnectableActor.h"
#include "Datas/USB_Macros.h"
#include "BatteryCharger.generated.h"

UCLASS()
class DESK_WAR_API ABatteryCharger : public AConnectableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABatteryCharger();

public:
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, Category = "Battery")
	FVoidFloatBP m_OnGivingBP;
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, Category = "Battery")
	FVoidVoidBP m_OnEmpty;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battery")
	float m_fMaxBattery;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battery")
	float m_fGiveBatteryPerSec;
	UPROPERTY()
	AUSB_PlayerPawn* m_PlayerPawn;
protected:
	float m_fCurrentBattery;
public:
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnConnected(UPinSkMeshComponent* skComp) override;
	virtual void OnDisconnected(UPinSkMeshComponent* skComp) override;
	UFUNCTION(BlueprintCallable,Category="Battery")
	float GetRemainBatteyPercentOne();
};
