

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Engine/EngineBaseTypes.h"
#include "Components/PhysicsSkMeshComponent.h"

#include "USBMovementComponent.generated.h"




USTRUCT(BlueprintType)
struct DESK_WAR_API FUSBFindFloorResult
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = CharacterFloor)
	uint32 bBlockingHit : 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = CharacterFloor)
	uint32 bWalkableFloor : 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = CharacterFloor)
	uint32 bLineTrace : 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = CharacterFloor)
	float FloorDist;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = CharacterFloor)
	float LineDist;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = CharacterFloor)
	FHitResult HitResult;

public:

	FUSBFindFloorResult()
		: bBlockingHit(false)
		, bWalkableFloor(false)
		, bLineTrace(false)
		, FloorDist(0.f)
		, LineDist(0.f)
		, HitResult(1.f)
	{
	}

	/** Returns true if the floor result hit a walkable surface. */
	bool IsWalkableFloor() const
	{
		return bBlockingHit && bWalkableFloor;
	}

	void Clear()
	{
		bBlockingHit = false;
		bWalkableFloor = false;
		bLineTrace = false;
		FloorDist = 0.f;
		LineDist = 0.f;
		HitResult.Reset(1.f, false);
	}

	/** Gets the distance to floor, either LineDist or FloorDist. */
	float GetDistanceToFloor() const
	{
		// When the floor distance is set using SetFromSweep, the LineDist value will be reset.
		// However, when SetLineFromTrace is used, there's no guarantee that FloorDist is set.
		return bLineTrace ? LineDist : FloorDist;
	}

	void SetFromSweep(const FHitResult& InHit, const float InSweepFloorDist, const bool bIsWalkableFloor);
	void SetFromLineTrace(const FHitResult& InHit, const float InSweepFloorDist, const float InLineDist, const bool bIsWalkableFloor);
};




UCLASS()
class DESK_WAR_API UUSBMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
public:
	UUSBMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
private:
	//my
	UPROPERTY()
	UPhysicsSkMeshComponent* m_MovingTarget;

	UPROPERTY()
	TArray<AActor*>* m_ptrAryTraceIgnoreActors;

	FVector m_vInputNormal;

	FUSBFindFloorResult CurrentFloor;
private:
	virtual void UpdateComponentVelocity() override;
	void AddIgnoreActorsToQuery(FCollisionQueryParams& queryParam);
	//
private://Ground Property
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "90.0", UIMin = "0.0", UIMax = "90.0"))
	float m_fWalkableFloorAngle;
	UPROPERTY(Category = "Character Movement: Walking", VisibleAnywhere)
	float m_fWalkableFloorZ;
	/** 250 The maximum ground speed when walking. Also determines maximum lateral speed when falling. */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MaxWalkSpeed;
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MaxAcceleration;
	/** 274 The ground speed that we should accelerate up to when walking at minimum analog stick tilt */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MinAnalogWalkSpeed;

private:
	UPROPERTY(Category = "Character Movement: m_eCurrentMovementMode", BlueprintReadOnly)
	TEnumAsByte<enum EMovementMode> m_eCurrentMovementMode;
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<enum EMovementMode> m_eDefaultLandMovementMode;
	UPROPERTY(Transient)
	TEnumAsByte<enum EMovementMode> m_eDefaultGroundMovementMode;
protected://pathfollow
	UPROPERTY()
	uint8 m_bMovementInProgress : 1;
	UPROPERTY(Transient)
	uint8 m_bHasRequestedVelocity : 1;
	UPROPERTY(Transient)
	uint8 m_bRequestedMoveWithMaxSpeed : 1;
public://gravity
	/** (Own) If enabled, Gravity is applied for the character. */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite)
	bool bEnableGravity;
	/** 177 Custom gravity scale. Gravity is multiplied by this amount for the character. */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite)
	float GravityScale;
