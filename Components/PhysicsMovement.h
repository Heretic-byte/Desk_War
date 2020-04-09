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
protected:
	UPROPERTY()
	UPhysicsSkMeshComponent* m_MovingTarget;
	TArray<AActor*> m_AryTraceIgnoreActors;
	FVector m_InputNormal;
	FVector m_Acceleration;
	bool m_bOnGround;
	bool m_bPressedJump;
	FHitResult m_GroundHitResult;
	float m_fGroundDist;
	//jump
	int m_nJumpCurrentCount;
	//
	float m_fWalkableSlopeHeight;
	float m_fMaxTimeStep;
public:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool m_bShowDebug;
	UPROPERTY(EditDefaultsOnly, Category = "PhysicsMovement", meta = (ClampMin = "1", UIMin = "1"))
	float m_fMovingForce;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement_Rotate")
	FRotator m_RotationRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsMovement", meta = (ClampMin = "0", UIMin = "0"))
	float m_fWalkableSlopeAngle;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category = "PhysicsMovement_Jump", meta = (ClampMin = "0", UIMin = "0"))
	float m_fJumpZVelocity;
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

	virtual bool IsMovingOnGround()  const override;

	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	float GetMaxForce() const;
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	virtual void AddForce(FVector forceWant);
	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	virtual void AddImpulse(FVector impulseWant);
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
	void ResetJumpState();
	void CheckJumpInput(float delta);
	void ClearJumpInput(float delta);
	bool CanJump() ;
	virtual bool DoJump();
	void TickCastGround();
	void TickRotate(const FRotator rotateWant,float delta);
	void TickMovement(float delta);
	FRotator SelectTargetRotation(float delta);
	bool SetAccel(float DeltaTime);

private:
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
public:
	void SetMovingComponent(USceneComponent* NewUpdatedComponent,bool bRemoveIgnoreActorOld);
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PhysSceneStep(FPhysScene* PhysScene, float DeltaTime);
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
	

	float m_fAnalogInputModifier;
	void CalcVelocity(float DeltaTime, float Friction);

	UFUNCTION(BlueprintCallable, Category = "PhysicsMovement")
	float GetMinAnalogSpeed() const;
	
	float ComputeAnalogInputModifier() const;
	virtual float GetMaxSpeed() const override;
	void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration);

	FVector ComputeGroundMovementDelta(const FVector& Delta, const FHitResult& RampHit)const;

	bool IsWalkable(const FHitResult& Hit) const;

	virtual void SetVelocity(FVector& velocity,FHitResult& sweep);

	FVector SlideAlongOnSurface(const FVector& velocity, float deltaTime,float Time, const FVector& InNormal, FHitResult& Hit, bool bHandleImpact);

	bool SweepCanMove(FVector  delta, float deltaTime, FHitResult& OutHit);

	void PullBackHit(FHitResult& Hit, const FVector& Start, const FVector& End, const float Dist);

	virtual void UpdateComponentVelocity() override;

	virtual FVector ComputeSlideVector(const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const override;

	virtual FVector HandleSlopeBoosting(const FVector& SlideResult, const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const;

	void DrawVectorFromHead(FVector wantVector, float length, FColor color) const;

	FRotator m_OnGroundRampRot;
	FRotator m_OnAirTargetRot;

	UPrimitiveComponent* GetMovingTargetComponent() const;

	void ShowVelocityAccel();

	virtual void StopActiveMovement() override;

	void AddIgnoreTraceActor(AActor* actorWant);
	void RemoveIgnoreTraceActor(AActor* actorWant);

	void Landing();

	bool m_bIsFalling;

	bool m_WasWalkable;

	FCollisionShape m_CollShape;

	FCollisionShape MakeMovingTargetBox();
};
