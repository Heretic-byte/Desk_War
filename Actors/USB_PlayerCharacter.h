// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Datas/USB_Macros.h"
#include "Datas/USB_Enum.h"
#include "Kismet/KismetMathLibrary.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "EasyActionPlugin.h"
#include "ActionManagerComponent.h"
#include "CActionFactory.h"
#include "USB_PlayerCharacter.generated.h"


class UInteractableComponent;
class UConnector;
class UPort;

class UUSB_GameManager;


//비쥬얼상의 헤드와 테일은,USB 4PIN으로 개명
//connector 와 port 개명다시
UCLASS()
class DESK_WAR_API AUSB_PlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AUSB_PlayerCharacter(const FObjectInitializer& ObjectInitializer);
	void CreateComponent();
private://ufunction
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable, Category = "Init")
	void InitUSB();
	UFUNCTION(BlueprintCallable, Category = "Init")
	void SetTailLocation();
	UFUNCTION(BlueprintCallable, Category = "Init")
	void SpawnSpineColls();
	UFUNCTION(BlueprintCallable, Category = "Init")
	void InitSplineComponent();
	UFUNCTION(BlueprintCallable, Category = "Init")
	void InitSplineMesh();
	UFUNCTION(BlueprintCallable, Category = "Init")
	void InitPhysicsConstraints();
	UFUNCTION(BlueprintCallable, Category = "Init")
	void InitMovement();
	UFUNCTION(BlueprintCallable, Category = "Init")
	void SetHeadTailDefault();
	UPhysicsConstraintComponent* AddPhysicsConstraint(const FTransform trans);
public:
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	void Interact();
	UFUNCTION(BlueprintCallable, Category = "GameSystem")
	void Kill();
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	void ChangeHead();
	UFUNCTION(BlueprintCallable, Category = "Connect")
	void TryConnect(UPort* port);//always head
	UFUNCTION(BlueprintCallable, Category = "Connect")
	void TryDisconnect();//always tail
protected:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void EnablePlayerInput();
	void DisablePlayerInput();
private:
	void SetHeadTail(UConnector* head, UConnector* tail);
	void TickUpdateSpine();
	void UpdateSplinePoint();
	void UpdateSplineMesh();
	void SlopeHeadMeshControl(float deltaTime);
	void TryFocusInteract();
	void SetHeadRotInterp(FRotator targetRot, float deltaTime, float interTime);
	
	void MoveForward(float v);
	void MoveRight(float v);
	void RotateYaw(float v);
	void RotatePitch(float v);

private://connection
	void GetPortCenterTracePoint(FVector& startPoint,FVector& endPoint, float length);
	void GetPortLeftSideTracePoint(FVector& startPoint, FVector& endPoint, float length);
	void GetPortRightSideTracePoint(FVector& startPoint, FVector& endPoint, float length);

	bool CheckPortHorizontalAngle(UPort * port);
	bool CheckPortVerticalAngle(UPort * port);
	void ConnectShotSuccess(UPort * portToAdd);
	void ConnectShotFail();
	UCActionBaseInterface* MoveForReadyConnect(UPort * portWant);
	UCActionBaseInterface* RotateForConnect(UPort * portWant);
	UCActionBaseInterface* MoveForPushConnection(UPort * portWant);
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	void TickCastForKnockBack(float time);
	bool KnockBackCast(const FVector &StartPoint, const FVector &EndPoint, FHitResult &HitResult, FRotator rotateWant);
	void CancelConnetFailShot();
public:
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	FVector GetUsbNeckSocLoc() const;
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	FVector Get4PinNeckSocLoc() const;
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	FVector GetConnectorCenterSocLoc() const;
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	FVector GetConnectorPushPosSocLoc() const;
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	FVector GetConnectorCenterRootSocLoc() const;
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	FVector GetConnectorRightSocLoc() const;
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	FVector GetConnectorLeftSocLoc() const;
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	UCapsuleComponent* GetHeadCollision() const;
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	UCapsuleComponent* GetTailCollision() const;
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	USkeletalMeshComponent* GetHeadMesh() const;
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	USkeletalMeshComponent* GetTailMesh() const;
protected://components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,Category="GamePlay")
	UCapsuleComponent* m_CollUSB;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GamePlay")
	USkeletalMeshComponent* m_MeshUSB;
	//4pin
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GamePlay")
	UCapsuleComponent* m_Coll4Pin;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GamePlay")
	USkeletalMeshComponent* m_Mesh4Pin;
	//misc
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spine_Physics")
	USplineComponent* m_SpineSpline;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	UPhysicalMaterial* m_SpineFriction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	UStaticMesh* m_SpineMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	TEnumAsByte< ESplineCoordinateSpace::Type> m_SplineSpace;
	bool m_bIsComponentLocal;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float m_fYawSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float m_fPitchSpeed;
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USceneComponent* m_CamRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* m_MainCam;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* m_MainSpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action")
	UActionManagerComponent* m_ActionManager;
	//
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
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Interaction")
	float m_fInteractRange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	float m_fLineExtraSpacing;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	float m_fLineRadius;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	float m_fAngularDamping;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	float m_fLinearDamping;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	float m_fCollMass;
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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spine_Physics")
	float m_fWorldScaleX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spine_Physics")
	int m_nSpineCount;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GamePlay")
	float m_fHeadChangeCD;
	float m_fHeadChangeCDTimer;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	bool m_bCanCamRotate;	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Connect")
	float m_fConnectHorizontalAngle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Connect_Fail")
	float m_fConnectFailImpulseOther;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Connect_Fail")
	float m_fConnectFailImpulseSelf;//PushPoint
public://delegate
	UPROPERTY(BlueprintAssignable, Category = "GameSystem")
	FVoidVoidBP m_OnKilled;
protected:
	void SetUpSceneComponent(USceneComponent * compo, USceneComponent* parent, FTransform trans);
	void SetUpActorComponent(UActorComponent * compo);
public:
	template <typename component>
	FORCEINLINE component* AddSceneComponent(TSubclassOf<component> templateWant, USceneComponent* parent, FTransform trans)
	{
		auto* Created=	NewObject<component>(this, NAME_None, RF_NoFlags, templateWant->GetDefaultObject<component>());
		Created->SetMobility(EComponentMobility::Movable);
		SetUpSceneComponent(Created, parent,trans);

		return Created;
	}
	template <typename component>
	FORCEINLINE component* AddActorComponent(TSubclassOf<component> templateWant)
	{
		auto* Created = NewObject<component>(this, NAME_None, RF_NoFlags, templateWant->GetDefaultObject<component>());
		SetUpActorComponent(Created);

		return Created;
	}
public:
		UFUNCTION(BlueprintCallable, Category = "GamePlay")
		void CantUp();
		UFUNCTION(BlueprintCallable, Category = "GamePlay")
		void CantDown();
		UFUNCTION(BlueprintCallable, Category = "GamePlay")
		void CantRight();
		UFUNCTION(BlueprintCallable, Category = "GamePlay")
		void CantLeft();
		UFUNCTION(BlueprintCallable, Category = "GamePlay")
		void Clear();
		bool bCantRight=false;
		bool bCantLeft = false;
		bool bCantDown = false;
		bool bCantUp = false;
};