public://jump
	/** 185 Initial velocity (instantaneous vertical acceleration) when jumping. */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Jump Z Velocity", ClampMin = "0", UIMin = "0"))
	float JumpZVelocity;
	/** 397
	 *	Apply gravity while the character is actively jumping (e.g. holding the jump key).
	 *	Helps remove frame-rate dependent jump height, but may alter base jump height.
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	uint8 bApplyGravityWhileJumping : 1;
protected://Friction
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float GroundFriction;
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float BrakingFrictionFactor;

	/** 388
	 * If true, BrakingFriction will be used to slow the character to a stop (when there is no m_vAcceleration).
	 * If false, braking uses the same friction passed to CalcVelocity() (ie GroundFriction when walking), multiplied by BrakingFrictionFactor.
	 * This setting applies to all movement modes; if only desired in certain modes, consider toggling it when movement modes change.
	 * @see BrakingFriction
	 */
	UPROPERTY(Category = "Character Movement (General Settings)", EditDefaultsOnly, BlueprintReadWrite)
	uint8 bUseSeparateBrakingFriction : 1;
protected://braiking
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float BrakingDecelerationWalking;

	/** 311
	 * Lateral deceleration when falling and not applying acceleration.
	 * @see MaxAcceleration
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float BrakingDecelerationFalling;

protected://Air
	/** 332
	 * When falling, amount of lateral movement control available to the character.
	 * 0 = no control, 1 = full control at max speed of MaxWalkSpeed.
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float m_fAirControl;

	/** 339
	 * When falling, multiplier applied to m_fAirControl when lateral velocity is less than AirControlBoostVelocityThreshold.
	 * Setting this to zero will disable air control boosting. Final result is clamped at 1.
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float AirControlBoostMultiplier;

	/** 346
	 * When falling, if lateral velocity magnitude is less than this value, m_fAirControl is multiplied by AirControlBoostMultiplier.
	 * Setting this to zero will disable air control boosting.
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float AirControlBoostVelocityThreshold;

	/** 353
	 * Friction to apply to lateral air movement when falling.
	 * If bUseSeparateBrakingFriction is false, also affects the ability to stop more quickly when braking (whenever m_vAcceleration is zero).
	 * @see BrakingFriction, bUseSeparateBrakingFriction
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float m_fFallingLateralFriction;

protected://General
	float m_fMass;
	/** 384 Change in rotation per second, used when UseControllerDesiredRotation or OrientRotationToMovement are true. Set a negative value for infinite rotation rate and instant turns. */
	UPROPERTY(Category = "Character Movement (Rotation Settings)", EditAnywhere, BlueprintReadWrite)
	FRotator RotationRate;
	UPROPERTY()
	FVector m_vAcceleration;
	UPROPERTY()
	FQuat m_qLastUpdateRotation;
	UPROPERTY()
	FVector m_vLastUpdateLocation;
	UPROPERTY()
	FVector m_vLastUpdateVelocity;
	UPROPERTY()
	FVector m_vPendingImpulseToApply;
	UPROPERTY()
	FVector m_vPendingForceToApply;
	UPROPERTY()
	float m_fAnalogInputModifier;

	/** 984
	 * Should use acceleration for path following?
	 * If true, acceleration is applied when path following to reach the target velocity.
	 * If false, path following velocity is set directly, disregarding acceleration.
	 */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	uint8 m_bRequestedMoveUseAcceleration : 1;

	UPROPERTY(Transient)
	FVector m_vRequestedVelocity;

	/** 1090 Temporarily holds launch velocity when pawn is to be launched so it happens at end of movement. */
	UPROPERTY()
	FVector m_vPendingLaunchVelocity;

private://magic number
	static const float MIN_TICK_TIME;
	static const float MIN_FLOOR_DIST;
	static const float MAX_FLOOR_DIST;
	static const float SWEEP_EDGE_REJECT_DISTANCE;
	static const float BRAKE_TO_STOP_VELOCITY;

protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode);

	void PhysWalking(float deltaTime);

	void PhysFalling(float deltaTime);

	/** 1780
	 * Adjusts velocity when walking so that Z velocity is zero.
	 * When bMaintainHorizontalGroundVelocity is false, also rescales the velocity vector to maintain the original magnitude, but in the horizontal direction.
	 */
	virtual void MaintainHorizontalGroundVelocity();

	/** 1832 감속 적용 Slows towards stop. */
	virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration);

	/** 1984 Enforce constraints on input given current state. For instance, don't move upwards if walking and looking up. */
	virtual FVector ConstrainInputAcceleration(const FVector& InputAcceleration) const;

	void ScaleInputAcceleration(const FVector& InputAcceleration);

	/** 2025 Perform movement on an autonomous client */
	virtual void PerformMovement(float DeltaTime);

public:
	UCapsuleComponent* GetBoundingCapsule() const;

	/** 662 Computes the analog input modifier based on current input vector and/or acceleration. */
	virtual float ComputeAnalogInputModifier() const;

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	virtual void SetMovementMode(EMovementMode NewMovementMode);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual float GetMaxSpeed() const override;
	virtual bool IsFalling() const override;
	virtual bool IsMovingOnGround() const override;
	virtual float GetGravityZ() const override;
	bool IsMovementInProgress() const { return m_bMovementInProgress; }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	virtual bool HasValidData() const;

	/** 1240 Transition from walking to falling */
	virtual void StartFalling(float timeTick, const FVector& Delta, const FVector& subLoc);

	/** 1255 Adjust distance from floor, trying to maintain a slight offset from the floor when walking (based on CurrentFloor). */
	virtual void AdjustFloorHeight();

	virtual void StartNewPhysics(float deltaTime);

	virtual bool DoJump();

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	virtual void Launch(FVector const& LaunchVel);

	virtual bool HandlePendingLaunch();

	/** 1317 Returns how far to rotate character during the time interval DeltaTime. */
	virtual FRotator GetDeltaRotation(float DeltaTime) const;

	/** 1320
	  * Compute a target rotation based on current movement. Used by PhysicsRotation() when bOrientRotationToMovement is true.
	  * Default implementation targets a rotation based on m_vAcceleration.
	  *
	  * @param CurrentRotation	- Current rotation of the Character
	  * @param DeltaTime		- Time slice for this movement
	  * @param DeltaRotation	- Proposed rotation change based simply on DeltaTime * RotationRate
	  *
	  * @return The target rotation given current movement.
	  */
	virtual FRotator ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const;

	/** 1332
	 * Use velocity requested by path following to compute a requested acceleration and speed.
	 * This does not affect the m_vAcceleration member variable, as that is used to indicate input acceleration.
	 * This may directly affect current Velocity.
	 *
	 * @param DeltaTime				Time slice for this operation
	 * @param MaxAccel				Max acceleration allowed in OutAcceleration result.
	 * @param MaxSpeed				Max speed allowed when computing OutRequestedSpeed.
	 * @param Friction				Current friction.
	 * @param BrakingDeceleration	Current braking deceleration.
	 * @param OutAcceleration		m_vAcceleration computed based on requested velocity.
	 * @param OutRequestedSpeed		Speed of resulting velocity request, which can affect the max speed allowed by movement.
	 * @return Whether there is a requested velocity and acceleration, resulting in valid OutAcceleration and OutRequestedSpeed values.
	 */
	virtual bool ApplyRequestedMove(float DeltaTime, float MaxAccel, float MaxSpeed, float Friction, float BrakingDeceleration, FVector& OutAcceleration, float& OutRequestedSpeed);

	/** 1351
	 * Compute new falling velocity from given velocity and gravity. Applies the limits of the current Physics Volume's TerminalVelocity.
	 */
	virtual FVector NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const;

	/** 1361
	 * Updates Velocity and m_vAcceleration based on the current state, applying the effects of friction and acceleration or deceleration. Does not apply gravity.
	 * This is used internally during movement updates. Normally you don't need to call this from outside code, but you might want to use it for custom movement modes.
	 *
	 * @param	DeltaTime						time elapsed since last frame.
	 * @param	Friction						coefficient of friction when not accelerating, or in the direction opposite acceleration.
	 * @param	bFluid							true if moving through a fluid, causing Friction to always be applied regardless of acceleration.
	 * @param	BrakingDeceleration				deceleration applied when not accelerating, or when exceeding max velocity.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration);

	/** 1373
	 *	Compute the max jump height based on the JumpZVelocity velocity and gravity.
	 *	This does not take into account the CharacterOwner's MaxJumpHoldTime.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual float GetMaxJumpHeight() const;

	/** 1380
	 *	Compute the max jump height based on the JumpZVelocity velocity and gravity.
	 *	This does take into account the CharacterOwner's MaxJumpHoldTime.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual float GetMaxJumpHeightWithJumpTime() const;

	/** 1387 Returns maximum acceleration for the current state. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual float GetMinAnalogSpeed() const;

	/** 1398 Returns maximum acceleration for the current state. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual float GetMaxAcceleration() const;

	/** 1402 Returns maximum deceleration for the current state when braking (ie when there is no acceleration). */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual float GetMaxBrakingDeceleration() const;

	/** 1406 Returns current acceleration, computed from input vector each update. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement", meta = (Keywords = "m_vAcceleration GetAcceleration"))
		FVector GetCurrentAcceleration() const;

	/** 1410 Returns modifier [0..1] based on the magnitude of the last input vector, which is used to modify the acceleration and max speed during movement. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		float GetAnalogInputModifier() const;


	/** 1440 Update the base of the character, which is the PrimitiveComponent we are standing on. */
	//virtual void SetBase(UPrimitiveComponent* NewBase, const FName BoneName = NAME_None, bool bNotifyActor = true);


	/** 1457 Applies momentum accumulated through AddImpulse() and AddForce(), then clears those forces. Does *not* use ClearAccumulatedForces() since that would clear pending launch velocity as well. */
	virtual void ApplyAccumulatedForces(float DeltaSeconds);

	/** 1460 Clears forces accumulated through AddImpulse() and AddForce(), and also pending launch velocity. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual void ClearAccumulatedForces();

	/** 1486 Handle falling movement. */
	virtual void PhysFalling(float deltaTime, int32 Iterations);

	// Helpers for PhysFalling

	/** 1491
	 * Get the lateral acceleration to use during falling movement. The Z component of the result is ignored.
	 * Default implementation returns current m_vAcceleration value modified by GetAirControl(), with Z component removed,
	 * with magnitude clamped to GetMaxAcceleration().
	 * This function is used internally by PhysFalling().
	 *
	 * @param DeltaTime Time step for the current update.
	 * @return m_vAcceleration to use during falling movement.
	 */
	virtual FVector GetFallingLateralAcceleration(float DeltaTime);

	/** 1502
	 * Get the air control to use during falling movement.
	 * Given an initial air control (TickAirControl), applies the result of BoostAirControl().
	 * This function is used internally by GetFallingLateralAcceleration().
	 *
	 * @param DeltaTime			Time step for the current update.
	 * @param TickAirControl	Current air control value.
	 * @param FallAcceleration	m_vAcceleration used during movement.
	 * @return Air control to use during falling movement.
	 * @see m_fAirControl, BoostAirControl(), LimitAirControl(), GetFallingLateralAcceleration()
	 */
	virtual FVector GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration);

