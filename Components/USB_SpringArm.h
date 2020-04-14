// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Components/SceneComponent.h"
#include "USB_SpringArm.generated.h"


UCLASS(ClassGroup = Camera, meta = (BlueprintSpawnableComponent), hideCategories = (Mobility))
class DESK_WAR_API UUSB_SpringArm : public USceneComponent
{
	GENERATED_UCLASS_BODY()
	
private:
	bool m_bWasWallBlocked;
	FVector m_LastTarget;
	void StartSmoothTime(float deltaTime);
	void EndSmoothTime();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float m_fWallBlockEndSmoothTime;
	float m_fWallBlockEndSmoothTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	FRotator m_RotOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float m_fWheelZoomSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float m_fMinimumArmLength;
	float m_fMaximumArmLength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float m_fCamZoomInSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float TargetArmLength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	FVector SocketOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	FVector TargetOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraCollision, meta = (editcondition = "bDoCollisionTest"))
	float ProbeSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraCollision, meta = (editcondition = "bDoCollisionTest"))
	TEnumAsByte<ECollisionChannel> ProbeChannel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraCollision)
	uint32 bDoCollisionTest : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraSettings)
	uint32 bUsePawnControlRotation : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraSettings)
	uint32 bInheritPitch : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraSettings)
	uint32 bInheritYaw : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraSettings)
	uint32 bInheritRoll : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag)
	uint32 bEnableCameraLag : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag)
	uint32 bEnableCameraRotationLag : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, AdvancedDisplay)
	uint32 bUseCameraLagSubstepping : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag)
	uint32 bDrawDebugLagMarkers : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, meta = (editcondition = "bEnableCameraLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float CameraLagSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, meta = (editcondition = "bEnableCameraRotationLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float CameraRotationLagSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, AdvancedDisplay, meta = (editcondition = "bUseCameraLagSubstepping", ClampMin = "0.005", ClampMax = "0.5", UIMin = "0.005", UIMax = "0.5"))
	float CameraLagMaxTimeStep;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, meta = (editcondition = "bEnableCameraLag", ClampMin = "0.0", UIMin = "0.0"))
	float CameraLagMaxDistance;
	UFUNCTION(BlueprintCallable, Category = SpringArm)
	FRotator GetTargetRotation() const;
	UFUNCTION(BlueprintCallable, Category = CameraCollision)
	FVector GetUnfixedCameraPosition() const;
	UFUNCTION(BlueprintCallable, Category = CameraCollision)
	bool IsCollisionFixApplied() const;
	bool bIsCameraFixed = false;
	FVector UnfixedCameraPosition;
	FVector PreviousDesiredLoc;
	FVector PreviousArmOrigin;
	FRotator PreviousDesiredRot;
public:
	void ZoomIn();
	void ZoomOut();
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PostLoad() override;
	virtual void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift) override;
	virtual bool HasAnySockets() const override;
	virtual FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const override;
	virtual void QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const override;
	static const FName SocketName;
protected:
	FVector RelativeSocketLocation;
	FQuat RelativeSocketRotation;
protected:
	virtual void UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime);
	FVector CollisionCameraFix(FVector &ArmOrigin, FVector &DesiredLoc,  float DeltaTime);
	virtual FVector BlendLocations(const FVector& DesiredArmLocation, const FVector& TraceHitLocation, bool bHitSomething, float DeltaTime, const FHitResult& hit);
	FVector ClampTargetLocation(const FVector& traceLoc, const FHitResult& hit);
};
