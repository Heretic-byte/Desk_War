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

UCLASS(BlueprintType)
class DESK_WAR_API AUSB_PlayerPawn : public AUSB_PhysicsPawn
{
	GENERATED_BODY()
public:
	AUSB_PlayerPawn(const FObjectInitializer& objInit);
protected:
	FVector m_CamOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Body")
	FName m_NamePinConnectSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Body")
	FName m_NamePinConnectStartSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Body")
	FName m_NamePinConnectPushPointSocket;
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
	bool m_bCanConnectDist;
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
	UPinSkMeshComponent* m_CurrentHeadPin;
	UPROPERTY()
	UPinSkMeshComponent* m_CurrentTailPin;
	UPROPERTY()
	UPortSkMeshComponent* m_CurrentFocusedPort;
public:
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
	bool CheckPortVerticalAngle(UPortSkMeshComponent * port);
	bool CheckPortHorizontalAngle(UPortSkMeshComponent * port);
	UCActionBaseInterface* MoveForReadyConnect(UPortSkMeshComponent * portWant);
	UCActionBaseInterface* RotateForConnect(UPortSkMeshComponent * portWant);
	UCActionBaseInterface* MoveForPushConnection(UPortSkMeshComponent * portWant);
protected:
	bool CheckConnectTransform();
	bool TryConnect(UPortSkMeshComponent* portWant);
	bool TryDisconnect();
	void BlockInput(bool tIsBlock);
	void AddIgnoreActorsToQuery(FCollisionQueryParams& queryParam);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	void InitTraceIgnoreAry();
	void TickTracePortable();
public:
	void BlockMovement();
	void UnblockMovement();
	virtual void Tick(float DeltaTime) override;
private:
	FORCEINLINE UPhysicsSkMeshComponent* _inline_GetHead()
	{
		return m_CurrentHead;
	}
	FORCEINLINE UPhysicsSkMeshComponent* _inline_GetTail()
	{
		return m_CurrentTail;
	}
	FORCEINLINE UPhysicsSkMeshComponent* _inline_GetHeadPin()
	{
		return m_CurrentHeadPin;
	}
	FORCEINLINE UPhysicsSkMeshComponent* _inline_GetTailPin()
	{
		return m_CurrentTailPin;
	}
};