protected:

	/** 1517
	 * Increase air control if conditions of AirControlBoostMultiplier and AirControlBoostVelocityThreshold are met.
	 * This function is used internally by GetAirControl().
	 *
	 * @param DeltaTime			Time step for the current update.
	 * @param TickAirControl	Current air control value.
	 * @param FallAcceleration	m_vAcceleration used during movement.
	 * @return Modified air control to use during falling movement
	 * @see GetAirControl()
	 */
	virtual float BoostAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration);

	/** 1529
	 * Limits the air control to use during falling movement, given an impact while falling.
	 * This function is used internally by PhysFalling().
	 *
	 * @param DeltaTime			Time step for the current update.
	 * @param FallAcceleration	m_vAcceleration used during movement.
	 * @param HitResult			Result of impact.
	 * @param bCheckForValidLandingSpot If true, will use IsValidLandingSpot() to determine if HitResult is a walkable surface. If false, this check is skipped.
	 * @return Modified air control acceleration to use during falling movement.
	 * @see PhysFalling()
	 */
	virtual FVector LimitAirControl(float DeltaTime, const FVector& FallAcceleration, const FHitResult& HitResult, bool bCheckForValidLandingSpot);


	/** 1543 Handle landing against Hit surface over remaingTime and iterations, calling SetPostLandedPhysics() and starting the new movement mode. */
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime);

	/** 1546 Use new physics after landing. Defaults to swimming if in water, walking otherwise. */
	virtual void SetPostLandedPhysics(const FHitResult& Hit);

