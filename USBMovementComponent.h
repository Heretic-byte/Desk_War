// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"	// PawnMovementComponent.h 포함
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Engine/EngineBaseTypes.h"
#include "USBMovementComponent.generated.h"

/**
* CharacterMovementComponent => FFindFloorResult 구조체를 따로 정의하지 않고 CharacterMovementComponent에 있는걸 사용하기로 했음
**/


/**
 * Tick function that calls UUSBMovementComponent::PostPhysicsTickComponent
 **/
USTRUCT()
struct FUSBMovementComponentPostPhysicsTickFunction : public FTickFunction
{
	GENERATED_USTRUCT_BODY()

		/** CharacterMovementComponent that is the target of this tick **/
		class UUSBMovementComponent* Target;

	/**
	 * Abstract function actually execute the tick.
	 * @param DeltaTime - frame time to advance, in seconds
	 * @param TickType - kind of tick for this frame
	 * @param CurrentThread - thread we are executing on, useful to pass along as new tasks are created
	 * @param MyCompletionGraphEvent - completion event for this task. Useful for holding the completion of this task until certain child tasks are complete.
	 **/
	virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;

	/** Abstract function to describe this tick. Used to print messages about illegal cycles in the dependency graph **/
	virtual FString DiagnosticMessage() override;
	/** Function used to describe this tick for active tick reporting. **/
	virtual FName DiagnosticContext(bool bDetailed) override;
};

template<>
struct TStructOpsTypeTraits<FUSBMovementComponentPostPhysicsTickFunction> : public TStructOpsTypeTraitsBase2<FUSBMovementComponentPostPhysicsTickFunction>
{
	enum
	{
		WithCopy = false
	};
};

UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class DESK_WAR_API UUSBMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()







public:

	/**
	 * 생성자
	 */
	UUSBMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	// FVector 내장함수 중 내부에 FMasth::Invsqrt 함수가 해당 비주얼 스튜디오 버전에서 정상적으로 컴파일 되지 않아 직접 만들었음
	// GetClampedToMaxSize, GetSfateNormal, GetSfateNormal2D와 기능은 동일함.
	FVector CustomGetClampedToMaxSize(FVector Vect, float MaxSize) const;
	FVector CustomGetSafeNormal(FVector Vect, float Tolerance = 1.e-8f) const;
	FVector CustomGetSafeNormal2D(FVector Vect, float Tolerance = 1.e-8f) const;

	
	

	UFUNCTION(BlueprintImplementableEvent)
	void OnLandedGround(const FHitResult& Hit);



	/** 
	 * bOrientRotationToMovement가 True일 때 변수 값이 True라면 Z축(Yaw)에 180을 더하여준다.
	 * 2개의 머리를 사용하는 USB가 올바른 방향으로 회전하도록 추가하였음
	 */
	UPROPERTY(Category = "Character Movement (Rotation Settings)", EditAnywhere, BlueprintReadWrite)
		uint8 bReverseRotationYaw : 1;





	/****************

		 Struct

	****************/

	/** 1417 Struct updated by StepUp() to return result of final step down, if applicable. */
	struct FStepDownResult
	{
		uint32 bComputedFloor : 1;		// True if the floor was computed as a result of the step down.
		FFindFloorResult FloorResult;	// The result of the floor test if the floor was updated.

		FStepDownResult()
			: bComputedFloor(false)
		{
		}
	};

	/****************

		Property

	****************/

private:

	/** 195
	 * Max angle in degrees of a walkable surface. Any greater than this and it is too steep to be walkable.
	 */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "90.0", UIMin = "0.0", UIMax = "90.0"))
	float WalkableFloorAngle;

	/** 201
	 * Minimum Z value for floor normal. If less, not a walkable surface. Computed from WalkableFloorAngle.
	 */
	UPROPERTY(Category = "Character Movement: Walking", VisibleAnywhere)
	float WalkableFloorZ;

	// 426 Tracks whether or not we need to update the bSweepWhileNavWalking flag do to an upgrade.
	uint8 bNeedsSweepWhileWalkingUpdate : 1;


	/** 862
	 * Ground movement mode to switch to after falling and resuming ground movement.
	 * Only allowed values are: MOVE_Walking, MOVE_NavWalking.
	 * @see SetGroundMovementMode(), GetGroundMovementMode()
	 */
	UPROPERTY(Transient)
	TEnumAsByte<enum EMovementMode> GroundMovementMode;


