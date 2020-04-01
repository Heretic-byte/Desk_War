// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Datas/USB_Macros.h"
#include "Components/PhysicsSkMeshComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "CollisionQueryParams.h"
#include "PhysicsMovement.generated.h"

UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class DESK_WAR_API UPhysicsMovement : public UPawnMovementComponent
{
	GENERATED_BODY()
public:
	UPhysicsMovement(const FObjectInitializer& objInit);
private:
	UPROPERTY()
	UPhysicsSkMeshComponent* m_MovingTarget;
	UPROPERTY()
	UPhysicsSkMeshComponent* m_MovingTargetTail;
	TArray<AActor*>* m_ptrAryTraceIgnoreActors;
public:
	FVoidVoid m_OnAutoMoveEnd;
	FVoidVoid m_OnAutoRotateEnd;
private:
	FVector m_AutoMoveDir;
	FRotator m_AutoRotateRot;
	FVector m_vInputNormal;
	FVector m_Acceleration;
	bool m_bOnGround;
	bool m_bPressedJump;
	FHitResult m_GroundHitResult;
	float m_fGroundDist;
	//jump
	bool m_bWasJumping;
	float m_fJumpKeyHoldTime;
	int m_nJumpCurrentCount;
	float m_fJumpForceTimeRemaining;
	float m_fAdditionalTraceMultipleLength;
	float m_fInitHeadMass;
private://auto move,block move
	bool m_bIsWallBlocking;
	bool m_bAutoMove;
	bool m_bBlockMove;
	float m_fBlockMoveTime;
	float m_fBlockMoveTimer;
private://auto rot
	bool m_bAutoRot;
	float m_fAutoRotTime;
	float m_fAutoRotTimer;
private://rotflip
	float m_fInitDesiredRotRollDelta;
public:
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement")
	bool m_bDebugShowForwardCast;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement", meta = (ClampMin = "1", UIMin = "1"))
	float m_fGroundCastBoxSize;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement", meta = (ClampMax = "0", UIMax = "0"))
	float m_fGroundCastOffset;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement", meta = (ClampMin = "1", UIMin = "1"))
	float m_fMovingForce;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement_Rotate")
	FRotator m_RotationRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_fWalkableSlopeAngle;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_fWalkableSlopeHeight;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement_Jump", meta = (ClampMin = "0", UIMin = "0"))
	float m_fJumpZVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement_Jump", meta = (ClampMin = "0", UIMin = "0"))
	float m_fMaxHoldTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement_Jump", meta = (ClampMin = "0", UIMin = "0"))
	int m_nJumpMaxCount;
	UPROPERTY(Category = "PhysicsMovement_Jump", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float m_fAirControl;
public:
	UPROPERTY(BlueprintAssignable, Category = "PhysicsMovement_Jump")
	FVoidVoidBP m_OnJumpBP;
	FVoidVoid m_OnJump;

public:
	UFUNCTION(BlueprintCallable,Category="PhysicsMovement_Jump")
	void Jump();
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement_Jump")
	void StopJumping();

	//virtual bool IsMovingOnGround() override const;
	virtual bool IsMovingOnGround()  const override;

	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	float GetMaxForce() const;
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	void AddForce(FVector forceWant);
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	void AddImpulse(FVector impulseWant);
private:
	void SetWalkableFloorAngle(float InWalkableFloorAngle);
	void SetAccelerationByDir(const FVector inputPure);
	float GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime) const;
	FRotator GetDeltaRotation(float DeltaTime) const;
	void AddIgnoreActorsToQuery(FCollisionQueryParams& queryParam);
protected:
	virtual bool IsFalling()  const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	void ResetJumpState();
	void CheckJumpInput(float delta);
	void ClearJumpInput(float delta);
	bool CanJump() const;
	bool DoJump();
private:
	void TickCastGround();
	void TickCastFlipCheck();
	void TickRotate(const FRotator rotateWant,float delta);
	void TickMovement(float delta);
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	FRotator SelectTargetRotation(float delta);
	bool SetAccel(float DeltaTime);
public:
	void EnableInputMove();
	void DisableInputMove(float timeWant);
	void EnableAutoMove(FVector dirWant, float timeWant);
	void DisableAutoMove();
	void EnableAutoRotate(FRotator rotWant, float timeWant);
	void DisableAutoRotate();
	void SetUpdatePhysicsMovement(UPhysicsSkMeshComponent* headUpdatedCompo, UPhysicsSkMeshComponent* tailUpdatedCompo);
	void SetCastingLength(UPhysicsSkMeshComponent * headUpdatedCompo);
	void SetTraceIgnoreActorAry(TArray<AActor*>* aryWant);
	void SetDamping(float fLinDamp, float fAngDamp);
	void SetInitHeadMass(float massHead);
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void PhysSceneStep(FPhysScene* PhysScene, float DeltaTime);
	FDelegateHandle OnPhysSceneStepHandle;
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_fMaxBrakingDeceleration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_fMaxSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_fMinAnalogSpeed;
	UPROPERTY(Category = "PhysicsMovement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float m_fGroundFriction;
	UPROPERTY(Category = "PhysicsMovement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "30", UIMin = "30"))
	float m_fMaxTimeStep;

	float m_fAnalogInputModifier;
	void CalcVelocity(float DeltaTime, float Friction);

	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	float GetMinAnalogSpeed() const;
	
	float ComputeAnalogInputModifier() const;
	virtual float GetMaxSpeed() const override;
	void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration);

	FVector ComputeGroundMovementDelta(const FVector& Delta, const FHitResult& RampHit)const;

	bool IsWalkable(const FHitResult& Hit) const;

	void SetVelocity(FVector& velocity,FHitResult& sweep,float delta);

	FVector SlideAlongOnSurface(const FVector& velocity, float deltaTime,float Time, const FVector& InNormal, FHitResult& Hit, bool bHandleImpact);

	bool SweepCanMove(FVector  delta, float deltaTime, FHitResult& OutHit, float offset = 0.03f);

	void PullBackHit(FHitResult& Hit, const FVector& Start, const FVector& End, const float Dist);

	virtual void UpdateComponentVelocity() override;

	virtual FVector ComputeSlideVector(const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const override;

	virtual FVector HandleSlopeBoosting(const FVector& SlideResult, const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const;


	void DrawVectorFromHead(FVector wantVector, float length, FColor color) const;

	FRotator TargetRot;

	UPrimitiveComponent* GetMovingTargetComponent() const;
};
