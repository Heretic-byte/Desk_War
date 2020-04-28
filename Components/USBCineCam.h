

#pragma once

#include "CoreMinimal.h"
#include "CineCameraComponent.h"
#include "USBCineCam.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API UUSBCineCam : public UCineCameraComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable,Category="Camera")
	void ShowCam(float duration);
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void HideCam();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