protected:

	/** 431
	 * True during movement update.
	 * Used internally so that attempts to change CharacterOwner and UpdatedComponent are deferred until after an update.
	 * @see IsMovementInProgress()
	 */
	UPROPERTY()
	uint8 bMovementInProgress : 1;

	// 1004 AI PATH FOLLOWING

	/** Was velocity requested by path following? */
	UPROPERTY(Transient)
		uint8 bHasRequestedVelocity : 1;

	/** Was acceleration requested to be always max speed? */
	UPROPERTY(Transient)
		uint8 bRequestedMoveWithMaxSpeed : 1;


	/** Flag set in pre-physics update to indicate that based movement should be updated post-physics */
	uint8 bDeferUpdateBasedMovement : 1;

	/** Whether to raycast to underlying geometry to better conform navmesh-walking characters */
	UPROPERTY(Category = "Character Movement: NavMesh Movement", EditAnywhere, BlueprintReadOnly)
		uint8 bProjectNavMeshWalking : 1;

	/** Use both WorldStatic and WorldDynamic channels for NavWalking geometry conforming */
	UPROPERTY(Category = "Character Movement: NavMesh Movement", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
		uint8 bProjectNavMeshOnBothWorldChannels : 1;



public:
	
	/** (Own) If enabled, Gravity is applied for the character. */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite)
		bool bEnableGravity;

	/** 177 Custom gravity scale. Gravity is multiplied by this amount for the character. */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite)
		float GravityScale;

	/** 181 Maximum height character can step up */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float MaxStepHeight;

	/** 185 Initial velocity (instantaneous vertical acceleration) when jumping. */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Jump Z Velocity", ClampMin = "0", UIMin = "0"))
		float JumpZVelocity;

	/** 189 Fraction of JumpZVelocity to use when automatically "jumping off" of a base actor that's not allowed to be a base for a character. (For example, if you're not allowed to stand on other players.) */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ClampMin = "0", UIMin = "0"))
		float JumpOffJumpZFactor;

	/** 209
	 * Actor's current movement mode (walking, falling, etc).
	 *    - walking:  Walking on a surface, under the effects of friction, and able to "step up" barriers. Vertical velocity is zero.
	 *    - falling:  Falling under the effects of gravity, after jumping or walking off the edge of a surface.
	 *    - flying:   Flying, ignoring the effects of gravity.
	 *    - swimming: Swimming through a fluid volume, under the effects of gravity and buoyancy.
	 *    - custom:   User-defined custom movement mode, including many possible sub-modes.
	 * This is automatically replicated through the Character owner and for client-server movement functions.
	 * @see SetMovementMode(), CustomMovementMode
	 */
	UPROPERTY(Category = "Character Movement: MovementMode", BlueprintReadOnly)
		TEnumAsByte<enum EMovementMode> MovementMode;

	/** 222
	 * Current custom sub-mode if MovementMode is set to Custom.
	 * This is automatically replicated through the Character owner and for client-server movement functions.
	 * @see SetMovementMode()
	 */
	UPROPERTY(Category = "Character Movement: MovementMode", BlueprintReadOnly)
		uint8 CustomMovementMode;

	/** 234
	 * Setting that affects movement control. Higher values allow faster changes in direction.
	 * If bUseSeparateBrakingFriction is false, also affects the ability to stop more quickly when braking (whenever Acceleration is zero), where it is multiplied by BrakingFrictionFactor.
	 * When braking, this property allows you to control how much friction is applied when moving across the ground, applying an opposing force that scales with current velocity.
	 * This can be used to simulate slippery surfaces such as ice or oil by changing the value (possibly based on the material pawn is standing on).
	 * @see BrakingDecelerationWalking, BrakingFriction, bUseSeparateBrakingFriction, BrakingFrictionFactor
	 */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float GroundFriction;

	/** 244 Saved location of object we are standing on, for UpdateBasedMovement() to determine if base moved in the last frame, and therefore pawn needs an update. */
	FQuat OldBaseQuat;

	/** 247 Saved location of object we are standing on, for UpdateBasedMovement() to determine if base moved in the last frame, and therefore pawn needs an update. */
	FVector OldBaseLocation;

	/** 250 The maximum ground speed when walking. Also determines maximum lateral speed when falling. */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float MaxWalkSpeed;

	/** 254 The maximum ground speed when walking and crouched. */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float MaxWalkSpeedCrouched;

	/** 258 The maximum swimming speed. */
	UPROPERTY(Category = "Character Movement: Swimming", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float MaxSwimSpeed;

	/** 262 The maximum flying speed. */
	UPROPERTY(Category = "Character Movement: Flying", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float MaxFlySpeed;

	/** 266 The maximum speed when using Custom movement mode. */
	UPROPERTY(Category = "Character Movement: Custom Movement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float MaxCustomMovementSpeed;

	/** 270 Max Acceleration (rate of change of velocity) */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float MaxAcceleration;

	/** 274 The ground speed that we should accelerate up to when walking at minimum analog stick tilt */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float MinAnalogWalkSpeed;

	/** 278
	 * Factor used to multiply actual value of friction used when braking.
	 * This applies to any friction value that is currently used, which may depend on bUseSeparateBrakingFriction.
	 * @note This is 2 by default for historical reasons, a value of 1 gives the true drag equation.
	 * @see bUseSeparateBrakingFriction, GroundFriction, BrakingFriction
	 */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float BrakingFrictionFactor;

	/** 287
	 * Friction (drag) coefficient applied when braking (whenever Acceleration = 0, or if character is exceeding max speed); actual value used is this multiplied by BrakingFrictionFactor.
	 * When braking, this property allows you to control how much friction is applied when moving across the ground, applying an opposing force that scales with current velocity.
	 * Braking is composed of friction (velocity-dependent drag) and constant deceleration.
	 * This is the current value, used in all movement modes; if this is not desired, override it or bUseSeparateBrakingFriction when movement mode changes.
	 * @note Only used if bUseSeparateBrakingFriction setting is true, otherwise current friction such as GroundFriction is used.
	 * @see bUseSeparateBrakingFriction, BrakingFrictionFactor, GroundFriction, BrakingDecelerationWalking
	 */
	 // 공기저항이 없는 게임월드에 마찰과 별도의 감속을 주기위한 변수
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", EditCondition = "bUseSeparateBrakingFriction"))
		float BrakingFriction;

	/** 298
	 * Time substepping when applying braking friction. Smaller time steps increase accuracy at the slight cost of performance, especially if there are large frame times.
	 */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ClampMin = "0.0166", ClampMax = "0.05", UIMin = "0.0166", UIMax = "0.05"))
		float BrakingSubStepTime;

	/** 304
	 * Deceleration when walking and not applying acceleration. This is a constant opposing force that directly lowers velocity by a constant value.
	 * @see GroundFriction, MaxAcceleration
	 */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float BrakingDecelerationWalking;

	/** 311
	 * Lateral deceleration when falling and not applying acceleration.
	 * @see MaxAcceleration
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float BrakingDecelerationFalling;

	/** 318
	 * Deceleration when swimming and not applying acceleration.
	 * @see MaxAcceleration
	 */
	UPROPERTY(Category = "Character Movement: Swimming", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float BrakingDecelerationSwimming;

	/** 325
	 * Deceleration when flying and not applying acceleration.
	 * @see MaxAcceleration
	 */
	UPROPERTY(Category = "Character Movement: Flying", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float BrakingDecelerationFlying;

	/** 332
	 * When falling, amount of lateral movement control available to the character.
	 * 0 = no control, 1 = full control at max speed of MaxWalkSpeed.
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float AirControl;

	/** 339
	 * When falling, multiplier applied to AirControl when lateral velocity is less than AirControlBoostVelocityThreshold.
	 * Setting this to zero will disable air control boosting. Final result is clamped at 1.
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float AirControlBoostMultiplier;

	/** 346
	 * When falling, if lateral velocity magnitude is less than this value, AirControl is multiplied by AirControlBoostMultiplier.
	 * Setting this to zero will disable air control boosting.
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float AirControlBoostVelocityThreshold;

	/** 353
	 * Friction to apply to lateral air movement when falling.
	 * If bUseSeparateBrakingFriction is false, also affects the ability to stop more quickly when braking (whenever Acceleration is zero).
	 * @see BrakingFriction, bUseSeparateBrakingFriction
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float FallingLateralFriction;

	/** 369
	 * Don't allow the character to perch on the edge of a surface if the contact is this close to the edge of the capsule.
	 * Note that characters will not fall off if they are within MaxStepHeight of a walkable surface below.
	 */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ClampMin = "0", UIMin = "0"))
		float PerchRadiusThreshold;

	/** 376
	 * When perching on a ledge, add this additional distance to MaxStepHeight when determining how high above a walkable floor we can perch.
	 * Note that we still enforce MaxStepHeight to start the step up; this just allows the character to hang off the edge or step slightly higher off the floor.
	 * (@see PerchRadiusThreshold)
	 */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ClampMin = "0", UIMin = "0"))
		float PerchAdditionalHeight;

	/** 384 Change in rotation per second, used when UseControllerDesiredRotation or OrientRotationToMovement are true. Set a negative value for infinite rotation rate and instant turns. */
	UPROPERTY(Category = "Character Movement (Rotation Settings)", EditAnywhere, BlueprintReadWrite)
		FRotator RotationRate;

	/** 388
	 * If true, BrakingFriction will be used to slow the character to a stop (when there is no Acceleration).
	 * If false, braking uses the same friction passed to CalcVelocity() (ie GroundFriction when walking), multiplied by BrakingFrictionFactor.
	 * This setting applies to all movement modes; if only desired in certain modes, consider toggling it when movement modes change.
	 * @see BrakingFriction
	 */
	UPROPERTY(Category = "Character Movement (General Settings)", EditDefaultsOnly, BlueprintReadWrite)
		uint8 bUseSeparateBrakingFriction : 1;

	/** 397
	 *	Apply gravity while the character is actively jumping (e.g. holding the jump key).
	 *	Helps remove frame-rate dependent jump height, but may alter base jump height.
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
		uint8 bApplyGravityWhileJumping : 1;

	/**403
	 * If true, smoothly rotate the Character toward the Controller's desired rotation (typically Controller->ControlRotation), using RotationRate as the rate of rotation change. Overridden by OrientRotationToMovement.
	 * Normally you will want to make sure that other settings are cleared, such as bUseControllerRotationYaw on the Character.
	 */
	UPROPERTY(Category = "Character Movement (Rotation Settings)", EditAnywhere, BlueprintReadWrite)
		uint8 bUseControllerDesiredRotation : 1;

	/** 410
	 * If true, rotate the Character toward the direction of acceleration, using RotationRate as the rate of rotation change. Overrides UseControllerDesiredRotation.
	 * Normally you will want to make sure that other settings are cleared, such as bUseControllerRotationYaw on the Character.
	 */
	UPROPERTY(Category = "Character Movement (Rotation Settings)", EditAnywhere, BlueprintReadWrite)
		uint8 bOrientRotationToMovement : 1;

	/** 417
	 * Whether or not the character should sweep for collision geometry while walking.
	 * @see USceneComponent::MoveComponent.
	 */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite)
		uint8 bSweepWhileNavWalking : 1;

	/** 441
	 * If true, high-level movement updates will be wrapped in a movement scope that accumulates updates and defers a bulk of the work until the end.
	 * When enabled, touch and hit events will not be triggered until the end of multiple moves within an update, which can improve performance.
	 *
	 * @see FScopedMovementUpdate
	 */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, AdvancedDisplay)
		uint8 bEnableScopedMovementUpdates : 1;

	/** 460 Ignores size of acceleration component, and forces max acceleration to drive character at full velocity. */
	UPROPERTY()
		uint8 bForceMaxAccel : 1;

	/** 464
	 * If true, movement will be performed even if there is no Controller for the Character owner.
	 * Normally without a Controller, movement will be aborted and velocity and acceleration are zeroed if the character is walking.
	 * Characters that are spawned without a Controller but with this flag enabled will initialize the movement mode to DefaultLandMovementMode or DefaultWaterMovementMode appropriately.
	 * @see DefaultLandMovementMode, DefaultWaterMovementMode
	 */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
		uint8 bRunPhysicsWithNoController : 1;

	/** 473
	 * Force the Character in MOVE_Walking to do a check for a valid floor even if he hasn't moved. Cleared after next floor check.
	 * Normally if bAlwaysCheckFloor is false we try to avoid the floor check unless some conditions are met, but this can be used to force the next check to always run.
	 */
	UPROPERTY(Category = "Character Movement: Walking", VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay)
		uint8 bForceNextFloorCheck : 1;

	/** 485 If true, Character can walk off a ledge. */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite)
		uint8 bCanWalkOffLedges : 1;

	/** 504
	 * Whether we skip prediction on frames where a proxy receives a network update. This can avoid expensive prediction on those frames,
	 * with the side-effect of predicting with a frame of additional latency.
	 */
	UPROPERTY(Category = "Character Movement (Networking)", EditDefaultsOnly)
		uint8 bNetworkSkipProxyPredictionOnNetUpdate : 1;

	/** 521 true to update CharacterOwner and UpdatedComponent after movement ends */
	UPROPERTY()
		uint8 bDeferUpdateMoveComponent : 1;

	/** 525 If enabled, the player will interact with physics objects when walking into them. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite)
		uint8 bEnablePhysicsInteraction : 1;

	/** 529 If enabled, the TouchForceFactor is applied per kg mass of the affected object. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		uint8 bTouchForceScaledToMass : 1;

	/** 533 If enabled, the PushForceFactor is applied per kg mass of the affected object. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		uint8 bPushForceScaledToMass : 1;

	/** 537 If enabled, the PushForce location is moved using PushForcePointZOffsetFactor. Otherwise simply use the impact point. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		uint8 bPushForceUsingZOffset : 1;

	/** 541 If enabled, the applied push force will try to get the physics object to the same velocity than the player, not faster. This will only
		scale the force down, it will never apply more force than defined by PushForceFactor. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		uint8 bScalePushForceToVelocity : 1;

	/** 548 What to update CharacterOwner and UpdatedComponent after movement ends */
	UPROPERTY()
		USceneComponent* DeferredUpdatedMoveComponent;

	/** 558 Mass of pawn (for when momentum is imparted to it). */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float Mass;

	/** 562 Force applied to objects we stand on (due to Mass and Gravity) is scaled by this amount. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		float StandingDownwardForceScale;

	/** 566 Initial impulse force to apply when the player bounces into a blocking physics object. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		float InitialPushForceFactor;

	/** 570 Force to apply when the player collides with a blocking physics object. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		float PushForceFactor;

	/** 574 Z-Offset for the position the force is applied to. 0.0f is the center of the physics object, 1.0f is the top and -1.0f is the bottom of the object. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "-1.0", UIMax = "1.0"), meta = (editcondition = "bEnablePhysicsInteraction"))
		float PushForcePointZOffsetFactor;

	/** 578 Force to apply to physics objects that are touched by the player. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		float TouchForceFactor;

	/** 582 Minimum Force applied to touched physics objects. If < 0.0f, there is no minimum. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		float MinTouchForce;

	/** 586 Maximum force applied to touched physics objects. If < 0.0f, there is no maximum. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		float MaxTouchForce;

	/** 590 Force per kg applied constantly to all overlapping components. */
	UPROPERTY(Category = "Character Movement: Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		float RepulsionForce;

	/** 611
	 * Current acceleration vector (with magnitude).
	 * This is calculated each update based on the input vector and the constraints of MaxAcceleration and the current movement mode.
	 */
	UPROPERTY()
		FVector Acceleration;

	/**
	 * Rotation after last PerformMovement or SimulateMovement update.
	 */
	UPROPERTY()
		FQuat LastUpdateRotation;

	/**
	 * Location after last PerformMovement or SimulateMovement update. Used internally to detect changes in position from outside character movement to try to validate the current floor.
	 */
	UPROPERTY()
		FVector LastUpdateLocation;

	/**
	 * Velocity after last PerformMovement or SimulateMovement update. Used internally to detect changes in velocity from external sources.
	 */
	UPROPERTY()
		FVector LastUpdateVelocity;

	/**648 Accumulated impulse to be added next tick. */
	UPROPERTY()
		FVector PendingImpulseToApply;

	/** Accumulated force to be added next tick. */
	UPROPERTY()
		FVector PendingForceToApply;

	/** 656
	 * Modifier to applied to values such as acceleration and max speed due to analog input.
	 */
	UPROPERTY()
		float AnalogInputModifier;

	/** 702
	 * Max time delta for each discrete simulation step.
	 * Used primarily in the the more advanced movement modes that break up larger time steps (usually those applying gravity such as falling and walking).
	 * Lowering this value can address issues with fast-moving objects or complex collision scenarios, at the cost of performance.
	 *
	 * WARNING: if (MaxSimulationTimeStep * MaxSimulationIterations) is too low for the min framerate, the last simulation step may exceed MaxSimulationTimeStep to complete the simulation.
	 * @see MaxSimulationIterations
	 */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ClampMin = "0.0166", ClampMax = "0.50", UIMin = "0.0166", UIMax = "0.50"))
		float MaxSimulationTimeStep;

	/** 713
	 * Max number of iterations used for each discrete simulation step.
	 * Used primarily in the the more advanced movement modes that break up larger time steps (usually those applying gravity such as falling and walking).
	 * Increasing this value can address issues with fast-moving objects or complex collision scenarios, at the cost of performance.
	 *
	 * WARNING: if (MaxSimulationTimeStep * MaxSimulationIterations) is too low for the min framerate, the last simulation step may exceed MaxSimulationTimeStep to complete the simulation.
	 * @see MaxSimulationTimeStep
	 */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ClampMin = "1", ClampMax = "25", UIMin = "1", UIMax = "25"))
		int32 MaxSimulationIterations;

	/** 841 Information about the floor the Character is standing on (updated only during walking movement). */
	UPROPERTY(Category = "Character Movement: Walking", VisibleInstanceOnly, BlueprintReadOnly)
		FFindFloorResult CurrentFloor;

	/** 845
	 * Default movement mode when not in water. Used at player startup or when teleported.
	 * @see DefaultWaterMovementMode
	 * @see bRunPhysicsWithNoController
	 */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<enum EMovementMode> DefaultLandMovementMode;

	/**
	 * Default movement mode when in water. Used at player startup or when teleported.
	 * @see DefaultLandMovementMode
	 * @see bRunPhysicsWithNoController
	 */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<enum EMovementMode> DefaultWaterMovementMode;

	/** 833 Used in determining if pawn is going off ledge.  If the ledge is "shorter" than this value then the pawn will be able to walk off it. **/
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
		float LedgeCheckThreshold;


	/** 871
	 * If true, walking movement always maintains horizontal velocity when moving up ramps, which causes movement up ramps to be faster parallel to the ramp surface.
	 * If false, then walking movement maintains velocity magnitude parallel to the ramp surface.
	 * 경사면을 올라갈 때 감속이 되는지 안되는지 여부이다.
	 * True면 평평한 지면을 이동하는 것과 같은 속도로 이동하고
	 * False면 경사면 각도에 따라서 감속을 적용한다.
	 */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite)
		uint8 bMaintainHorizontalGroundVelocity : 1;

	/** 878 If true, impart the base actor's X velocity when falling off it (which includes jumping) */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite)
		uint8 bImpartBaseVelocityX : 1;

	/** 882 If true, impart the base actor's Y velocity when falling off it (which includes jumping) */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite)
		uint8 bImpartBaseVelocityY : 1;

	/** 886 If true, impart the base actor's Z velocity when falling off it (which includes jumping) */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite)
		uint8 bImpartBaseVelocityZ : 1;

	/** 890
	 * If true, impart the base component's tangential components of angular velocity when jumping or falling off it.
	 * Only those components of the velocity allowed by the separate component settings (bImpartBaseVelocityX etc) will be applied.
	 * @see bImpartBaseVelocityX, bImpartBaseVelocityY, bImpartBaseVelocityZ
	 */
	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite)
		uint8 bImpartBaseAngularVelocity : 1;

	/** 898 Used by movement code to determine if a change in position is based on normal movement or a teleport. If not a teleport, velocity can be recomputed based on the change in position. */
	UPROPERTY(Category = "Character Movement (General Settings)", Transient, VisibleInstanceOnly, BlueprintReadWrite)
		uint8 bJustTeleported : 1;

	/** 902 True when a network replication update is received for simulated proxies. */
	UPROPERTY(Transient)
		uint8 bNetworkUpdateReceived : 1;

	/** 906 True when the networked movement mode has been replicated. */
	UPROPERTY(Transient)
		uint8 bNetworkMovementModeChanged : 1;

	/** 942
	 * Whether the character ignores changes in rotation of the base it is standing on.
	 * If true, the character maintains current world rotation.
	 * If false, the character rotates with the moving base.
	 */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite)
		uint8 bIgnoreBaseRotation : 1;

	/** 950
	 * Set this to true if riding on a moving base that you know is clear from non-moving world obstructions.
	 * Optimization to avoid sweeps during based movement, use with care.
	 */
	UPROPERTY()
		uint8 bFastAttachedMove : 1;

	/** 957
	 * Whether we always force floor checks for stationary Characters while walking.
	 * Normally floor checks are avoided if possible when not moving, but this can be used to force them if there are use-cases where they are being skipped erroneously
	 * (such as objects moving up into the character from below).
	 */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
		uint8 bAlwaysCheckFloor : 1;

	/** 965
	 * Performs floor checks as if the character is using a shape with a flat base.
	 * This avoids the situation where characters slowly lower off the side of a ledge (as their capsule 'balances' on the edge).
	 */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
		uint8 bUseFlatBaseForFloorChecks : 1;

	/** 972 Used to prevent reentry of JumpOff() */
	UPROPERTY()
		uint8 bPerformingJumpOff : 1;

	/** 976 Used to safely leave NavWalking movement mode */
	UPROPERTY()
		uint8 bWantsToLeaveNavWalking : 1;

	/** 984
	 * Should use acceleration for path following?
	 * If true, acceleration is applied when path following to reach the target velocity.
	 * If false, path following velocity is set directly, disregarding acceleration.
	 */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
		uint8 bRequestedMoveUseAcceleration : 1;

	/** 1043
	 * Velocity requested by path following.
	 * @see RequestDirectMove()
	 */
	UPROPERTY(Transient)
		FVector RequestedVelocity;

	/** 1090 Temporarily holds launch velocity when pawn is to be launched so it happens at end of movement. */
	UPROPERTY()
		FVector PendingLaunchVelocity;

	// MAGIC NUMBERS
	/** 2432 Minimum delta time considered when ticking. Delta times below this are not considered. This is a very small non-zero value to avoid potential divide-by-zero in simulation code. */
	static const float MIN_TICK_TIME;

	/** Minimum acceptable distance for Character capsule to float above floor when walking. */
	static const float MIN_FLOOR_DIST;

	/** Maximum acceptable distance for Character capsule to float above floor when walking. */
	static const float MAX_FLOOR_DIST;

	/** Reject sweep impacts that are this close to the edge of the vertical portion of the capsule when performing vertical sweeps, and try again with a smaller capsule. */
	static const float SWEEP_EDGE_REJECT_DISTANCE;

	/** 2446 Stop completely when braking and velocity magnitude is lower than this. */
	static const float BRAKE_TO_STOP_VELOCITY;

	/****************

		Function

	****************/


