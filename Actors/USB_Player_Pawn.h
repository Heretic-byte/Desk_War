// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "Components/InputComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/InteractableComponent.h"


#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/Port.h"
#include "Components/StaticMeshComponent.h"

#include "Components/Port.h"
#include "Components/Connector.h"

#include "Actors/USB_PlayerController.h"
#include "ActionManagerComponent.h"

#include "UObject/ConstructorHelpers.h"

#include "Managers/USB_GameManager.h"

#include "USB_Player_Pawn.generated.h"

UCLASS()
class DESK_WAR_API AUSB_Player_Pawn : public APawn
{
	GENERATED_BODY()
public:
	AUSB_Player_Pawn(const FObjectInitializer& obj);
protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "USB_Player")
	USphereComponent* m_CollUsb;
	UPROPERTY(BlueprintReadOnly,VisibleAnywhere,Category="USB_Player")
	USkeletalMeshComponent* m_MeshUsb;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "USB_Player")
	USkeletalMeshComponent* m_Mesh4Pin;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "USB_Player")
	USceneComponent* m_SceneCamRoot;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "USB_Player")
	USpringArmComponent* m_SpringMain;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "USB_Player")
	UCameraComponent* m_CamMain;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "USB_Player")
	UConnector* m_USB;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "USB_Player")
	UConnector* m_Pin4;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spine_Physics")
	USplineComponent* m_SpineSpline;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	UPhysicalMaterial* m_SpineFriction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	UStaticMesh* m_SpineMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action")
	UActionManagerComponent* m_ActionManager;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fallback")
	TArray<TEnumAsByte<EObjectTypeQuery>> m_TraceTypes;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fallback")
	TArray<AActor*> m_IgnoreActors;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<USphereComponent*> m_ArySpineColls;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<USplineMeshComponent*> m_ArySplineMeshCompos;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<UPhysicsConstraintComponent*> m_AryPhysicsCompos;
	UPROPERTY(VisibleAnywhere)
	UUSB_GameManager* m_GameManager;
	UPROPERTY(VisibleAnywhere)
	UInteractableComponent* m_InteractObj;
	UPROPERTY(VisibleAnywhere)
	APlayerController* m_PlayerController;
	UPROPERTY(VisibleAnywhere)
	UConnector* m_ConnectorHead;
	UPROPERTY(VisibleAnywhere)
	UConnector* m_ConnectorTail;
protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fMovingForce;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fLimitLinearVelocity;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fAngularDamping;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fTorqueSpeedWeight;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fTorquePitchSpeedWeight;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fAirControlWeight;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fJumpZVelocity;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spine_Physics")
	float m_fWorldScaleX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	int m_nSpineCount;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "USB_Player")
	float m_fHeadChangeCD;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	bool m_bCanCamRotate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Connect")
	float m_fConnectHorizontalAngle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Connect_Fail")
	float m_fConnectFailImpulseOther;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Connect_Fail")
	float m_fConnectFailImpulseSelf;//PushPoint
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bones")
	FName m_NameUSBSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bones")
	FName m_Name4PinSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bones")
	FName m_NameConnectSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bones")
	FName m_NameConnectStartSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bones")
	FName m_NameConnectLeftSide;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bones")
	FName m_NameConnectRightSide;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bones")
	FName m_NameConnectPushPointSocket;
protected:
	UPrimitiveComponent* m_PrimHead;
	float m_fOrientRotSpeed;
	float m_fVertical;
	float m_fHorizontal;
	float m_fMaxLinearSpeedSqr;
	bool m_bIsGround;
	float m_fHeadChangeCDTimer;
	FVector m_CurrentGroundNormal;
protected:
	virtual void BeginPlay() override;
public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UPrimitiveComponent* GetHead() const;
	bool GetIsGround() const;
private:
	void SetHead(UPrimitiveComponent* headWantPhysics);
	FVector GetHeadVelocityDir();
	void ForceForward(float v);
	void ForceRight(float v);
	void RotateYaw(float v);
	void RotatePitch(float v);
	void TickForceMove(float delta);
	void TickHeadYawTorque(const FVector& velocity,const FVector headMeshDir);
	void TickHeadRollTorque(const FVector& velocity);
	void TickHeadPitchRotate(const FVector& velocity, float deltaTime);
	void TickLimitVelocity();
	void CastGround(FHitResult& hitResult);
	//
protected:
	UFUNCTION(Category="USB_Player",BlueprintCallable)
	void DoJump();
	UFUNCTION(BlueprintCallable, Category = "Init")
	void SetTailLocation();
};
