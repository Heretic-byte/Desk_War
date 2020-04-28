

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "CamTimer.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class DESK_WAR_API UCamTimer : public UCameraComponent
{
	GENERATED_BODY()
public:
	UCamTimer();
protected:
	UPROPERTY(EditAnywhere, Category = "Camera",BlueprintReadOnly)
	float m_fBlendTime;
	UPROPERTY()
	APlayerController* m_PrevCamController;
	UPROPERTY()
	AActor* m_PrevCameraActor;

	float m_fShowTimer;
protected:
	virtual void BeginPlay() override;
public:
	UFUNCTION(BlueprintCallable,Category="Camera")
	void ShowCamera(APlayerController* con, float duration);
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void HideCamera();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