protected:

	/** 116 Called after MovementMode has changed. Base implementation does special handling for starting certain modes, then notifies the CharacterOwner. */
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode);

	/** 1737 @note Movement update functions should only be called through StartNewPhysics()*/
	virtual void PhysWalking(float deltaTime, int32 Iterations);

	/** 1740 @note Movement update functions should only be called through StartNewPhysics()*/
	//virtual void PhysNavWalking(float deltaTime, int32 Iterations);

	/** 1743 @note Movement update functions should only be called through StartNewPhysics()*/
	//virtual void PhysFlying(float deltaTime, int32 Iterations);

	/** 1746 @note Movement update functions should only be called through StartNewPhysics()*/
	//virtual void PhysSwimming(float deltaTime, int32 Iterations);

	/** 1749 @note Movement update functions should only be called through StartNewPhysics()*/
	//virtual void PhysCustom(float deltaTime, int32 Iterations);

	/** 1780
	 * Adjusts velocity when walking so that Z velocity is zero.
	 * When bMaintainHorizontalGroundVelocity is false, also rescales the velocity vector to maintain the original magnitude, but in the horizontal direction.
	 */
	virtual void MaintainHorizontalGroundVelocity();

	/** 1832 감속 적용 Slows towards stop. */
	virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration);

	/** 1984 Enforce constraints on input given current state. For instance, don't move upwards if walking and looking up. */
	virtual FVector ConstrainInputAcceleration(const FVector& InputAcceleration) const;

	/** 1987 Scale input acceleration, based on movement acceleration rate. */
	virtual FVector ScaleInputAcceleration(const FVector& InputAcceleration) const;

	/** 2025 Perform movement on an autonomous client */
	/** 클라이언트 게임용 클라에서 무브먼트를 제어 */
	virtual void PerformMovement(float DeltaTime);


	/** 2031 Simulate movement on a non-owning client. Called by SimulatedTick(). */
	/** 네트워크 게임용 서버에서 무브먼트를 제어*/
	// PerformMovement만 사용하고 지워버려도 괜찮지 않을까 싶음
	virtual void SimulateMovement(float DeltaTime);