public:

	/** 1575 Called by owning Character upon successful teleport from AActor::TeleportTo(). */
	virtual void OnTeleported() override;


	/** 1603 Check if pawn is falling */
	virtual bool CheckFall(const FUSBFindFloorResult& OldFloor, const FHitResult& Hit, const FVector& Delta, const FVector& OldLocation,float timeTick, bool bMustJump);

	void RevertMove(const FVector& OldLocation, bool bFailMove);

	/** 1612 Perform rotation over deltaTime */
	virtual void PhysicsRotation(float DeltaTime);

	/** 1615 if true, DesiredRotation will be restricted to only Yaw component in PhysicsRotation() */
	virtual bool ShouldRemainVertical() const;

	/** 1621 Set movement mode to the default based on the current physics volume. */
	virtual void SetDefaultMovementMode();

	// 1640
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

	/** 1645
	 * Add impulse to character. Impulses are accumulated each tick and applied together
	 * so multiple calls to this function will accumulate.
	 * An impulse is an instantaneous force, usually applied once. If you want to continually apply
	 * forces each frame, use AddForce().
	 * Note that changing the momentum of characters like this can change the movement mode.
	 *
	 * @param	Impulse				Impulse to apply.
	 * @param	bVelocityChange		Whether or not the impulse is relative to mass.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual void AddImpulse(FVector Impulse, bool bVelocityChange = false);

	/** 1658
	 * Add force to character. Forces are accumulated each tick and applied together
	 * so multiple calls to this function will accumulate.
	 * Forces are scaled depending on timestep, so they can be applied each frame. If you want an
	 * instantaneous force, use AddImpulse.
	 * Adding a force always takes the actor's mass into account.
	 * Note that changing the momentum of characters like this can change the movement mode.
	 *
	 * @param	Force			Force to apply.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual void AddForce(FVector Force);

	/** 1703 Return true if the hit result should be considered a walkable surface for the character. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual bool IsWalkable(const FHitResult& Hit) const;

	/** 1707 Get the max angle in degrees of a walkable surface for the character. */
	FORCEINLINE float GetWalkableFloorAngle() const { return m_fWalkableFloorAngle; }

	/** 1714 Set the max angle in degrees of a walkable surface for the character. Also computes m_fWalkableFloorZ. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		void SetWalkableFloorAngle(float InWalkableFloorAngle);

	/** 1718 Get the Z component of the normal of the steepest walkable surface for the character. Any lower than this and it is not walkable. */
	FORCEINLINE float GetWalkableFloorZ() const { return m_fWalkableFloorZ; }

	/** 1725 Set the Z component of the normal of the steepest walkable surface for the character. Also computes m_fWalkableFloorAngle. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		void SetWalkableFloorZ(float InWalkableFloorZ);


	/** 1755
	 * Compute a vector of movement, given a delta and a hit result of the surface we are on.
	 *
	 * @param Delta:				Attempted movement direction
	 * @param RampHit:				Hit result of sweep that found the ramp below the capsule
	 * @param bHitFromLineTrace:	Whether the floor trace came from a line trace
	 *
	 * @return If on a walkable surface, this returns a vector that moves parallel to the surface. The magnitude may be scaled if bMaintainHorizontalGroundVelocity is true.
	 * If a ramp vector can't be computed, this will just return Delta.
	*/
	virtual FVector ComputeGroundMovementDelta(const FVector& Delta, const FHitResult& RampHit, const bool bHitFromLineTrace) const;

	/** 1767
	 * Move along the floor, using CurrentFloor and ComputeGroundMovementDelta() to get a movement direction.
	 * If a second walkable surface is hit, it will also be moved along using the same approach.
	 *
	 * @param InVelocity:			Velocity of movement
	 * @param DeltaSeconds:			Time over which movement occurs
	 * @param OutStepDownResult:	[Out] If non-null, and a floor check is performed, this will be updated to reflect that result.
	 */
	virtual void MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds);


	/** 1803 Custom version of SlideAlongSurface that handles different movement modes separately; namely during walking physics we might not want to slide up slopes. */
	virtual float SlideAlongSurface(const FVector& Delta, float Time, const FVector& Normal, FHitResult& Hit, bool bHandleImpact) override;


	/** 1809
	 * Calculate slide vector along a surface.
	 * Has special treatment when falling, to avoid boosting up slopes (calling HandleSlopeBoosting() in this case).
	 *
	 * @param Delta:	Attempted move.
	 * @param Time:		Amount of move to apply (between 0 and 1).
	 * @param Normal:	Normal opposed to movement. Not necessarily equal to Hit.Normal (but usually is).
	 * @param Hit:		HitResult of the move that resulted in the slide.
	 * @return			New deflected vector of movement.
	 */
	virtual FVector ComputeSlideVector(const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const override;

	/** 1821
	 * Limit the slide vector when falling if the resulting slide might boost the character faster upwards.
	 * @param SlideResult:	Vector of movement for the slide (usually the result of ComputeSlideVector)
	 * @param Delta:		Original attempted move
	 * @param Time:			Amount of move to apply (between 0 and 1).
	 * @param Normal:		Normal opposed to movement. Not necessarily equal to Hit.Normal (but usually is).
	 * @param Hit:			HitResult of the move that resulted in the slide.
	 * @return:				New slide result.
	 */
	virtual FVector HandleSlopeBoosting(const FVector& SlideResult, const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const;

	/** 1838
	 * Return true if the 2D distance to the impact point is inside the edge tolerance (CapsuleRadius minus a small rejection threshold).
	 * Useful for rejecting adjacent hits when finding a floor or landing spot.
	 */
	virtual bool IsWithinEdgeTolerance(const FVector& CapsuleLocation, const FVector& TestImpactPoint, const float CapsuleRadius) const;

	/** 1844
	 * Sweeps a vertical trace to find the floor for the capsule at the given location. Will attempt to perch if ShouldComputePerchResult() returns true for the downward sweep result.
	 * No floor will be found if collision is disabled on the capsule!
	 *
	 * @param CapsuleLocation		Location where the capsule sweep should originate
	 * @param OutFloorResult		[Out] Contains the result of the floor check. The HitResult will contain the valid sweep or line test upon success, or the result of the sweep upon failure.
	 * @param bCanUseCachedLocation If true, may use a cached value (can be used to avoid unnecessary floor tests, if for example the capsule was not moving since the last test).
	 * @param DownwardSweepResult	If non-null and it contains valid blocking hit info, this will be used as the result of a downward sweep test instead of doing it as part of the update.
	 */
	virtual void FindFloor(const FVector& CapsuleLocation, FUSBFindFloorResult& OutFloorResult, bool bCanUseCachedLocation, const FHitResult* DownwardSweepResult = NULL) const;


	/** 1865
	 * Compute distance to the floor from bottom sphere of capsule and store the result in OutFloorResult.
	 * This distance is the swept distance of the capsule to the first point impacted by the lower hemisphere, or distance from the bottom of the capsule in the case of a line trace.
	 * This function does not care if collision is disabled on the capsule (unlike FindFloor).
	 * @see FindFloor
	 *
	 * @param CapsuleLocation:	Location of the capsule used for the query
	 * @param LineDistance:		If non-zero, max distance to test for a simple line check from the capsule base. Used only if the sweep test fails to find a walkable floor, and only returns a valid result if the impact normal is a walkable normal.
	 * @param SweepDistance:	If non-zero, max distance to use when sweeping a capsule downwards for the test. MUST be greater than or equal to the line distance.
	 * @param OutFloorResult:	Result of the floor check. The HitResult will contain the valid sweep or line test upon success, or the result of the sweep upon failure.
	 * @param SweepRadius:		The radius to use for sweep tests. Should be <= capsule radius.
	 * @param DownwardSweepResult:	If non-null and it contains valid blocking hit info, this will be used as the result of a downward sweep test instead of doing it as part of the update.
	 */
	virtual void ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance, FUSBFindFloorResult& OutFloorResult, float SweepRadius, const FHitResult* DownwardSweepResult = NULL) const;


	/** 1894
	 * Sweep against the world and return the first blocking hit.
	 * Intended for tests against the floor, because it may change the result of impacts on the lower area of the test (especially if bUseFlatBaseForFloorChecks is true).
	 *
	 * @param OutHit			First blocking hit found.
	 * @param Start				Start location of the capsule.
	 * @param End				End location of the capsule.
	 * @param TraceChannel		The 'channel' that this trace is in, used to determine which components to hit.
	 * @param CollisionShape	Capsule collision shape.
	 * @param Params			Additional parameters used for the trace.
	 * @param ResponseParam		ResponseContainer to be used for this trace.
	 * @return True if OutHit contains a blocking hit entry.
	 */
	virtual bool FloorSweepTest(
		struct FHitResult& OutHit,
		const FVector& Start,
		const FVector& End,
		ECollisionChannel TraceChannel,
		float radius,float halfHeight,
		const struct FCollisionQueryParams& Params,
		const struct FCollisionResponseParams& ResponseParam
	) const;

	/** 1917 Verify that the supplied hit result is a valid landing spot when falling. */
	virtual bool IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const;

	/** 1920
	 * Determine whether we should try to find a valid landing spot after an impact with an invalid one (based on the Hit result).
	 * For example, landing on the lower portion of the capsule on the edge of geometry may be a walkable surface, but could have reported an unwalkable impact normal.
	 */
	virtual bool ShouldCheckForValidLandingSpot(float DeltaTime, const FVector& Delta, const FHitResult& Hit) const;

protected:
	// 1956 Enum used to control GetPawnCapsuleExtent behavior
	enum EShrinkCapsuleExtent
	{
		SHRINK_None,			// Don't change the size of the capsule
		SHRINK_RadiusCustom,	// Change only the radius, based on a supplied param
		SHRINK_HeightCustom,	// Change only the height, based on a supplied param
		SHRINK_AllCustom,		// Change both radius and height, based on a supplied param
	};

	/** 1965 Get the capsule extent for the Pawn owner, possibly reduced in size depending on ShrinkMode.
	 * @param ShrinkMode			Controls the way the capsule is resized.
	 * @param CustomShrinkAmount	The amount to shrink the capsule, used only for ShrinkModes that specify custom.
	 * @return The capsule extent of the Pawn owner, possibly reduced in size depending on ShrinkMode.
	 */
	FVector GetPawnCapsuleExtent(const EShrinkCapsuleExtent ShrinkMode, const float CustomShrinkAmount = 0.f) const;

	/** 1972 Get the collision shape for the Pawn owner, possibly reduced in size depending on ShrinkMode.
	 * @param ShrinkMode			Controls the way the capsule is resized.
	 * @param CustomShrinkAmount	The amount to shrink the capsule, used only for ShrinkModes that specify custom.
	 * @return The capsule extent of the Pawn owner, possibly reduced in size depending on ShrinkMode.
	 */
	FCollisionShape GetPawnCapsuleCollisionShape(const EShrinkCapsuleExtent ShrinkMode, const float CustomShrinkAmount = 0.f) const;

public:
	/** 2174 React to instantaneous change in position. Invalidates cached floor recomputes it if possible if there is a current movement base. */
	virtual void UpdateFloorFromAdjustment();
public:
	/** When true, player wants to jump */
	UPROPERTY(BlueprintReadOnly, Category = "CharacterJump")
	uint32 bPressedJump : 1;

	/** Tracks whether or not the character was already jumping last frame. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "CharacterJump")
	uint32 bWasJumping : 1;

	/**
	 * Jump key Held Time.
	 * This is the time that the player has held the jump key, in seconds.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, VisibleInstanceOnly, Category = "CharacterJump")
	float JumpKeyHoldTime;

	/** Amount of jump force time remaining, if JumpMaxHoldTime > 0. */
	UPROPERTY(Transient, BlueprintReadOnly, VisibleInstanceOnly, Category = "CharacterJump")
	float JumpForceTimeRemaining;

	/**
	 * The max time the jump key can be held.
	 * Note that if StopJumping() is not called before the max jump hold time is reached,
	 * then the character will carry on receiving vertical velocity. Therefore it is usually
	 * best to call StopJumping() when jump input has ceased (such as a button up event).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterJump", Meta = (ClampMin = 0.0, UIMin = 0.0))
	float JumpMaxHoldTime;

	/**
	 * The max number of jumps the character can perform.
	 * Note that if JumpMaxHoldTime is non zero and StopJumping is not called, the player
	 * may be able to perform and unlimited number of jumps. Therefore it is usually
	 * best to call StopJumping() when jump input has ceased (such as a button up event).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterJump")
	int32 JumpMaxCount;

	/**
	 * Tracks the current number of jumps performed.
	 * This is incremented in CheckJumpInput, used in CanJump_Implementation, and reset in OnMovementModeChanged.
	 * When providing overrides for these methods, it's recommended to either manually
	 * increment / reset this value, or call the Super:: method.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "CharacterJump")
	int32 JumpCurrentCount;

	/**
	 * Make the character jump on the next update.
	 * If you want your character to jump according to the time that the jump key is held,
	 * then you can set JumpKeyHoldTime to some non-zero value. Make sure in this case to
	 * call StopJumping() when you want the jump's z-velocity to stop being applied (such
	 * as on a button up event), otherwise the character will carry on receiving the
	 * velocity until JumpKeyHoldTime is reached.
	 */
	UFUNCTION(BlueprintCallable, Category = "CharacterJump")
	virtual void Jump();

	/**
	 * Stop the character from jumping on the next update.
	 * Call this from an input event (such as a button 'up' event) to cease applying
	 * jump Z-velocity. If this is not called, then jump z-velocity will be applied
	 * until JumpMaxHoldTime is reached.
	 */
	UFUNCTION(BlueprintCallable, Category = "CharacterJump")
	virtual void StopJumping();

	/**
	 * Check if the character can jump in the current state.
	 *
	 * The default implementation may be overridden or extended by implementing the custom CanJump event in Blueprints.
	 *
	 * @Return Whether the character can jump in the current state.
	 */
	UFUNCTION(BlueprintCallable, Category = "CharacterJump")
	bool CanJump() const;

public:
	/** Marks character as not trying to jump */
	virtual void ResetJumpState();

	/**
	 * True if jump is actively providing a force, such as when the jump key is held and the time it has been held is less than JumpMaxHoldTime.
	 * @see CharacterMovement->IsFalling
	 */
	UFUNCTION(BlueprintCallable, Category = "CharacterJump")
	virtual bool IsJumpProvidingForce() const;
	/** Trigger jump if jump button has been pressed. */
	virtual void CheckJumpInput(float DeltaTime);
	/** Update jump input state after having checked input. */
	virtual void ClearJumpInput(float DeltaTime);
	/**
	 * Get the maximum jump time for the character.
	 * Note that if StopJumping() is not called before the max jump hold time is reached,
	 * then the character will carry on receiving vertical velocity. Therefore it is usually
	 * best to call StopJumping() when jump input has ceased (such as a button up event).
	 *
	 * @return Maximum jump time for the character
	 */
	virtual float GetJumpMaxHoldTime() const;

private:
	void SetPhysicalVelocity(FVector& velo,FHitResult* outSweep);
	bool Sweep(UPrimitiveComponent * Prim, FVector & velo);
	void PullBackHit(FHitResult& Hit, const FVector& Start, const FVector& End, const float Dist);
};
