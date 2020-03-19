// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionManagerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/USB_SpringArm.h"
#include "Actors/USB_PhysicsPawn.h"
#include "Components/PhysicsMovement.h"
#include "USB_PlayerPawn.generated.h"
class UPhysicsSkMeshComponent;
class APlayerController;
class UPortSkMeshComponent;
UCLASS(BlueprintType, Blueprintable)
class DESK_WAR_API AUSB_PlayerPawn : public AUSB_PhysicsPawn
{
	GENERATED_BODY()
public:
	AUSB_PlayerPawn(const FObjectInitializer& objInit);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fMaxConnectRotTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fMinConnectRotTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fDefaultFailImpulsePower;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fConnectReadyDuration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fConnectPushDuration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fPortTraceRange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fConnectHorizontalAngle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fEjectionPower;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fBlockMoveTimeWhenEject;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fHeadChangeCD;
	float m_fHeadChangeCDTimer;
	UPROPERTY()
	TArray<UPrimitiveComponent*> m_AryPhysicsBody;
	float m_fTotalMass;
	bool m_bBlockHeadChange;
protected://component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category="Movement")
	UPhysicsMovement* m_Movement;
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
	TArray<AActor*> m_AryTraceIgnoreActors;
	UPROPERTY()
	UPhysicsSkMeshComponent* m_CurrentHead;
	UPROPERTY()
	UPhysicsSkMeshComponent* m_CurrentTail;
	UPROPERTY()
	UPortSkMeshComponent* m_CurrentFocusedPort;
private:
	UPROPERTY()
	UPinSkMeshComponent* m_CurrentTryConnectingPin;
	UPROPERTY()
	UPortSkMeshComponent* m_CurrentTryConnectingPort;
public:
	UFUNCTION(BlueprintCallable, Category = "Connection")
	void TryConnect(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
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
private://construct
	void InitPlayerPawn();
	void CreatePhysicMovement();
	void CreateCameraFamily();
	void CreateSkFaceMesh();
private:
	void SetHeadTail(UPhysicsSkMeshComponent* headWant, UPhysicsSkMeshComponent* tailWant);
	void MoveForward(float v);
	void MoveRight(float v);
	void RotateYaw(float v);
	void RotatePitch(float v);
private:
	void AddPhysicsBody(UPrimitiveComponent* wantP);
	void RemovePhysicsBody(UPrimitiveComponent* wantP);
protected:
	void SuccessConnection();
	void FailConnection(const FHitResult & hitResult);
	void SetPhysicsVelocityAllBody(FVector linearV);
	bool TryDisconnect();
	void BlockInput(bool tIsBlock);
	void AddIgnoreActorsToQuery(FCollisionQueryParams& queryParam);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	void InitTraceIgnoreAry();
	void TickTracePortable();
	void ConnectChargingEnd();
public:
	void EnableInputMove();
	void DisableInputMove(float timer);
	void EnableAutoMove(FVector wantDir,float timer);
	void DisableAutoMove();
	void EnableAutoRotate(FRotator wantRot,float timer);
	void DisableAutoRotate();
	virtual void Tick(float DeltaTime) override;
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
};