public:
	//virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// UpdatedComponent를 Shpere로 Cast 하여 반환하는 메소드
	USphereComponent* GetMovementSphere() const;

	// UpdatedComponent를 Shpere로 Cast 하여 반환하는 메소드
	void MyTickMoving(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);



	/** 662 Computes the analog input modifier based on current input vector and/or acceleration. */
	virtual float ComputeAnalogInputModifier() const;

	/** 691
	 * Compute remaining time step given remaining time and current iterations.
	 * The last iteration (limited by MaxSimulationIterations) always returns the remaining time, which may violate MaxSimulationTimeStep.
	 *
	 * @param RemainingTime		Remaining time in the tick.
	 * @param Iterations		Current iteration of the tick (starting at 1).
	 * @return The remaining time step to use for the next sub-step of iteration.
	 * @see MaxSimulationTimeStep, MaxSimulationIterations
	 */
	float GetSimulationTimeStep(float RemainingTime, int32 Iterations) const;

	/** 1135
	 * Change movement mode.
	 *
	 * @param NewMovementMode	The new movement mode
	 * @param NewCustomMode		The new custom sub-mode, only applicable if NewMovementMode is Custom.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode = 0);

	// 1170
	//virtual void ApplyNetworkMovementMode(const uint8 ReceivedMode);

	// Begin UObject Interface
	//virtual void Serialize(FArchive& Archive) override;
	// End UObject Interface

	//Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//virtual void OnRegister() override;
	//virtual void BeginDestroy() override;
	//virtual void PostLoad() override;
	//virtual void Deactivate() override;
	virtual void RegisterComponentTickFunctions(bool bRegister) override;
	//virtual void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift) override;
	//End UActorComponent Interface

	//BEGIN UMovementComponent Interface
	virtual float GetMaxSpeed() const override;
	//virtual void StopActiveMovement() override;
	//virtual bool IsCrouching() const override;
	virtual bool IsFalling() const override;
	virtual bool IsMovingOnGround() const override;
	virtual bool IsSwimming() const override;
	//virtual bool IsFlying() const override;
	virtual float GetGravityZ() const override;
	//virtual void AddRadialForce(const FVector& Origin, float Radius, float Strength, enum ERadialImpulseFalloff Falloff) override;
	//virtual void AddRadialImpulse(const FVector& Origin, float Radius, float Strength, enum ERadialImpulseFalloff Falloff, bool bVelChange) override;
	//END UMovementComponent Interface

	/** 1203
	 * Returns true if currently performing a movement update.
	 * @see bMovementInProgress
	 */
	bool IsMovementInProgress() const { return bMovementInProgress; }

