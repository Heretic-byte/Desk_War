

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Datas/USB_Macros.h"
#include "Battery.generated.h"

class AUSB_PlayerPawn;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DESK_WAR_API UBattery : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBattery();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
protected:
	UPROPERTY(EditAnywhere,Category="Battery")
	float m_fMaxBattery;

	float m_fCurrentBattery;
	
	UPROPERTY(BlueprintAssignable, Category = "Battery")
	FVoidFloatBP m_OnBatteryGave;

	UPROPERTY(VisibleAnywhere)
	AUSB_PlayerPawn* m_Player;
public:
	UFUNCTION(BlueprintCallable, Category = "Battery")
	float GetBattery(float use);
	UFUNCTION(BlueprintCallable, Category = "Battery")
	float GetBatteryCurrentPercentOne();
	UFUNCTION(BlueprintCallable, Category = "Battery")
	void AddBatteryToPlayer();
	UFUNCTION(BlueprintCallable, Category = "Battery")
	void RemoveBatteryToPlayer();

};
