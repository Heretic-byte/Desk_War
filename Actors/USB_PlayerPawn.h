// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionManagerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/USB_SpringArm.h"
#include "Actors/USB_PhysicsPawn.h"
#include "Components/USBMovement.h"
#include "Datas/USB_Enum.h"
#include "USB_PlayerPawn.generated.h"

class UPhysicsSkMeshComponent;
class APlayerController;
class UPortSkMeshComponent;
class UBattery;


UCLASS(BlueprintType, Blueprintable)
class DESK_WAR_API AUSB_PlayerPawn : public AUSB_PhysicsPawn
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectVectorBP, const FVector&, pinLoc);
	AUSB_PlayerPawn(const FObjectInitializer& objInit);
public:
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FOnConnectVectorBP m_OnConnectedBP;
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FOnConnectVectorBP m_OnDisconnectedBP;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fDefaultFailImpulsePower;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fPortTraceRange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fBlockMoveTimeWhenEject;
	float m_fBlockMoveTimeWhenEjectTimer;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fHeadChangeCD;
	float m_fHeadChangeCDTimer;
	UPROPERTY()
	TArray<UPrimitiveComponent*> m_AryPhysicsBody;
	
	float m_fTotalMass;
	float m_fMaxSpeedSqr;
private:
	bool m_bBlockHeadChange;
	bool m_bBlockJump;
	bool m_bBlockChargeClick;
	bool m_bBlockInputMove;
protected://component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category="Movement")
	UUSBMovement* m_UsbMovement;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USceneComponent* m_CamRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* m_MainCam;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UUSB_SpringArm* m_MainSpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USB_Action")
	UActionManagerComponent* m_ActionManager;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USB_Body")
	USkeletalMeshComponent* m_MeshFaceSk;
	UPROPERTY(VisibleAnywhere, Category = "USB_Body")
	APlayerController* m_PlayerCon;
private:
	UPROPERTY()
	TArray<UBattery*> m_AryBatteries;
	UPROPERTY()
	TArray<AActor*> m_AryTraceIgnoreActors;
	UPROPERTY()
	UPhysicsSkMeshComponent* m_CurrentHead;
	UPROPERTY()
	UPhysicsSkMeshComponent* m_CurrentTail;
	UPROPERTY()
	UPortSkMeshComponent* m_CurrentFocusedPort;
	UPROPERTY()
	UPinSkMeshComponent* m_CurrentHeadPin;
	UPROPERTY()
	UPinSkMeshComponent* m_BaseHeadPin;
	UPROPERTY()
	UPinSkMeshComponent* m_BaseTailPin;
	UPROPERTY()
	UPortSkMeshComponent* m_PortTailPrev;
	UPROPERTY()
	UPortSkMeshComponent* m_PortHeadPrev;
private:
	FDelegateHandle m_ConnectChargingHandle;
public:
	UFUNCTION(BlueprintCallable, Category = "Connection")
	void TryConnect(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void ChangeHeadTail();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void AddTraceIgnoreActor(AActor* actorWant);
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	bool RemoveTraceIgnoreActor(AActor* actorWant);
	UFUNCTION(BlueprintCallable, Category = "USB_Getter")
	UPhysicsSkMeshComponent* GetHead();
	UFUNCTION(BlueprintCallable, Category = "USB_Getter")
	UPhysicsSkMeshComponent* GetTail();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void ConnectShot();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void DisconnectShot();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void Jump();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void StopJumping();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	USceneComponent* GetFocusedPortTarget();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void ZoomIn();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void ZoomOut();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	UBattery* GetBattery();
private://construct
	void InitPlayerPawn();
	void CreatePhysicMovement();
	void CreateCameraFamily();
	void CreateSkFaceMesh();
private:
	void SetHeadTail(UPhysicsSkMeshComponent* headWant, UPhysicsSkMeshComponent* tailWant,
		UPortSkMeshComponent* headPrevPort, UPortSkMeshComponent* tailPrevPort,
		bool bRemoveIgnoreOld=false);
	void MoveForward(float v);
	void MoveRight(float v);
	void RotateYaw(float v);
	void RotatePitch(float v);
private:
	void AddPhysicsBody(UPrimitiveComponent* wantP);
	void RemovePhysicsBody(UPrimitiveComponent* wantP);
	void ExitGame();
protected:
	void ConnectChargingStart();
	void SuccessConnection(UPortSkMeshComponent* portConnect);
	void AdjustPinTransform(UPortSkMeshComponent * portConnect);
public:
	void FailConnection(UPortSkMeshComponent* portConnect,const FHitResult * hitResult, EFailConnectionReason reason);
protected:
	void SetPhysicsVelocityAllBody(FVector linearV);
	bool TryDisconnect();
	void AddIgnoreActorsToQuery(FCollisionQueryParams& queryParam);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	void InitTraceIgnoreAry();
	void TickTracePortable();
	bool IsImpulseVelocityLower();
public:
	virtual void Tick(float DeltaTime) override;
	void EnableUSBInput();
	void DisableUSBInput(float dur=0.f);
	UFUNCTION(BlueprintCallable,Category="Input")
	void EnableUSBMove();
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DisableUSBMove(float dur = -1.f);
	void AddBattery(UBattery* batt);
	void RemoveBattery(UBattery* batt);
public:
	FORCEINLINE float GetTotalMass()
	{
		return m_fTotalMass;
	}
	FORCEINLINE UPhysicsSkMeshComponent* _inline_GetHead()
	{
		return m_CurrentHead;
	}
	FORCEINLINE UPhysicsSkMeshComponent* _inline_GetTail()
	{
		return m_CurrentTail;
	}
	FORCEINLINE UPhysicsMovement* GetMovement()
	{
		return m_UsbMovement;
	}

	FORCEINLINE bool IsMovingOnGround()
	{
		return m_UsbMovement->IsMovingOnGround();
	}

	FORCEINLINE const TArray<UPrimitiveComponent*>& GetPhysicsAry()
	{
		return m_AryPhysicsBody;
	}

	
};