#if WITH_EDITOR
	// 1221
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	/** 1229Return true if we have a valid CharacterOwner and UpdatedComponent. */
	virtual bool HasValidData() const;

	/** 1240 Transition from walking to falling */
	virtual void StartFalling(int32 Iterations, float remainingTime, float timeTick, const FVector& Delta, const FVector& subLoc);

	/** 1243
	 * Whether Character should go into falling mode when walking and changing position, based on an old and new floor result (both of which are considered walkable).
	 * Default implementation always returns false.
	 * @return true if Character should start falling
	 */
	virtual bool ShouldCatchAir(const FFindFloorResult& OldFloor, const FFindFloorResult& NewFloor);

	/** 1255 Adjust distance from floor, trying to maintain a slight offset from the floor when walking (based on CurrentFloor). */
	virtual void AdjustFloorHeight();

	/** 1262 Update or defer updating of position based on Base movement */
	virtual void MaybeUpdateBasedMovement(float DeltaSeconds);

	/** 1265 Update position based on Base movement */
	virtual void UpdateBasedMovement(float DeltaSeconds);

	/** 1268 Update controller's view rotation as pawn's base rotates */
	virtual void UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation);

	/** 1274
	* Update OldBaseLocation and OldBaseQuat if there is a valid movement base, and store the relative location/rotation if necessary.
	* Ignores bDeferUpdateBasedMovement and forces the update.
	*/
	virtual void SaveBaseLocation();

	/** 1277 changes physics based on MovementMode */
	virtual void StartNewPhysics(float deltaTime, int32 Iterations);

	/** 1280
	 * Perform jump. Called by Character when a jump has been detected because Character->bPressedJump was true. Checks CanJump().
	 * Note that you should usually trigger a jump through Character::Jump() instead.
	 * @return	True if the jump was triggered successfully.
	 */
	virtual bool DoJump();

	// 1288
	/** Queue a pending launch with velocity LaunchVel. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual void Launch(FVector const& LaunchVel);

	/** 1291 Handle a pending launch during an update. Returns true if the launch was triggered. */
	virtual bool HandlePendingLaunch();

	/** 1294
	 * If we have a movement base, get the velocity that should be imparted by that base, usually when jumping off of it.
	 * Only applies the components of the velocity enabled by bImpartBaseVelocityX, bImpartBaseVelocityY, bImpartBaseVelocityZ.
	 */
	 // 이 메소드가 있기 때문에 지면이 움직여도 그에 맞춰서 컴포넌트가 같이 움직일 수 있음!!
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual FVector GetImpartedMovementBaseVelocity() const;

	/** 1301 Force this pawn to bounce off its current base, which isn't an acceptable base for it. */
	virtual void JumpOff(AActor* MovementBaseActor);

	/** 1304 Can be overridden to choose to jump based on character velocity, base actor dimensions, etc. */
	virtual FVector GetBestDirectionOffActor(AActor* BaseActor) const; // Calculates the best direction to go to "jump off" an actor.

	/** 1317 Returns how far to rotate character during the time interval DeltaTime. */
	virtual FRotator GetDeltaRotation(float DeltaTime) const;

	/** 1320
	  * Compute a target rotation based on current movement. Used by PhysicsRotation() when bOrientRotationToMovement is true.
	  * Default implementation targets a rotation based on Acceleration.
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
	 * This does not affect the Acceleration member variable, as that is used to indicate input acceleration.
	 * This may directly affect current Velocity.
	 *
	 * @param DeltaTime				Time slice for this operation
	 * @param MaxAccel				Max acceleration allowed in OutAcceleration result.
	 * @param MaxSpeed				Max speed allowed when computing OutRequestedSpeed.
	 * @param Friction				Current friction.
	 * @param BrakingDeceleration	Current braking deceleration.
	 * @param OutAcceleration		Acceleration computed based on requested velocity.
	 * @param OutRequestedSpeed		Speed of resulting velocity request, which can affect the max speed allowed by movement.
	 * @return Whether there is a requested velocity and acceleration, resulting in valid OutAcceleration and OutRequestedSpeed values.
	 */
	virtual bool ApplyRequestedMove(float DeltaTime, float MaxAccel, float MaxSpeed, float Friction, float BrakingDeceleration, FVector& OutAcceleration, float& OutRequestedSpeed);

	/** 1351
	 * Compute new falling velocity from given velocity and gravity. Applies the limits of the current Physics Volume's TerminalVelocity.
	 */
	virtual FVector NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const;

	/** 1361
	 * Updates Velocity and Acceleration based on the current state, applying the effects of friction and acceleration or deceleration. Does not apply gravity.
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
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement", meta = (Keywords = "Acceleration GetAcceleration"))
		FVector GetCurrentAcceleration() const;

	/** 1410 Returns modifier [0..1] based on the magnitude of the last input vector, which is used to modify the acceleration and max speed during movement. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		float GetAnalogInputModifier() const;

	/** 1414 Returns true if we can step up on the actor in the given FHitResult. */
	virtual bool CanStepUp(const FHitResult& Hit) const;

	/** 1429
	 * Move up steps or slope. Does nothing and returns false if CanStepUp(Hit) returns false.
	 *
	 * @param GravDir			Gravity vector direction (assumed normalized or zero)
	 * @param Delta				Requested move
	 * @param Hit				[In] The hit before the step up.
	 * @param OutStepDownResult	[Out] If non-null, a floor check will be performed if possible as part of the final step down, and it will be updated to reflect this result.
	 * @return true if the step up was successful.
	 */
	virtual bool StepUp(const FVector& GravDir, const FVector& Delta, const FHitResult& Hit, struct FStepDownResult* OutStepDownResult = NULL);

	/** 1440 Update the base of the character, which is the PrimitiveComponent we are standing on. */
	//virtual void SetBase(UPrimitiveComponent* NewBase, const FName BoneName = NAME_None, bool bNotifyActor = true);

	/** 1443
	 * Update the base of the character, using the given floor result if it is walkable, or null if not. Calls SetBase().
	 */
	void SetBaseFromFloor(const FFindFloorResult& FloorResult);

	/** 1448
	 * Applies downward force when walking on top of physics objects.
	 * @param DeltaSeconds Time elapsed since last frame.
	 */
	virtual void ApplyDownwardForce(float DeltaSeconds);

	/** 1454 Applies repulsion force to all touched components. */
	virtual void ApplyRepulsionForce(float DeltaSeconds);

	/** 1457 Applies momentum accumulated through AddImpulse() and AddForce(), then clears those forces. Does *not* use ClearAccumulatedForces() since that would clear pending launch velocity as well. */
	virtual void ApplyAccumulatedForces(float DeltaSeconds);

	/** 1460 Clears forces accumulated through AddImpulse() and AddForce(), and also pending launch velocity. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual void ClearAccumulatedForces();

	virtual void PhysFalling(float deltaTime, int32 Iterations);

	// Helpers for PhysFalling

	/** 1491
	 * Get the lateral acceleration to use during falling movement. The Z component of the result is ignored.
	 * Default implementation returns current Acceleration value modified by GetAirControl(), with Z component removed,
	 * with magnitude clamped to GetMaxAcceleration().
	 * This function is used internally by PhysFalling().
	 *
	 * @param DeltaTime Time step for the current update.
	 * @return Acceleration to use during falling movement.
	 */
	virtual FVector GetFallingLateralAcceleration(float DeltaTime);

	/** 1502
	 * Get the air control to use during falling movement.
	 * Given an initial air control (TickAirControl), applies the result of BoostAirControl().
	 * This function is used internally by GetFallingLateralAcceleration().
	 *
	 * @param DeltaTime			Time step for the current update.
	 * @param TickAirControl	Current air control value.
	 * @param FallAcceleration	Acceleration used during movement.
	 * @return Air control to use during falling movement.
	 * @see AirControl, BoostAirControl(), LimitAirControl(), GetFallingLateralAcceleration()
	 */
	virtual FVector GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration);

