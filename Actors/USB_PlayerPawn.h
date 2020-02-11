// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionManagerComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Actors/USB_PhysicsPawn.h"
#include "Components/PhysicsMovement.h"
#include "USB_PlayerPawn.generated.h"

UCLASS()
class DESK_WAR_API AUSB_PlayerPawn : public AUSB_PhysicsPawn
{
	GENERATED_BODY()
public:
	AUSB_PlayerPawn(const FObjectInitializer& objInit);
protected://component
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Movement")
	UPhysicsMovement* m_Movement;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USceneComponent* m_CamRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* m_MainCam;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* m_MainSpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action")
	UActionManagerComponent* m_ActionManager;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "USB_Body")
	USkeletalMeshComponent* m_MeshFaceSk;
private:
	UPinSkMeshComponent* m_CurrentHead;
	UPinSkMeshComponent* m_CurrentTail;
public:
	UFUNCTION(BlueprintCallable, Category = "USB_Action")
	void ChangeHeadTail();
	UFUNCTION(BlueprintCallable, Category = "USB_Getter")
	UPinSkMeshComponent* GetHead();
private://construct
	void InitPlayerPawn();
	void CreatePhysicMovement();
	void CreateCameraFamily();
	void CreateSkFaceMesh();
private:
	void SetHeadTail(UPinSkMeshComponent* headWant, UPinSkMeshComponent* tailWant);
	void MoveForward(float v);
	void MoveRight(float v);
	void RotateYaw(float v);
	void RotatePitch(float v);
protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
private:
	FORCEINLINE UPinSkMeshComponent* _inline_GetHead()
	{
		return m_CurrentHead;
	}
};
