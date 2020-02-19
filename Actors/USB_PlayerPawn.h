// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionManagerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/USB_SpringArm.h"
#include "Actors/USB_PhysicsPawn.h"
#include "Components/PhysicsMovement.h"
#include "USB_PlayerPawn.generated.h"
class UPortSkMeshComponent;


UCLASS(BlueprintType)
class DESK_WAR_API AUSB_PlayerPawn : public AUSB_PhysicsPawn
{
	GENERATED_BODY()
public:
	AUSB_PlayerPawn(const FObjectInitializer& objInit);
protected:
	FVector m_CamOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "USB_Action")
	float m_fPortTraceRange;
protected://component
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Movement")
	UPhysicsMovement* m_Movement;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USceneComponent* m_CamRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* m_MainCam;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UUSB_SpringArm* m_MainSpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "USB_Action")
	UActionManagerComponent* m_ActionManager;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "USB_Body")
	USkeletalMeshComponent* m_MeshFaceSk;
private:
	UPROPERTY()
	TArray<AActor*> m_AryTraceIgnoreActors;
	UPROPERTY()
	USkeletalMeshComponent* m_CurrentHead;
	UPROPERTY()
	USkeletalMeshComponent* m_CurrentTail;
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
	UPrimitiveComponent* GetHead();
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void ConnectShot();
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
	void SetHeadTail(USkeletalMeshComponent* headWant, USkeletalMeshComponent* tailWant);
	void MoveForward(float v);
	void MoveRight(float v);
	void RotateYaw(float v);
	void RotatePitch(float v);
protected:
	bool TryConnect();
	void BlockInput(bool tIsBlock);
	void AddIgnoreActorsToQuery(FCollisionQueryParams& queryParam);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	void InitTraceIgnoreAry();
	void TickTracePortable();
public:
	virtual void Tick(float DeltaTime) override;
private:
	FORCEINLINE UPrimitiveComponent* _inline_GetHead()
	{
		return m_CurrentHead;
	}
};