protected:

	/** 1517
	 * Increase air control if conditions of AirControlBoostMultiplier and AirControlBoostVelocityThreshold are met.
	 * This function is used internally by GetAirControl().
	 *
	 * @param DeltaTime			Time step for the current update.
	 * @param TickAirControl	Current air control value.
	 * @param FallAcceleration	Acceleration used during movement.
	 * @return Modified air control to use during falling movement
	 * @see GetAirControl()
	 */
	virtual float BoostAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration);

	/** 1529
	 * Limits the air control to use during falling movement, given an impact while falling.
	 * This function is used internally by PhysFalling().
	 *
	 * @param DeltaTime			Time step for the current update.
	 * @param FallAcceleration	Acceleration used during movement.
	 * @param HitResult			Result of impact.
	 * @param bCheckForValidLandingSpot If true, will use IsValidLandingSpot() to determine if HitResult is a walkable surface. If false, this check is skipped.
	 * @return Modified air control acceleration to use during falling movement.
	 * @see PhysFalling()
	 */
	virtual FVector LimitAirControl(float DeltaTime, const FVector& FallAcceleration, const FHitResult& HitResult, bool bCheckForValidLandingSpot);


	/** 1543 Handle landing against Hit surface over remaingTime and iterations, calling SetPostLandedPhysics() and starting the new movement mode. */
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations);

	/** 1546 Use new physics after landing. Defaults to swimming if in water, walking otherwise. */
	virtual void SetPostLandedPhysics(const FHitResult& Hit);

