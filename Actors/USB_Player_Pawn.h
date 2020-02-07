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


#include "UObject/ConstructorHelpers.h"
#include "USB_Player_Pawn.generated.h"

UCLASS()
class DESK_WAR_API AUSB_Player_Pawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AUSB_Player_Pawn(const FObjectInitializer& obj);

protected:
	UPROPERTY(BlueprintReadOnly,VisibleAnywhere,Category="USB_Player")
	USkeletalMeshComponent* m_MeshUsb;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "USB_Player")
	USceneComponent* m_SceneCamRoot;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "USB_Player")
	USpringArmComponent* m_SpringMain;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "USB_Player")
	UCameraComponent* m_CamMain;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fMovingForce;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fLimitLinearVelocity;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fAngularDamping;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fTorqueSpeedWeight;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fAirControlWeight;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_fJumpZVelocity;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_YawW =1.f;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_PitchW = 1.f;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	float m_LerpRotateTime = 1.f;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "USB_Player")
	FName m_ForwardBoneName;
protected:
	UPrimitiveComponent* m_PrimHead;
	float m_fOrientRotSpeed;
	float m_fVertical;
	float m_fHorizontal;
	float m_fMaxLinearSpeedSqr;
	bool m_bIsGround;
	FVector m_ForceDir;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetInputForceDir();


	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPrimitiveComponent* GetHead() const;

	bool GetIsGround() const;
private:
	void SetHead(UPrimitiveComponent* headWantPhysics);
	FVector GetHeadVelocity();
	void ForceForward(float v);
	void ForceRight(float v);
	void RotateYaw(float v);
	void RotatePitch(float v);
	void TickForceMove(float delta);
	void TickYawRotate(FVector &ForceDir, float delta);
	void TickPitchRollRotate(float delta);
	void TickLimitVelocity();
	void CastGround(FHitResult& hitResult);
protected:
	UFUNCTION(Category="USB_Player",BlueprintCallable)
	void DoJump();
};