public:

	/** 1575 Called by owning Character upon successful teleport from AActor::TeleportTo(). */
	virtual void OnTeleported() override;

	/** 1594 Returns true if there is a suitable floor SideStep from current position. */
	virtual bool CheckLedgeDirection(const FVector& OldLocation, const FVector& SideStep, const FVector& GravDir) const;

	/** 1597
	 * @param Delta is the current move delta (which ended up going over a ledge).
	 * @return new delta which moves along the ledge
	 */
	virtual FVector GetLedgeMove(const FVector& OldLocation, const FVector& Delta, const FVector& GravDir) const;

	/** 1603 Check if pawn is falling */
	virtual bool CheckFall(const FFindFloorResult& OldFloor, const FHitResult& Hit, const FVector& Delta, const FVector& OldLocation, float remainingTime, float timeTick, int32 Iterations, bool bMustJump);

	/** 1606
	 *  Revert to previous position OldLocation, return to being based on OldBase.
	 *  if bFailMove, stop movement and notify controller
	 */
	void RevertMove(const FVector& OldLocation, UPrimitiveComponent* OldBase, const FVector& InOldBaseLocation, const FFindFloorResult& OldFloor, bool bFailMove);

	/** 1612 Perform rotation over deltaTime */
	virtual void PhysicsRotation(float DeltaTime);

	/** 1615 if true, DesiredRotation will be restricted to only Yaw component in PhysicsRotation() */
	virtual bool ShouldRemainVertical() const;

	/** 1621 Set movement mode to the default based on the current physics volume. */
	virtual void SetDefaultMovementMode();

	virtual void UpdateProxyAcceleration();

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

	/** 1689 Returns whether this pawn is currently allowed to walk off ledges */
	virtual bool CanWalkOffLedges() const;

	/** 1692 Returns The distance from the edge of the capsule within which we don't allow the character to perch on the edge of a surface. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		float GetPerchRadiusThreshold() const;

	/** 1696
	 * Returns the radius within which we can stand on the edge of a surface without falling (if this is a walkable surface).
	 * Simply computed as the capsule radius minus the result of GetPerchRadiusThreshold().
	 */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		float GetValidPerchRadius() const;

	/** 1703 Return true if the hit result should be considered a walkable surface for the character. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		virtual bool IsWalkable(const FHitResult& Hit) const;

	/** 1707 Get the max angle in degrees of a walkable surface for the character. */
	FORCEINLINE float GetWalkableFloorAngle() const { return WalkableFloorAngle; }

	/** 1710 Get the max angle in degrees of a walkable surface for the character. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement", meta = (DisplayName = "GetWalkableFloorAngle", ScriptName = "GetWalkableFloorAngle"))
		float K2_GetWalkableFloorAngle() const;

	/** 1714 Set the max angle in degrees of a walkable surface for the character. Also computes WalkableFloorZ. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		void SetWalkableFloorAngle(float InWalkableFloorAngle);

	/** 1718 Get the Z component of the normal of the steepest walkable surface for the character. Any lower than this and it is not walkable. */
	FORCEINLINE float GetWalkableFloorZ() const { return WalkableFloorZ; }

	/** 1721 Get the Z component of the normal of the steepest walkable surface for the character. Any lower than this and it is not walkable. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement", meta = (DisplayName = "GetWalkableFloorZ", ScriptName = "GetWalkableFloorZ"))
		float K2_GetWalkableFloorZ() const;

	/** 1725 Set the Z component of the normal of the steepest walkable surface for the character. Also computes WalkableFloorAngle. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		void SetWalkableFloorZ(float InWalkableFloorZ);

	/** 1729 Post-physics tick function for this character */
	UPROPERTY()
		struct FUSBMovementComponentPostPhysicsTickFunction PostPhysicsTickFunction;

	/** 1733 Tick function called after physics (sync scene) has finished simulation, before cloth */
	virtual void PostPhysicsTickComponent(float DeltaTime, FUSBMovementComponentPostPhysicsTickFunction& ThisTickFunction);

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
	virtual void MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDownResult = NULL);

	/** 1792 Handle a blocking impact. Calls ApplyImpactPhysicsForces for the hit, if bEnablePhysicsInteraction is true. */
	/*  Hit 이벤트시 bEnablePhysicsInteraction가 True라면 ApplyImpactPhysicsForces를 통해 물리력을 가함 */
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;

	/** 1795
	 * Apply physics forces to the impacted component, if bEnablePhysicsInteraction is true.
	 * @param Impact				HitResult that resulted in the impact
	 * @param ImpactAcceleration	Acceleration of the character at the time of impact
	 * @param ImpactVelocity		Velocity of the character at the time of impact
	 */
	virtual void ApplyImpactPhysicsForces(const FHitResult& Impact, const FVector& ImpactAcceleration, const FVector& ImpactVelocity);

	/** 1803 Custom version of SlideAlongSurface that handles different movement modes separately; namely during walking physics we might not want to slide up slopes. */
	virtual float SlideAlongSurface(const FVector& Delta, float Time, const FVector& Normal, FHitResult& Hit, bool bHandleImpact) override;

	/** 1806 Custom version that allows upwards slides when walking if the surface is walkable. */
	virtual void TwoWallAdjust(FVector& Delta, const FHitResult& Hit, const FVector& OldHitNormal) const override;

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
	virtual void FindFloor(const FVector& CapsuleLocation, FFindFloorResult& OutFloorResult, bool bCanUseCachedLocation, const FHitResult* DownwardSweepResult = NULL) const;

	/** 1855
	* Sweeps a vertical trace to find the floor for the capsule at the given location. Will attempt to perch if ShouldComputePerchResult() returns true for the downward sweep result.
	* No floor will be found if collision is disabled on the capsule!
	*
	* @param CapsuleLocation		Location where the capsule sweep should originate
	* @param FloorResult			Result of the floor check
	*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement", meta = (DisplayName = "FindFloor", ScriptName = "FindFloor"))
		virtual void K2_FindFloor(FVector CapsuleLocation, FFindFloorResult& FloorResult) const;

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
	virtual void ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance, FFindFloorResult& OutFloorResult, float SweepRadius, const FHitResult* DownwardSweepResult = NULL) const;

	/** 1880
	* Compute distance to the floor from bottom sphere of capsule and store the result in FloorResult.
	* This distance is the swept distance of the capsule to the first point impacted by the lower hemisphere, or distance from the bottom of the capsule in the case of a line trace.
	* This function does not care if collision is disabled on the capsule (unlike FindFloor).
	*
	* @param CapsuleLocation		Location where the capsule sweep should originate
	* @param LineDistance			If non-zero, max distance to test for a simple line check from the capsule base. Used only if the sweep test fails to find a walkable floor, and only returns a valid result if the impact normal is a walkable normal.
	* @param SweepDistance			If non-zero, max distance to use when sweeping a capsule downwards for the test. MUST be greater than or equal to the line distance.
	* @param SweepRadius			The radius to use for sweep tests. Should be <= capsule radius.
	* @param FloorResult			Result of the floor check
	*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement", meta = (DisplayName = "ComputeFloorDistance", ScriptName = "ComputeFloorDistance"))
		virtual void K2_ComputeFloorDist(FVector CapsuleLocation, float LineDistance, float SweepDistance, float SweepRadius, FFindFloorResult& FloorResult) const;

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
		const struct FCollisionShape& CollisionShape,
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

	/** 1926
	 * Check if the result of a sweep test (passed in InHit) might be a valid location to perch, in which case we should use ComputePerchResult to validate the location.
	 * @see ComputePerchResult
	 * @param InHit:			Result of the last sweep test before this query.
	 * @param bCheckRadius:		If true, only allow the perch test if the impact point is outside the radius returned by GetValidPerchRadius().
	 * @return Whether perching may be possible, such that ComputePerchResult can return a valid result.
	 */
	virtual bool ShouldComputePerchResult(const FHitResult& InHit, bool bCheckRadius = true) const;

	/** 1935
	 * Compute the sweep result of the smaller capsule with radius specified by GetValidPerchRadius(),
	 * and return true if the sweep contacts a valid walkable normal within InMaxFloorDist of InHit.ImpactPoint.
	 * This may be used to determine if the capsule can or cannot stay at the current location if perched on the edge of a small ledge or unwalkable surface.
	 * Note: Only returns a valid result if ShouldComputePerchResult returned true for the supplied hit value.
	 *
	 * @param TestRadius:			Radius to use for the sweep, usually GetValidPerchRadius().
	 * @param InHit:				Result of the last sweep test before the query.
	 * @param InMaxFloorDist:		Max distance to floor allowed by perching, from the supplied contact point (InHit.ImpactPoint).
	 * @param OutPerchFloorResult:	Contains the result of the perch floor test.
	 * @return True if the current location is a valid spot at which to perch.
	 */
	virtual bool ComputePerchResult(const float TestRadius, const FHitResult& InHit, const float InMaxFloorDist, FFindFloorResult& OutPerchFloorResult) const;

protected:
	/** 1952 Called when the collision capsule touches another primitive component */
	UFUNCTION()
		virtual void CapsuleTouched(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

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

	/** 1990
	 * Event triggered at the end of a movement update. If scoped movement updates are enabled (bEnableScopedMovementUpdates), this is within such a scope.
	 * If that is not desired, bind to the CharacterOwner's OnMovementUpdated event instead, as that is triggered after the scoped movement update.
	 */
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity);

	/** 1996 Internal function to call OnMovementUpdated delegate on CharacterOwner. */
	virtual void CallMovementUpdateDelegate(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity);

public:

	/** 2049 Round acceleration, for better consistency and lower bandwidth in networked games. */
	virtual FVector RoundAcceleration(FVector InAccel) const;

	/** 2174 React to instantaneous change in position. Invalidates cached floor recomputes it if possible if there is a current movement base. */
	virtual void UpdateFloorFromAdjustment();

	/** 2398
	 * When moving the character, we should inform physics as to whether we are teleporting.
	 * This allows physics to avoid injecting forces into simulations from client corrections (etc.)
	 */
	ETeleportType GetTeleportType() const;

















	/*******************************************************
	 *
	 *
	 *		Character.h에서 가져온 변수 및 메소드들
	 *		Character에서 다루는 Jump및 MovementBase 요소들이
	 *		Pawn에는 없기 때문에 여기서 새로 정의
	 *
	 *
	 *******************************************************/
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

protected:
	/**
	 * Customizable event to check if the character can jump in the current state.
	 * Default implementation returns true if the character is on the ground and not crouching,
	 * has a valid CharacterMovementComponent and CanEverJump() returns true.
	 * Default implementation also allows for 'hold to jump higher' functionality:
	 * As well as returning true when on the ground, it also returns true when GetMaxJumpTime is more
	 * than zero and IsJumping returns true.
	 *
	 *
	 * @Return Whether the character can jump in the current state.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "CharacterJump", meta = (DisplayName = "CanJump"))
		bool CanJumpInternal() const;
	virtual bool CanJumpInternal_Implementation() const;

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





	/******** 여기부터 Base 관련  *********/


protected:
	/** Info about our current movement base (object we are standing on). */
	UPROPERTY()
		struct FBasedMovementInfo BasedMovement;

	/** Flag that we are receiving replication of the based movement. */
	UPROPERTY()
		bool bInBaseReplication;

	/** Saved translation offset of mesh. */
	UPROPERTY()
		FVector BaseTranslationOffset;

	/** Saved rotation offset of mesh. */
	UPROPERTY()
		FQuat BaseRotationOffset;

	/** Event called after actor's base changes (if SetBase was requested to notify us with bNotifyPawn). */
	virtual void BaseChange();

public:
	/** Sets the component the Character is walking on, used by CharacterMovement walking movement to be able to follow dynamic objects. */
	virtual void SetBase(UPrimitiveComponent* NewBase, const FName BoneName = NAME_None, bool bNotifyActor = true);

	/** Accessor for BasedMovement */
	FORCEINLINE const FBasedMovementInfo& GetBasedMovement() const { return BasedMovement; }

	/** Save a new relative location in BasedMovement and a new rotation with is either relative or absolute. */
	void SaveRelativeBasedMovement(const FVector& NewRelativeLocation, const FRotator& NewRotation, bool bRelativeRotation);

	/** Get the saved translation offset of mesh. This is how much extra offset is applied from the center of the capsule. */
	UFUNCTION(BlueprintCallable, Category = Character)
		FVector GetBaseTranslationOffset() const { return BaseTranslationOffset; }

	/** Get the saved rotation offset of mesh. This is how much extra rotation is applied from the capsule rotation. */
	virtual FQuat GetBaseRotationOffset() const { return BaseRotationOffset; }

	/** Get the saved rotation offset of mesh. This is how much extra rotation is applied from the capsule rotation. */
	UFUNCTION(BlueprintCallable, Category = Character, meta = (DisplayName = "GetBaseRotationOffset", ScriptName = "GetBaseRotationOffset"))
		FRotator GetBaseRotationOffsetRotator() const { return GetBaseRotationOffset().Rotator(); }

	/** Return PrimitiveComponent we are based on (standing and walking on). 이건 CharacterMovementComponent 내에 있음 */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
		FORCEINLINE UPrimitiveComponent* GetMovementBase() const { return BasedMovement.MovementBase; }
	//virtual UPrimitiveComponent* GetMovementBase() const override final { return BasedMovement.MovementBase; }
};


