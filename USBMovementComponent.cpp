// Fill out your copyright notice in the Description page of Project Settings.

/************************************************************************************************************************************************************
*
*		19.10.03 Character Movement Component 소스 복사 및 분석 시작
*
*		19.10.04 SimulateMovemnt 메소드를 중심으로 다 복사완료.
*				 다만 SimulateMovement는 네트워크 기반에서 동작하는 메소드라 네트워크 관련 제어값과 별개로 동작하도록 개조 중
*				  - 네트워크 관련 전부 주석 처리
*				  - 빠른 빌드를 위해 동적인 지면과 캐릭터가 같이 움직이도록 도와주는 Base 처리도 주석처리(메소드 내부만, 껍데기는 남겨둠)
*				  - 근데 그러자니 Floor도 다 터지네 망 ㅡㅡ
*
*		19.10.14 Floor관련 변수, 메소드 우선적으로 옮겨두기 시작
*				 GetMovementSphere() 메소드 생성	=>	CharacterOwner->GetCapsuleComponent() 대체
*				 지면인식 및 경사면 타기 성공!
*				 중력은 대충 구현해두고 나중에 다듬어야함
*				 GetImpartedMovementBaseVelocity 로 지면이동시 같은 속도로 이동되는거 하려 했는데 Base가 NULL이 떠서 손봐야함 ㅡㅡ
*
*		19.10.15 SimulateMovement를 개조하다보니 어차피 구성요소를 다 가져왔으니 PerformMovement에서 이것들을 적용하는게 더 좋다고 판단
*				 PerformMovement 가져옴. 개조 필요.
*				 Root모션 관련 다 제거
*				 PerformMovement -> StartNewPhysics -> PhysWalking까지 옮겨뒀음. 다만 아직 Base가 없어서인지 제대로 움직이질 못함... 잘 찾아보자.
*
*		19.10.24 Base관련 변수 및 함수 정리
*				 Base 관련된 것들을 옮기기 전에 Falling 처리부터 하기로함.
*				 PhysFalling 옮기기
*				 Falling 동작
*
*				 Character.h / .cpp에서 Jump와 관련된 요소들을 가져옴. Character가 아닌 Pawn이기 때문에 무브먼트에서 처리
*				 아직 요소들만 가져오고 Tick 함수 같은 곳에서 처리하는 부분은 구현하지 않음
*
*				 FVector::GetClampedToMaxSize()가 이상하게 동작하여 CustomGetClampedToMaxSize를 따로 작성하였다.
*				 예전에 예제 작성해볼 때도 겪었던 오류인데 도저히 원인 파악이 안됨
*				 => 방향키 눌렀을 때 슝하고 이상한 곳으로 날라가던 현상이 조금 완화됨
*				 => 해당 버그는 Walking 상태에서 방향키를 눌렀다가 떼었을 때, 방향벡터가 조금이라도 변하였을 때(ex 고개를 돌림) 방향키를 눌러 이동할 때 나타남
*				 ==> 앞으로 블랙아웃 현상이라 기술
*
*				 Falling 상태에서 움직이는지 안움직이는지 잘 모르겠음 차후 확인 필요
*
*
*		19.10.25 Falling 상태에서 움직임(방향키에 따른 가속) 동작 확인
*				 => 현재 감속량이 굉장히 큰데 이는 Air Control에 의한 감속이 정상 동작하는지는 확인할 필요가 있음
*
*				 어제 작업한 CustomGetClampedToMaxSize에서 특정방향, 특정 Max 값에서만 정상 동작하던 오류를 해결
*				 => 방향벡터가 조금이라도 변하였을 때(ex 고개를 돌림) 나타나던 블랙아웃 현상이 해결됨
*
*				 블랙아웃 현상에 대한 고찰
*				 => Walking 상태에서 방향키를 누르다 떼면 현상이 나타난다.
*				 => Walking 상태에서 방향키를 누르는 중에 반대 방향 키를 같이 눌러도 현상이 나타난다.
*				 => Falling 상태에서 방향키를 한 번이라도 눌렀다면 Walking 상태로 넘어가자마자 현상이 나타난다.
*				 => Walking 상태에서 서로 방향이 수직인 키를 누르고 있을 때 한 키의 반대방향 키를 누를 때는 현상이 나타나지 않는다.
*				 => Walking => Falling 상태로 넘어가면 방향키를 떼어도 현상이 나타나지 않는다.
*				 => 블랙아웃 현상이 나타났을 때 Velocity의 값이 굉장히 크게 변한다. (X, Y 중 하나만)
*				 ===> 블랙아웃 현상은 방향키에 의해 Velocity가 가속된 이후 다시 Zero Vector로 감속되는 와중에 생기는 오류로 추정됨.
*
*				 멘탈이 나갈꺼 같다. 이게 대체 무슨 증상이지?...
*				 CalcVelocity 앞에 UE_LOG(LogTemp, Log, TEXT("Before Velocity : %s"), *Velocity.ToString());를 붙이면 정상동작하고 안붙이면 블랙아웃 현상이 나타난다.
*				 LOG가 인게임에 영향을 줄리도 없고 Velocity.ToString 또한 그럴텐데 대체 뭘까.... 일단 계속 증상을 파고 들어가보자.
*				 => 일단 이 증상은 CalcVelocity 함수에서 나타나는 것 이 확인.
*				 => CalcVelocity 내에서 가속이 없을 때를 나타내는 변수인 bZeroAcceleration를 살펴보자
*
*				 CalcVelocity() 내의 else if (!bZeroAcceleration) 부분과 ApplyVelocityBraking()에서
*				 FVector::GetSafeNormal()를 사용하면서 생기던 오류였다.
*				 VS 2019 16.3.3 버전에서 생기는 컴파일러의 오류란다. (이걸 작성하는 시점에선 16.3.6버전 사용)
*				 FVector::GetClampedToMaxSize()도 동일한 이유에서 에러가 나오던 것 같다.
*				 무슨 이윤진 몰라도 해당 함수들이 호출되기 전에 UE_LOG 매크로를 사용하면 정상 동작했던 것...
*				 해결 방법은 직접 커스텀 함수를 만들어서 바꾸는 방법과 이전버전의 컴파일러로 교체하여 사용하는 방법이 있다.
*				 => 본인은 전자의 방법을 사용하였음.
*				 => 해당 함수들에는 모두 FMath::InvSqrt()가 사용되었는데 이것이 문제인 것 같음.
*				 ==> 블랙아웃 버그 해결!
*
*		19.10.28 Tick 내 점프 추가
*				 OnMovementModeChanged 추가 및 기존 Character에 있던 것도 같이 가져옴(점프 구현을 위함)
*				 이제 기존 Character에서 점프를 구현하듯 Jump() StopJumping()만 추가해줘도 캐릭터가 점프 할 수 있음
*
*				 대망의 Base 작업 화이팅화이팅합시다.
*				 Base 관련 namespase, 구조체는 Character.h에서 사용
*				 Character에서 Base 관련 요소 가져옴. 네트워크 관련은 제외.
*				 Base 관련되어 주석처리했던 부분들 전부 정상화
*				 ==>> Base 정상 작동!!! 지면에 따라 움직인다 ㅠㅠㅠㅠ
*				 단, PostPhysicsTick 부분은 전부 주석처리. Base가 시뮬레이션일 때 그에 따른 처리를 하는 것으로 보임. 나중에 적용
*
*				 Walking 상태에서 입력 버튼을 꾹 누르면 지면 밖으로 나가도 Falling으로 변하지 않는 버그 발견
*				 Base가 갱신되지 않기 때문인걸로 추정
*				 => 예전에 Ledge(모서리) 처리 부분을 주석처리 해둬서 생긴 문제였음. 해결
*
*				 PosyPhysicsTick 부분 가져왔음. 가져왔는데 음... 어...
*				 정상동작하는지는 의문 내일 확인해보자.
*				 일단 시뮬레이팅 오브젝트에 비비다보면 슝하고 날라가는거 있음
*
*		19.10.29 GetSafeNormal2D => CustomGetSafeNormal2D. 이유는 여태까지와 동일
*				 => 적용 후 시뮬레이팅 오브젝트에 비비다 날라가는 오류 사라짐
*
*				 적용안되어 있던 Ledge 처리 관련 함수 추가
*
*				 bEnablePhysicsInteraction = true시 시뮬레이팅 엑터에 관여하는 부분 보완 완료.
*				 이제 기존 CharacterMovementComponent랑 거의 비슷하게 동작하는 것 같다.
*
*				 Add Impulse, Add Force, Launch 추가.
*				 이 함수들을 통해 Sphere에 물리력을 가할 수 있음.
*
*				 OnTeleported가 제대로 동작하지 않음. 다만 이건 당장에 큰 문제는 없으니 나중에 점검해보자.
*
*		19.10.31 bEnableGravity 변수 추가. 사용자가 Falling 상태에서 중력을 적용할지 말지를 정함.
*				 값에 False면 GetGravityZ()에서 0을 반환
*
*				 USB 캐릭터에 적용 아주 잘됨!
*				 버그 1. Ledge 밖으로 나갈 때 위치가 꺾이는 버그
*				 버그 2. USB가 시작될 때 USBMovementComponent가 Valid하지 않다가 Valid한 버그
*
*				 개선이 필요한 부분
*				 1. Flying모드 개발 필요성 - 공중에서 날아가는 부분은 Falling 모드에서 자연스럽게 날리기가 쉽지 않음.
*
*				 버그 1에 대한 방안으로 CheckLedgeDirection 부분에서 Sweep 실패시 두번째 Sweep하는 부분을 주석처리 하였다.
*				 버그 1은 거의 사라졌으나 아주 가끔 낮은 확률로 나타난다. 계속 테스트 하면서 1,2번 본거 같은데 잘못 봤을 수도 있다.
*				 주석처리한 부분은 정확도를 위해 추가된 부분이니 오류가 있더라도 크리티컬하진 않고 미미한 오류이지 않을까라고 생각.
*
*		19.11.08 Walkable Floor Angle 과  Wakable Floor Z 값이 연동되는 부분을 추가하였음.
*				 
*		19.11.25 PhysicsRotation 및 관련 메소드 추가
*				 캐릭터의 회전과 관련된 부분.
*
*
************************************************************************************************************************************************************/






#include "USBMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PhysicsVolume.h"
#include "GameFramework/Controller.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AI/Navigation/AvoidanceManager.h"
#include "Engine/Engine.h"



/***********************************************************************************************
*
*							CharacterMovement 소스 복사
*
***********************************************************************************************/

/** 41
 * Character stats
 *
 * 프로파일링을 위한 SCOPE_CYCLE_COUNTER 매크로에 들어갈 STAT 목록
 */
DECLARE_CYCLE_STAT(TEXT("Char Tick"), STAT_CharacterMovementTick, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char NonSimulated Time"), STAT_CharacterMovementNonSimulated, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char Simulated Time"), STAT_CharacterMovementSimulated, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char PerformMovement"), STAT_CharacterMovementPerformMovement, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char ReplicateMoveToServer"), STAT_CharacterMovementReplicateMoveToServer, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char CallServerMove"), STAT_CharacterMovementCallServerMove, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char ServerMove"), STAT_CharacterMovementServerMove, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char ServerForcePositionUpdate"), STAT_CharacterMovementForcePositionUpdate, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char RootMotionSource Calculate"), STAT_CharacterMovementRootMotionSourceCalculate, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char RootMotionSource Apply"), STAT_CharacterMovementRootMotionSourceApply, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char ClientUpdatePositionAfterServerUpdate"), STAT_CharacterMovementClientUpdatePositionAfterServerUpdate, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char CombineNetMove"), STAT_CharacterMovementCombineNetMove, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char NetSmoothCorrection"), STAT_CharacterMovementSmoothCorrection, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char SmoothClientPosition"), STAT_CharacterMovementSmoothClientPosition, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char SmoothClientPosition_Interp"), STAT_CharacterMovementSmoothClientPosition_Interp, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char SmoothClientPosition_Visual"), STAT_CharacterMovementSmoothClientPosition_Visual, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char Physics Interation"), STAT_CharPhysicsInteraction, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char StepUp"), STAT_CharStepUp, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char FindFloor"), STAT_CharFindFloor, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char AdjustFloorHeight"), STAT_CharAdjustFloorHeight, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char Update Acceleration"), STAT_CharUpdateAcceleration, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char MoveUpdateDelegate"), STAT_CharMoveUpdateDelegate, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char PhysWalking"), STAT_CharPhysWalking, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char PhysFalling"), STAT_CharPhysFalling, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char PhysNavWalking"), STAT_CharPhysNavWalking, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char NavProjectPoint"), STAT_CharNavProjectPoint, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char NavProjectLocation"), STAT_CharNavProjectLocation, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char ProcessLanded"), STAT_CharProcessLanded, STATGROUP_Character);


DECLARE_CYCLE_STAT(TEXT("Char HandleImpact"), STAT_CharHandleImpact, STATGROUP_Character);


// 75 MAGIC NUMBERS
const float MAX_STEP_SIDE_Z = 0.08f;	// maximum z value for the normal on the vertical side of steps
const float SWIMBOBSPEED = -80.f;
const float VERTICAL_SLOPE_NORMAL_Z = 0.001f; // Slope is vertical if Abs(Normal.Z) <= this threshold. Accounts for precision problems that sometimes angle normals slightly off horizontal for vertical surface.

const float UUSBMovementComponent::MIN_TICK_TIME = 1e-6f;
const float UUSBMovementComponent::MIN_FLOOR_DIST = 1.9f;
const float UUSBMovementComponent::MAX_FLOOR_DIST = 2.4f;
const float UUSBMovementComponent::BRAKE_TO_STOP_VELOCITY = 10.f;
const float UUSBMovementComponent::SWEEP_EDGE_REJECT_DISTANCE = 0.15f;

// 89 Defines for build configs
#if DO_CHECK && !UE_BUILD_SHIPPING // Disable even if checks in shipping are enabled.
#define devCode( Code )		checkCode( Code )
#else
#define devCode(...)
#endif


// 97 CVars
//namespace CharacterMovementCVars
	/**************************************************************************************************
	*
	*	콘솔 변수를 생성/등록 한다.
	*	콘솔 변수에 대한 자세한 설명은 아래 링크 참조
	*	https://docs.unrealengine.com/ko/Programming/Development/Tools/ConsoleManager/index.html
	*
	**************************************************************************************************/
/*{// 소스 가져오면서 에러 없애려고 가져왔는데 필요 없는애들 빼다보니 그 사이에 이것도 필요가 없어진 것 같음
	// Listen server smoothing
	static int32 NetEnableListenServerSmoothing = 1;
	FAutoConsoleVariableRef CVarNetEnableListenServerSmoothing(
		TEXT("p.NetEnableListenServerSmoothing"),
		NetEnableListenServerSmoothing,
		TEXT("Whether to enable mesh smoothing on listen servers for the local view of remote clients.\n")
		TEXT("0: Disable, 1: Enable"),
		ECVF_Default);

	// Latent proxy prediction
	static int32 NetEnableSkipProxyPredictionOnNetUpdate = 1;
	FAutoConsoleVariableRef CVarNetEnableSkipProxyPredictionOnNetUpdate(
		TEXT("p.NetEnableSkipProxyPredictionOnNetUpdate"),
		NetEnableSkipProxyPredictionOnNetUpdate,
		TEXT("Whether to allow proxies to skip prediction on frames with a network position update, if bNetworkSkipProxyPredictionOnNetUpdate is also true on the movement component.\n")
		TEXT("0: Disable, 1: Enable"),
		ECVF_Default);

	// Logging when character is stuck. Off by default in shipping.
#if UE_BUILD_SHIPPING
	static float StuckWarningPeriod = -1.f;
#else
	static float StuckWarningPeriod = 1.f;
#endif

	FAutoConsoleVariableRef CVarStuckWarningPeriod(
		TEXT("p.CharacterStuckWarningPeriod"),
		StuckWarningPeriod,
		TEXT("How often (in seconds) we are allowed to log a message about being stuck in geometry.\n")
		TEXT("<0: Disable, >=0: Enable and log this often, in seconds."),
		ECVF_Default);

	static int32 NetEnableMoveCombining = 1;
	FAutoConsoleVariableRef CVarNetEnableMoveCombining(
		TEXT("p.NetEnableMoveCombining"),
		NetEnableMoveCombining,
		TEXT("Whether to enable move combining on the client to reduce bandwidth by combining similar moves.\n")
		TEXT("0: Disable, 1: Enable"),
		ECVF_Default);

	static int32 NetUseClientTimestampForReplicatedTransform = 1;
	FAutoConsoleVariableRef CVarNetUseClientTimestampForReplicatedTransform(
		TEXT("p.NetUseClientTimestampForReplicatedTransform"),
		NetUseClientTimestampForReplicatedTransform,
		TEXT("If enabled, use client timestamp changes to track the replicated transform timestamp, otherwise uses server tick time as the timestamp.\n")
		TEXT("Game session usually needs to be restarted if this is changed at runtime.\n")
		TEXT("0: Disable, 1: Enable"),
		ECVF_Default);

	static int32 ReplayUseInterpolation = 0;
	FAutoConsoleVariableRef CVarReplayUseInterpolation(
		TEXT("p.ReplayUseInterpolation"),
		ReplayUseInterpolation,
		TEXT(""),
		ECVF_Default);
		
}*/
/***********************************************************************************************/


/****************************************************
 *
 *
 *			PostPhysics 관련 320~341줄
 *
 *
 ****************************************************/
void FUSBMovementComponentPostPhysicsTickFunction::ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	FActorComponentTickFunction::ExecuteTickHelper(Target, /*bTickInEditor=*/ false, DeltaTime, TickType, [this](float DilatedTime)
		{
			Target->PostPhysicsTickComponent(DilatedTime, *this);
		});
}

FString FUSBMovementComponentPostPhysicsTickFunction::DiagnosticMessage()
{
	return Target->GetFullName() + TEXT("[UCharacterMovementComponent::PreClothTick]");
}

FName FUSBMovementComponentPostPhysicsTickFunction::DiagnosticContext(bool bDetailed)
{
	if (bDetailed)
	{
		return FName(*FString::Printf(TEXT("SkeletalMeshComponentClothTick/%s"), *GetFullNameSafe(Target)));
	}

	return FName(TEXT("SkeletalMeshComponentClothTick"));
}


/**********************************
*
*			  생성자
*
**********************************/
UUSBMovementComponent::UUSBMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PostPhysicsTickFunction.bCanEverTick = true;
	PostPhysicsTickFunction.bStartWithTickEnabled = false;
	PostPhysicsTickFunction.SetTickFunctionEnable(false);
	PostPhysicsTickFunction.TickGroup = TG_PostPhysics;

	bApplyGravityWhileJumping = true;

	GravityScale = 1.f;
	GroundFriction = 8.0f;
	JumpZVelocity = 420.0f;
	JumpOffJumpZFactor = 0.5f;
	RotationRate = FRotator(0.f, 360.0f, 0.0f);
	SetWalkableFloorZ(0.71f);	// WalkableFloorAngle도 같이 초기화됨

	MaxStepHeight = 45.0f;
	PerchRadiusThreshold = 0.0f;
	PerchAdditionalHeight = 40.f;

	MaxFlySpeed = 600.0f;
	MaxWalkSpeed = 400.0f;
	MaxSwimSpeed = 300.0f;
	MaxCustomMovementSpeed = MaxWalkSpeed;

	MaxSimulationTimeStep = 0.05f;
	MaxSimulationIterations = 8;

	//MaxDepenetrationWithGeometry = 500.f;
	//MaxDepenetrationWithGeometryAsProxy = 100.f;
	//MaxDepenetrationWithPawn = 100.f;
	//MaxDepenetrationWithPawnAsProxy = 2.f;

	// Set to match EVectorQuantization::RoundTwoDecimals
	//NetProxyShrinkRadius = 0.01f;
	//NetProxyShrinkHalfHeight = 0.01f;

	//NetworkSimulatedSmoothLocationTime = 0.100f;
	//NetworkSimulatedSmoothRotationTime = 0.050f;
	//ListenServerNetworkSimulatedSmoothLocationTime = 0.040f;
	//ListenServerNetworkSimulatedSmoothRotationTime = 0.033f;
	//NetworkMaxSmoothUpdateDistance = 256.f;
	//NetworkNoSmoothUpdateDistance = 384.f;
	//NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
	//ServerLastClientGoodMoveAckTime = -1.f;
	//ServerLastClientAdjustmentTime = -1.f;
	//NetworkMinTimeBetweenClientAckGoodMoves = 0.10f;
	//NetworkMinTimeBetweenClientAdjustments = 0.10f;
	//NetworkMinTimeBetweenClientAdjustmentsLargeCorrection = 0.05f;
	//NetworkLargeClientCorrectionDistance = 15.0f;

	MaxWalkSpeedCrouched = MaxWalkSpeed * 0.5f;
	//MaxOutOfWaterStepHeight = 40.0f;
	//OutofWaterZ = 420.0f;
	AirControl = 0.05f;
	AirControlBoostMultiplier = 2.f;
	AirControlBoostVelocityThreshold = 25.f;
	FallingLateralFriction = 0.f;
	MaxAcceleration = 2048.0f;
	BrakingFrictionFactor = 2.0f; // Historical value, 1 would be more appropriate.
	BrakingSubStepTime = 1.0f / 33.0f;
	BrakingDecelerationWalking = MaxAcceleration;
	BrakingDecelerationFalling = 0.f;
	BrakingDecelerationFlying = 0.f;
	BrakingDecelerationSwimming = 0.f;
	LedgeCheckThreshold = 4.0f;
	//JumpOutOfWaterPitch = 11.25f;

#if WITH_EDITORONLY_DATA
	//CrouchedSpeedMultiplier_DEPRECATED = 0.5f;
	//UpperImpactNormalScale_DEPRECATED = 0.5f;
	//bForceBraking_DEPRECATED = false;
#endif

	Mass = 100.0f;
	bJustTeleported = true;
	//CrouchedHalfHeight = 40.0f;
	//Buoyancy = 1.0f;
	LastUpdateRotation = FQuat::Identity;
	LastUpdateVelocity = FVector::ZeroVector;
	PendingImpulseToApply = FVector::ZeroVector;
	PendingLaunchVelocity = FVector::ZeroVector;
	DefaultWaterMovementMode = MOVE_Swimming;
	DefaultLandMovementMode = MOVE_Walking;
	GroundMovementMode = MOVE_Walking;
	MovementMode = GroundMovementMode; // 임의 추가
	bForceNextFloorCheck = true;
	//bShrinkProxyCapsule = true;
	bCanWalkOffLedges = true;
	//bCanWalkOffLedgesWhenCrouching = false;
	//bNetworkSmoothingComplete = true; // Initially true until we get a net update, so we don't try to smooth to an uninitialized value.
	bWantsToLeaveNavWalking = false;
	//bIsNavWalkingOnServer = false;
	bSweepWhileNavWalking = true;
	bNeedsSweepWhileWalkingUpdate = false;

	bEnablePhysicsInteraction = true;
	StandingDownwardForceScale = 1.0f;
	InitialPushForceFactor = 500.0f;
	PushForceFactor = 750000.0f;
	PushForcePointZOffsetFactor = -0.75f;
	bPushForceUsingZOffset = false;
	bPushForceScaledToMass = false;
	bScalePushForceToVelocity = true;

	TouchForceFactor = 1.0f;
	bTouchForceScaledToMass = true;
	MinTouchForce = -1.0f;
	MaxTouchForce = 250.0f;
	RepulsionForce = 2.5f;

	//bAllowPhysicsRotationDuringAnimRootMotion = false; // Old default behavior.
	bUseControllerDesiredRotation = false;

	bUseSeparateBrakingFriction = false; // Old default behavior.

	bMaintainHorizontalGroundVelocity = true;
	bImpartBaseVelocityX = true;
	bImpartBaseVelocityY = true;
	bImpartBaseVelocityZ = true;
	bImpartBaseAngularVelocity = true;
	//bIgnoreClientMovementErrorChecksAndCorrection = false;
	bAlwaysCheckFloor = true;

	// default character can jump, walk, and swim
	NavAgentProps.bCanJump = true;
	NavAgentProps.bCanWalk = true;
	NavAgentProps.bCanSwim = true;
	ResetMoveState();

	//ClientPredictionData = NULL;
	//ServerPredictionData = NULL;

	// This should be greater than tolerated player timeout * 2.
	//MinTimeBetweenTimeStampResets = 4.f * 60.f;
	//LastTimeStampResetServerTime = 0.f;

	bEnableScopedMovementUpdates = true;
	// Disabled by default since it can be a subtle behavior change, you should opt in if you want to accept that.
	//bEnableServerDualMoveScopedMovementUpdates = false;

	bRequestedMoveUseAcceleration = true;
	//bUseRVOAvoidance = false;
	//bUseRVOPostProcess = false;
	//AvoidanceLockVelocity = FVector::ZeroVector;
	//AvoidanceLockTimer = 0.0f;
	//AvoidanceGroup.bGroup0 = true;
	//GroupsToAvoid.Packed = 0xFFFFFFFF;
	//GroupsToIgnore.Packed = 0;
	//AvoidanceConsiderationRadius = 500.0f;

	OldBaseQuat = FQuat::Identity;
	OldBaseLocation = FVector::ZeroVector;

	//NavMeshProjectionInterval = 0.1f;
	//NavMeshProjectionInterpSpeed = 12.f;
	//NavMeshProjectionHeightScaleUp = 0.67f;
	//NavMeshProjectionHeightScaleDown = 1.0f;
	//NavWalkingFloorDistTolerance = 10.0f;


	// Jump 관련 요소. 출처 Character.cpp 
	JumpKeyHoldTime = 0.0f;
	JumpMaxHoldTime = 0.0f;
	JumpMaxCount = 1;
	JumpCurrentCount = 0;
	bWasJumping = false;

	// 직접 추가한 변수들
	bEnableGravity = true;
	bReverseRotationYaw = false;
}

void UUSBMovementComponent::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

}

// FVector::GetClampedToMaxSize(MaxSzie)가 이상하게 동작해서 따로 작성하였음.
FVector UUSBMovementComponent::CustomGetClampedToMaxSize(FVector Vect, float MaxSize) const {
	// 너무 작다면 무시
	if (Vect.IsNearlyZero()) {
		return Vect;
	}

	float Scale = 1.0f;

	// 만약 벡터의 길이가 MaxSize보다 크면 MaxSize에 맞춰준다.
	if (Vect.SizeSquared() > MaxSize* MaxSize)
	{
		Scale = MaxSize / Vect.Size();
	}

	return Vect * Scale;
}

// FVector::GetSafeNormal()이 이상하게 동작해서 따로 복사
FVector UUSBMovementComponent::CustomGetSafeNormal(FVector Vect, float Tolerance) const {
	const float SquareSum = Vect.SizeSquared();

	if (SquareSum == 1.f)
	{
		return Vect;
	}
	else if (SquareSum < Tolerance)
	{
		return FVector::ZeroVector;
	}
	//const float Scale = Vect.Size();
	return Vect / Vect.Size();//FVector(Vect.X * Scale, Vect.Y * Scale, Vect.Z * Scale);
}

// FVector::GetSafeNormal2D()가 이상하게 동작해서 따로 복사
FVector UUSBMovementComponent::CustomGetSafeNormal2D(FVector Vect, float Tolerance) const {
	const float SquareSum = Vect.SizeSquared2D();// Vect.X * Vect.X + Vect.Y * Vect.Y;

	if (SquareSum == 1.f)
	{
		if (Vect.Z == 0.f)
		{
			return Vect;
		}
		else
		{
			return FVector(Vect.X, Vect.Y, 0.f);
		}
	}
	else if (SquareSum < Tolerance)
	{
		return FVector::ZeroVector;
	}

	return FVector(Vect.X, Vect.Y, 0.f) / Vect.Size2D();
}



USphereComponent* UUSBMovementComponent::GetMovementSphere() const {
	if (!(PawnOwner && UpdatedComponent)) {
		return NULL;
	}

	return Cast<USphereComponent>(UpdatedComponent);
}

void OnLandedGround(const FHitResult& Hit) {
/*블루프린트에서 오버라이드*/
}

/**********************************
*
*			  틱
*
**********************************/
void UUSBMovementComponent::MyTickMoving(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	/*
	if (MovementMode == MOVE_Walking) {
		GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Yellow, TEXT("MOVE_Walking"));
	}
	else if (MovementMode == MOVE_Falling) {
		GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Yellow, TEXT("MOVE_Falling"));
	}
	*/

	//CharacterMovementComponent Tick 이벤트 1248번째 줄
	CheckJumpInput(DeltaTime);

	const FVector InputVector = ConsumeInputVector();
	Acceleration = ScaleInputAcceleration(ConstrainInputAcceleration(InputVector));
	AnalogInputModifier = ComputeAnalogInputModifier();


	// 이동 가능한 상태인지 확인
	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime)) {
		return;
	}

	PerformMovement(DeltaTime);


	if (bEnablePhysicsInteraction)
	{
		SCOPE_CYCLE_COUNTER(STAT_CharPhysicsInteraction);
		ApplyDownwardForce(DeltaTime);
		ApplyRepulsionForce(DeltaTime);
	}
}


void UUSBMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	
	MyTickMoving(DeltaTime, TickType, ThisTickFunction);
	


	//SimulateMovement(DeltaTime);


}

/**************************************************************


	이 아래로는 CharacterMovement 복사 후 수정한 소스


**************************************************************/

// 543
#if WITH_EDITOR
void UUSBMovementComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const UProperty* PropertyThatChanged = PropertyChangedEvent.MemberProperty;
	if (PropertyThatChanged && PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UUSBMovementComponent, WalkableFloorAngle))
	{
		// Compute WalkableFloorZ from the Angle.
		SetWalkableFloorAngle(WalkableFloorAngle);
	}
}
#endif // WITH_EDITOR

// 632
void UUSBMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{

	if (NewUpdatedComponent)
	{
		const APawn* NewPawnOwner = Cast<APawn>(NewUpdatedComponent->GetOwner());
		if (NewPawnOwner == NULL)
		{
			//UE_LOG(LogCharacterMovement, Error, TEXT("%s owned by %s must update a component owned by a Pawn"), *GetName(), *GetNameSafe(NewUpdatedComponent->GetOwner()));
			return;
		}

		// check that UpdatedComponent is a Sphere
		if (Cast<USphereComponent>(NewUpdatedComponent) == NULL)
		{
			//UE_LOG(LogCharacterMovement, Error, TEXT("%s owned by %s must update a sphere component"), *GetName(), *GetNameSafe(NewUpdatedComponent->GetOwner()));
			return;
		}
	}

	if (bMovementInProgress)
	{
		// failsafe to avoid crashes in CharacterMovement. 
		bDeferUpdateMoveComponent = true;
		DeferredUpdatedMoveComponent = NewUpdatedComponent;
		return;
	}
	bDeferUpdateMoveComponent = false;
	DeferredUpdatedMoveComponent = NULL;

	USceneComponent* OldUpdatedComponent = UpdatedComponent;
	UPrimitiveComponent* OldPrimitive = Cast<UPrimitiveComponent>(UpdatedComponent);
	if (IsValid(OldPrimitive) && OldPrimitive->OnComponentBeginOverlap.IsBound())
	{
		// 이거 뭔지 모르겠음;
		OldPrimitive->OnComponentBeginOverlap.RemoveDynamic(this, &UUSBMovementComponent::CapsuleTouched);
	}

	Super::SetUpdatedComponent(NewUpdatedComponent);	// PawnMovementComponent.cpp에서 PawnOwner를 초기화

	if (UpdatedComponent != OldUpdatedComponent)
	{
		ClearAccumulatedForces();
	}

	if (UpdatedComponent == NULL)
	{
		StopActiveMovement();
	}

	const bool bValidUpdatedPrimitive = IsValid(UpdatedPrimitive);

	if (bValidUpdatedPrimitive && bEnablePhysicsInteraction)
	{
		// 이거 뭔지 모르겠음;
		UpdatedPrimitive->OnComponentBeginOverlap.AddUniqueDynamic(this, &UUSBMovementComponent::CapsuleTouched);
	}

	if (bNeedsSweepWhileWalkingUpdate)
	{
		bSweepWhileNavWalking = bValidUpdatedPrimitive ? UpdatedPrimitive->GetGenerateOverlapEvents() : false;
		bNeedsSweepWhileWalkingUpdate = false;
	}
}

// 704
bool UUSBMovementComponent::HasValidData() const
{
	const bool bIsValid = UpdatedComponent && IsValid(PawnOwner);
#if ENABLE_NAN_DIAGNOSTIC
	if (bIsValid)
	{
		// NaN-checking updates
		if (Velocity.ContainsNaN())
		{
			logOrEnsureNanError(TEXT("UUSBMovementComponent::HasValidData() detected NaN/INF for (%s) in Velocity:\n%s"), *GetPathNameSafe(this), *Velocity.ToString());
			UUSBMovementComponent* MutableThis = const_cast<UUSBMovementComponent*>(this);
			MutableThis->Velocity = FVector::ZeroVector;
		}
		if (!UpdatedComponent->GetComponentTransform().IsValid())
		{
			logOrEnsureNanError(TEXT("UUSBMovementComponent::HasValidData() detected NaN/INF for (%s) in UpdatedComponent->ComponentTransform:\n%s"), *GetPathNameSafe(this), *UpdatedComponent->GetComponentTransform().ToHumanReadableString());
		}
		if (UpdatedComponent->GetComponentRotation().ContainsNaN())
		{
			logOrEnsureNanError(TEXT("UUSBMovementComponent::HasValidData() detected NaN/INF for (%s) in UpdatedComponent->GetComponentRotation():\n%s"), *GetPathNameSafe(this), *UpdatedComponent->GetComponentRotation().ToString());
		}
	}
#endif
	return bIsValid;
}

// 730
FCollisionShape UUSBMovementComponent::GetPawnCapsuleCollisionShape(const EShrinkCapsuleExtent ShrinkMode, const float CustomShrinkAmount) const
{
	FVector Extent = GetPawnCapsuleExtent(ShrinkMode, CustomShrinkAmount);
	return FCollisionShape::MakeCapsule(Extent);
}

// 736
FVector UUSBMovementComponent::GetPawnCapsuleExtent(const EShrinkCapsuleExtent ShrinkMode, const float CustomShrinkAmount) const
{
	check(PawnOwner);

	float Radius, HalfHeight;
	Radius = HalfHeight = GetMovementSphere()->GetScaledSphereRadius();
	FVector CapsuleExtent(Radius, Radius, HalfHeight);

	float RadiusEpsilon = 0.f;
	float HeightEpsilon = 0.f;

	switch (ShrinkMode)
	{
	case SHRINK_None:
		return CapsuleExtent;

	case SHRINK_RadiusCustom:
		RadiusEpsilon = CustomShrinkAmount;
		break;

	case SHRINK_HeightCustom:
		HeightEpsilon = CustomShrinkAmount;
		break;

	case SHRINK_AllCustom:
		RadiusEpsilon = CustomShrinkAmount;
		HeightEpsilon = CustomShrinkAmount;
		break;

	default:
		//UE_LOG(LogCharacterMovement, Warning, TEXT("Unknown EShrinkCapsuleExtent in UCharacterMovementComponent::GetCapsuleExtent"));
		break;
	}

	// Don't shrink to zero extent.
	const float MinExtent = KINDA_SMALL_NUMBER * 10.f;
	CapsuleExtent.X = FMath::Max(CapsuleExtent.X - RadiusEpsilon, MinExtent);
	CapsuleExtent.Y = CapsuleExtent.X;
	CapsuleExtent.Z = FMath::Max(CapsuleExtent.Z - HeightEpsilon, MinExtent);

	return CapsuleExtent;
}

// 780
bool UUSBMovementComponent::DoJump()
{
	if (PawnOwner && CanJump())
	{
		// Don't jump if we can't move up/down.
		if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
		{
			Velocity.Z = FMath::Max(Velocity.Z, JumpZVelocity);
			SetMovementMode(MOVE_Falling);
			return true;
		}
	}

	return false;
}

// 796
// 이 메소드가 있기 때문에 지면이 움직여도 그에 맞춰서 컴포넌트가 같이 움직일 수 있음!!
FVector UUSBMovementComponent::GetImpartedMovementBaseVelocity() const
{
	FVector Result = FVector::ZeroVector;
	if (PawnOwner)
	{
		UPrimitiveComponent* MovementBase = GetMovementBase();//CurrentFloor.HitResult.GetComponent();//PawnOwner->GetMovementBase();

		if (MovementBaseUtility::IsDynamicBase(MovementBase))
		{
			FVector BaseVelocity = MovementBaseUtility::GetMovementBaseVelocity(MovementBase, GetBasedMovement().BoneName);//PawnOwner->GetBasedMovement().BoneName);

			if (bImpartBaseAngularVelocity)
			{
				//CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()지만 Sphere에선 HalfHeight이랑 Radius랑 같음
				const FVector CharacterBasePosition = (UpdatedComponent->GetComponentLocation() - FVector(0.f, 0.f, GetMovementSphere()->GetScaledSphereRadius()));
				const FVector BaseTangentialVel = MovementBaseUtility::GetMovementBaseTangentialVelocity(MovementBase, GetBasedMovement().BoneName, CharacterBasePosition);//PawnOwner->GetBasedMovement().BoneName, CharacterBasePosition);
				BaseVelocity += BaseTangentialVel;
			}

			if (bImpartBaseVelocityX)
			{
				Result.X = BaseVelocity.X;
			}
			if (bImpartBaseVelocityY)
			{
				Result.Y = BaseVelocity.Y;
			}
			if (bImpartBaseVelocityZ)
			{
				Result.Z = BaseVelocity.Z;
			}
		}
	}

	return Result;
}

// 831
void UUSBMovementComponent::Launch(FVector const& LaunchVel)
{
	if ((MovementMode != MOVE_None) && IsActive() && HasValidData())
	{
		PendingLaunchVelocity = LaunchVel;
	}
}

// 839
bool UUSBMovementComponent::HandlePendingLaunch()
{
	if (!PendingLaunchVelocity.IsZero() && HasValidData())
	{
		Velocity = PendingLaunchVelocity;
		SetMovementMode(MOVE_Falling);
		PendingLaunchVelocity = FVector::ZeroVector;
		bForceNextFloorCheck = true;
		return true;
	}

	return false;
}

// 853
void UUSBMovementComponent::JumpOff(AActor* MovementBaseActor)
{
	if (!bPerformingJumpOff)
	{
		bPerformingJumpOff = true;
		if (PawnOwner)
		{
			const float MaxSpeed = GetMaxSpeed() * 0.85f;
			Velocity += MaxSpeed * GetBestDirectionOffActor(MovementBaseActor);
			if (Velocity.Size2D() > MaxSpeed)
			{
				Velocity = MaxSpeed * CustomGetSafeNormal(Velocity);
			}
			Velocity.Z = JumpOffJumpZFactor * JumpZVelocity;
			SetMovementMode(MOVE_Falling);
		}
		bPerformingJumpOff = false;
	}
}

// 873
FVector UUSBMovementComponent::GetBestDirectionOffActor(AActor* BaseActor) const
{
	// By default, just pick a random direction.  Derived character classes can choose to do more complex calculations,
	// such as finding the shortest distance to move in based on the BaseActor's Bounding Volume.
	const float RandAngle = FMath::DegreesToRadians(FMath::SRand() * 360.f);	//GetNetworkSafeRandomAngleDegrees());
	return CustomGetSafeNormal(FVector(FMath::Cos(RandAngle), FMath::Sin(RandAngle), 0.5f));
}

// 912
void UUSBMovementComponent::SetDefaultMovementMode()
{
	// check for water volume
	if (CanEverSwim() && IsInWater())
	{
		SetMovementMode(DefaultWaterMovementMode);
	}
	else if (!PawnOwner || MovementMode != DefaultLandMovementMode)
	{
		const float SavedVelocityZ = Velocity.Z;
		SetMovementMode(DefaultLandMovementMode);

		// Avoid 1-frame delay if trying to walk but walking fails at this location.
		if (MovementMode == MOVE_Walking && GetMovementBase() == NULL)
		{
			Velocity.Z = SavedVelocityZ; // Prevent temporary walking state from zeroing Z velocity.
			SetMovementMode(MOVE_Falling);
		}

	}
}

// 952
void UUSBMovementComponent::SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode)
{
	if (NewMovementMode != MOVE_Custom)
	{
		NewCustomMode = 0;
	}

	// If trying to use NavWalking but there is no navmesh, use walking instead.
	if (NewMovementMode == MOVE_NavWalking)
	{
		/*
		if (GetNavData() == nullptr)
		{
			NewMovementMode = MOVE_Walking;
		}
		*/
	}

	// Do nothing if nothing is changing.
	if (MovementMode == NewMovementMode)
	{
		// Allow changes in custom sub-mode.
		if ((NewMovementMode != MOVE_Custom) || (NewCustomMode == CustomMovementMode))
		{
			return;
		}
	}

	const EMovementMode PrevMovementMode = MovementMode;
	const uint8 PrevCustomMode = CustomMovementMode;

	MovementMode = NewMovementMode;
	CustomMovementMode = NewCustomMode;

	// We allow setting movement mode before we have a component to update, in case this happens at startup.
	if (!HasValidData())
	{
		return;
	}

	// Handle change in movement mode
	OnMovementModeChanged(PrevMovementMode, PrevCustomMode);

	// @todo UE4 do we need to disable ragdoll physics here? Should this function do nothing if in ragdoll?
}

// 997
void UUSBMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (!HasValidData())
	{
		return;
	}

	// Update collision settings if needed
/* NavWalking은 적용하지 않을 생각
	if (MovementMode == MOVE_NavWalking)
	{
		GroundMovementMode = MovementMode;
		// Walking uses only XY velocity
		Velocity.Z = 0.f;
		SetNavWalkingPhysics(true);
	}
	else if (PreviousMovementMode == MOVE_NavWalking)
	{
		if (MovementMode == DefaultLandMovementMode || IsWalking())
		{
			const bool bSucceeded = TryToLeaveNavWalking();
			if (!bSucceeded)
			{
				return;
			}
		}
		else
		{
			SetNavWalkingPhysics(false);
		}
	}
*/

// React to changes in the movement mode.
	if (MovementMode == MOVE_Walking)
	{
		// Walking uses only XY velocity, and must be on a walkable floor, with a Base.
		Velocity.Z = 0.f;
		//bCrouchMaintainsBaseLocation = true;
		GroundMovementMode = MovementMode;

		// make sure we update our new floor/base on initial entry of the walking physics
		FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, false);
		AdjustFloorHeight();
		SetBaseFromFloor(CurrentFloor);
	}
	else
	{
		CurrentFloor.Clear();
		//bCrouchMaintainsBaseLocation = false;

		if (MovementMode == MOVE_Falling)
		{
			Velocity += GetImpartedMovementBaseVelocity();
			//CharacterOwner->Falling();	// 캐릭터 자손에서 따로 구현해주어야 하는 함수
		}

		SetBase(NULL);

		if (MovementMode == MOVE_None)
		{
			// Kill velocity and clear queued up events
			StopMovementKeepPathing();
			ResetJumpState();
			ClearAccumulatedForces();
		}
	}

	if (MovementMode == MOVE_Falling && PreviousMovementMode != MOVE_Falling)
	{
		IPathFollowingAgentInterface* PFAgent = GetPathFollowingAgent();
		if (PFAgent)
		{
			PFAgent->OnStartedFalling();
		}
	}

	//CharacterOwner->OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	// 여기부터 Character 의 OnMovementModeChanged
	if (!bPressedJump || !IsFalling())
	{
		ResetJumpState();
	}

	//K2_OnMovementModeChanged(PrevMovementMode, CharacterMovement->MovementMode, PrevCustomMode, CharacterMovement->CustomMovementMode);
	//MovementModeChangedDelegate.Broadcast(this, PrevMovementMode, PrevCustomMode);

	ensureMsgf(GroundMovementMode == MOVE_Walking || GroundMovementMode == MOVE_NavWalking, TEXT("Invalid GroundMovementMode %d. MovementMode: %d, PreviousMovementMode: %d"), GroundMovementMode.GetValue(), MovementMode.GetValue(), PreviousMovementMode);

};

// 1310
void UUSBMovementComponent::PostPhysicsTickComponent(float DeltaTime, FUSBMovementComponentPostPhysicsTickFunction& ThisTickFunction)
{
	if (bDeferUpdateBasedMovement)
	{
		FScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);
		UpdateBasedMovement(DeltaTime);
		SaveBaseLocation();
		bDeferUpdateBasedMovement = false;
	}
}

// 1632
void UUSBMovementComponent::SimulateMovement(float DeltaSeconds)
{

	if (!HasValidData() || UpdatedComponent->Mobility != EComponentMobility::Movable || UpdatedComponent->IsSimulatingPhysics())
	{
		return;
	}

	/**********************빼고 읽자**********************/
	//const bool bIsSimulatedProxy = (PawnOwner->Role == ROLE_SimulatedProxy);

	// Workaround for replication not being updated initially
	//if (bIsSimulatedProxy &&
	//	PawnOwner->ReplicatedMovement.Location.IsZero() &&
	//	PawnOwner->ReplicatedMovement.Rotation.IsZero() &&
	//	PawnOwner->ReplicatedMovement.LinearVelocity.IsZero())
	//{
	//	return;
	//}

	// If base is not resolved on the client, we should not try to simulate at all
	//if (PawnOwner->GetReplicatedBasedMovement().IsBaseUnresolved())
	//{
		// UE_LOG(LogCharacterMovement, Verbose, TEXT("Base for simulated character '%s' is not resolved on client, skipping SimulateMovement"), *PawnOwner->GetName());
	//	return;
	//}
	/**********************빼고 읽자**********************/

	FVector OldVelocity;
	FVector OldLocation;

	// Scoped updates can improve performance of multiple MoveComponent calls.
	{					/**********************빼고 읽자**********************/
		//			FScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);
					/**********************빼고 읽자**********************/


		//bool bHandledNetUpdate = false;
		//if (bIsSimulatedProxy)
		//{
						/**********************빼고 읽자**********************/
						// Handle network changes
		//				if (bNetworkUpdateReceived)
		//				{
		//					bNetworkUpdateReceived = false;
		//					bHandledNetUpdate = true;
							// UE_LOG(LogCharacterMovement, Verbose, TEXT("Proxy %s received net update"), *PawnOwner->GetName());
		//					if (bNetworkMovementModeChanged)
		//					{
		//						ApplyNetworkMovementMode(PawnOwner->GetReplicatedMovementMode());
		//						bNetworkMovementModeChanged = false;
		//					}
		//					else if (bJustTeleported || bForceNextFloorCheck)
		//					{
								// Make sure floor is current. We will continue using the replicated base, if there was one.
		//						bJustTeleported = false;
		//						UpdateFloorFromAdjustment();
		//					}
		//				}

						/**********************빼고 읽자**********************/


		//				else if (bForceNextFloorCheck)
		//				{
		//					UpdateFloorFromAdjustment();	// 다른 조건이 없다면 항상 포함해야 할듯
		//				}
		//}

		if (bForceNextFloorCheck)
		{
			UpdateFloorFromAdjustment();	// 다른 조건이 없다면 항상 포함해야 할듯
		}

		if (MovementMode != MOVE_None)
		{
			//TODO: Also ApplyAccumulatedForces()?
			HandlePendingLaunch();
		}
		ClearAccumulatedForces();


		if (MovementMode == MOVE_None)
		{
			return;
		}



		// 애초에 bIsSimulatedProxy 이게 false였으면 이 구간까지 못왔음
		const bool bSimGravityDisabled = false;//(bIsSimulatedProxy && PawnOwner->bSimGravityDisabled);		// Pawn엔 없다...
		const bool bZeroReplicatedGroundVelocity = (/*bIsSimulatedProxy && */IsMovingOnGround() && PawnOwner->ReplicatedMovement.LinearVelocity.IsZero());

		/**********************빼고 읽자**********************/
		// bSimGravityDisabled means velocity was zero when replicated and we were stuck in something. Avoid external changes in velocity as well.
		// Being in ground movement with zero velocity, we cannot simulate proxy velocities safely because we might not get any further updates from the server.
		if (bSimGravityDisabled || bZeroReplicatedGroundVelocity)
		{
			Velocity = FVector::ZeroVector;
		}
		/**********************빼고 읽자**********************/

		// Base 관련
		MaybeUpdateBasedMovement(DeltaSeconds);

		// simulated pawns predict location
		OldVelocity = Velocity;
		OldLocation = UpdatedComponent->GetComponentLocation();

		UpdateProxyAcceleration();

		/**********************빼고 읽자**********************/
		// May only need to simulate forward on frames where we haven't just received a new position update.
		//if (!bHandledNetUpdate || !bNetworkSkipProxyPredictionOnNetUpdate || !CharacterMovementCVars::NetEnableSkipProxyPredictionOnNetUpdate)
		//{
		/**********************빼고 읽자**********************/
			//UE_LOG(LogCharacterMovement, Verbose, TEXT("Proxy %s simulating movement"), *GetNameSafe(PawnOwner));
		FStepDownResult StepDownResult;
		MoveSmooth(Velocity, DeltaSeconds, &StepDownResult);

		// find floor and check if falling
		if (IsMovingOnGround() || MovementMode == MOVE_Falling)
		{
			if (StepDownResult.bComputedFloor)
			{
				// 이미 MoveSmooth 함수에서 계산되었다면
				CurrentFloor = StepDownResult.FloorResult;
			}
			else if (Velocity.Z <= 0.f)
			{
				// 지면에 이미 붙어있는 상태였다면
				FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, Velocity.IsZero(), NULL);
			}
			else
			{
				// 것도 아니고 밑도 끝도 없이 떨어지고 있는 중이라면 지면이 필요 없음
				CurrentFloor.Clear();
			}

			if (!CurrentFloor.IsWalkableFloor())
			{
				// 걸을 수 없는 지면이라면
				if (!bSimGravityDisabled)
				{
					// 중력을 적용 시켜야 한다면
					// No floor, must fall.
											// ***Character->IsJumpProvidingForce() ==>  점프가 진행중일 때 True 반환
											// ***True if jump is actively providing a force, such as when the jump key is held and the time it has been held is less than JumpMaxHoldTime.
					if (Velocity.Z <= 0.f || bApplyGravityWhileJumping || !IsJumpProvidingForce())
					{
						// 점프가 진행중이 아니거나 점프가 진행중이어도 중력을 적용한다면
						// 중력적용!
						Velocity = NewFallVelocity(Velocity, FVector(0.f, 0.f, GetGravityZ()), DeltaSeconds);
					}
				}

				// 떨어지는 상태로
				SetMovementMode(MOVE_Falling);
			}
			else
			{
				// 걸을 수 있는 지면이라면
				// Walkable floor
				if (IsMovingOnGround())
				{
					// Walking 상태라면
					// 지면과의 거리를 조정하고 해당 지면을 Base로 삼는다.
					AdjustFloorHeight();
					SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
				}
				else if (MovementMode == MOVE_Falling)
				{
					// Falling 상태라면
					if (CurrentFloor.FloorDist <= MIN_FLOOR_DIST || (bSimGravityDisabled && CurrentFloor.FloorDist <= MAX_FLOOR_DIST))
					{
						// Falling 상태인데 지면과의 거리가 조오오오금만 떨어져 있다면
						// 착지한셈 치고 지면에 착지하면서 해당 지면(일반 물리 오브젝트일 수 있으니) 물리력을 가한다.
						// Landed
						SetPostLandedPhysics(CurrentFloor.HitResult);
					}
					else
					{
						// 지면과 거리가 있다면
						if (!bSimGravityDisabled)
						{
							// 중력이 안꺼져 있다면 중력 적용
							// Continue falling.
							Velocity = NewFallVelocity(Velocity, FVector(0.f, 0.f, GetGravityZ()), DeltaSeconds);
						}

						// 지면은 더이상 의미가 없오
						CurrentFloor.Clear();
					}
				}
			}
		}
		//}

		/**********************빼고 읽자**********************/
		//else
		//{
			//UE_LOG(LogCharacterMovement, Verbose, TEXT("Proxy %s SKIPPING simulate movement"), *GetNameSafe(PawnOwner));
		//}
		/**********************빼고 읽자**********************/

		// consume path following requested velocity
		bHasRequestedVelocity = false;


		OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	} // End scoped movement update

	// Call custom post-movement events. These happen after the scoped movement completes in case the events want to use the current state of overlaps etc.
	CallMovementUpdateDelegate(DeltaSeconds, OldLocation, OldVelocity);

	SaveBaseLocation();
	UpdateComponentVelocity();
	bJustTeleported = false;

	FVector moveResult = UpdatedComponent->GetComponentLocation() + FVector(Velocity * DeltaSeconds);
	UpdatedComponent->SetWorldLocation(moveResult);


	LastUpdateLocation = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
	LastUpdateRotation = UpdatedComponent ? UpdatedComponent->GetComponentQuat() : FQuat::Identity;
	LastUpdateVelocity = Velocity;

}

// 1812 - 기존 SetBase는 결국 CharacterOwner의 SetBase를 호출하기 위한 메소드이므로 주석처리 해둠
//void UUSBMovementComponent::SetBase(UPrimitiveComponent* NewBase, FName BoneName, bool bNotifyActor)
//{
	// prevent from changing Base while server is NavWalking (no Base in that mode), so both sides are in sync
	// otherwise it will cause problems with position smoothing

//	if (PawnOwner)// && !bIsNavWalkingOnServer)
//	{
		// Pawn에는 Base가 따로 없어서 에러가남
		//PawnOwner->SetBase(NewBase, NewBase ? BoneName : NAME_None, bNotifyActor);
//	}
//}

// 1823
void UUSBMovementComponent::SetBaseFromFloor(const FFindFloorResult& FloorResult)
{
	if (FloorResult.IsWalkableFloor())
	{
		SetBase(FloorResult.HitResult.GetComponent(), FloorResult.HitResult.BoneName);
	}
	else
	{
		SetBase(nullptr);
	}
}

// 1835
void UUSBMovementComponent::MaybeUpdateBasedMovement(float DeltaSeconds)
{

	bDeferUpdateBasedMovement = false;

	UPrimitiveComponent* MovementBase = GetMovementBase();
	if (MovementBaseUtility::UseRelativeLocation(MovementBase))
	{
		// Need to see if anything we're on is simulating physics or has a parent that is.
		bool bBaseIsSimulatingPhysics = false;
		USceneComponent* AttachParent = MovementBase;
		while (!bBaseIsSimulatingPhysics && AttachParent)
		{
			bBaseIsSimulatingPhysics = AttachParent->IsSimulatingPhysics();
			AttachParent = AttachParent->GetAttachParent();
		}

		if (!bBaseIsSimulatingPhysics)
		{
			bDeferUpdateBasedMovement = false;
			UpdateBasedMovement(DeltaSeconds);
			// If previously simulated, go back to using normal tick dependencies.
			if (PostPhysicsTickFunction.IsTickFunctionEnabled())
			{
				PostPhysicsTickFunction.SetTickFunctionEnable(false);
				MovementBaseUtility::AddTickDependency(PrimaryComponentTick, MovementBase);
			}
		}
		else
		{
			// defer movement base update until after physics
			bDeferUpdateBasedMovement = true;
			// If previously not simulating, remove tick dependencies and use post physics tick function.
			if (!PostPhysicsTickFunction.IsTickFunctionEnabled())
			{
				PostPhysicsTickFunction.SetTickFunctionEnable(true);
				MovementBaseUtility::RemoveTickDependency(PrimaryComponentTick, MovementBase);
			}
		}
	}
	else
	{
		// Remove any previous physics tick dependencies. SetBase() takes care of the other dependencies.

		if (PostPhysicsTickFunction.IsTickFunctionEnabled())
		{
			PostPhysicsTickFunction.SetTickFunctionEnable(false);
		}

	}

}

// 1892 @todo UE4 - handle lift moving up and down through encroachment
void UUSBMovementComponent::UpdateBasedMovement(float DeltaSeconds)
{

	if (!HasValidData())
	{
		return;
	}

	const UPrimitiveComponent* MovementBase = GetMovementBase();
	if (!MovementBaseUtility::UseRelativeLocation(MovementBase))
	{
		return;
	}

	// 이거 지워도 괜찮나? 모르겠다.
	if (!IsValid(MovementBase) || !IsValid(MovementBase->GetOwner()))
	{
		SetBase(NULL);
		return;
	}

	// Ignore collision with bases during these movements.
	TGuardValue<EMoveComponentFlags> ScopedFlagRestore(MoveComponentFlags, MoveComponentFlags | MOVECOMP_IgnoreBases);

	FQuat DeltaQuat = FQuat::Identity;
	FVector DeltaPosition = FVector::ZeroVector;

	FQuat NewBaseQuat;
	FVector NewBaseLocation;
	if (!MovementBaseUtility::GetMovementBaseTransform(MovementBase, GetBasedMovement().BoneName, NewBaseLocation, NewBaseQuat))
	{
		return;
	}

	// Find change in rotation
	const bool bRotationChanged = !OldBaseQuat.Equals(NewBaseQuat, 1e-8f);
	if (bRotationChanged)
	{
		DeltaQuat = NewBaseQuat * OldBaseQuat.Inverse();
	}

	// only if base moved
	if (bRotationChanged || (OldBaseLocation != NewBaseLocation))
	{
		// Calculate new transform matrix of base actor (ignoring scale).
		const FQuatRotationTranslationMatrix OldLocalToWorld(OldBaseQuat, OldBaseLocation);
		const FQuatRotationTranslationMatrix NewLocalToWorld(NewBaseQuat, NewBaseLocation);

		if (PawnOwner->IsMatineeControlled())
		{
			FRotationTranslationMatrix HardRelMatrix(GetBasedMovement().Rotation, GetBasedMovement().Location);
			const FMatrix NewWorldTM = HardRelMatrix * NewLocalToWorld;
			const FQuat NewWorldRot = bIgnoreBaseRotation ? UpdatedComponent->GetComponentQuat() : NewWorldTM.ToQuat();
			MoveUpdatedComponent(NewWorldTM.GetOrigin() - UpdatedComponent->GetComponentLocation(), NewWorldRot, true);
		}
		else
		{
			FQuat FinalQuat = UpdatedComponent->GetComponentQuat();

			if (bRotationChanged && !bIgnoreBaseRotation)
			{
				// Apply change in rotation and pipe through FaceRotation to maintain axis restrictions
				const FQuat PawnOldQuat = UpdatedComponent->GetComponentQuat();
				const FQuat TargetQuat = DeltaQuat * FinalQuat;
				FRotator TargetRotator(TargetQuat);
				PawnOwner->FaceRotation(TargetRotator, 0.f);
				FinalQuat = UpdatedComponent->GetComponentQuat();

				if (PawnOldQuat.Equals(FinalQuat, 1e-6f))
				{
					// Nothing changed. This means we probably are using another rotation mechanism (bOrientToMovement etc). We should still follow the base object.
					// @todo: This assumes only Yaw is used, currently a valid assumption. This is the only reason FaceRotation() is used above really, aside from being a virtual hook.
					if (bOrientRotationToMovement || (bUseControllerDesiredRotation && PawnOwner->Controller))
					{
						TargetRotator.Pitch = 0.f;
						TargetRotator.Roll = 0.f;
						MoveUpdatedComponent(FVector::ZeroVector, TargetRotator, false);
						FinalQuat = UpdatedComponent->GetComponentQuat();
					}
				}

				// Pipe through ControlRotation, to affect camera.
				if (PawnOwner->Controller)
				{
					const FQuat PawnDeltaRotation = FinalQuat * PawnOldQuat.Inverse();
					FRotator FinalRotation = FinalQuat.Rotator();
					UpdateBasedRotation(FinalRotation, PawnDeltaRotation.Rotator());
					FinalQuat = UpdatedComponent->GetComponentQuat();
				}
			}

			// We need to offset the base of the character here, not its origin, so offset by half height
			float HalfHeight, Radius;
			Radius = HalfHeight = GetMovementSphere()->GetScaledSphereRadius();

			FVector const BaseOffset(0.0f, 0.0f, HalfHeight);
			FVector const LocalBasePos = OldLocalToWorld.InverseTransformPosition(UpdatedComponent->GetComponentLocation() - BaseOffset);
			FVector const NewWorldPos = ConstrainLocationToPlane(NewLocalToWorld.TransformPosition(LocalBasePos) + BaseOffset);
			DeltaPosition = ConstrainDirectionToPlane(NewWorldPos - UpdatedComponent->GetComponentLocation());

			// move attached actor
			if (bFastAttachedMove)
			{
				// we're trusting no other obstacle can prevent the move here
				UpdatedComponent->SetWorldLocationAndRotation(NewWorldPos, FinalQuat, false);
			}
			else
			{
				// hack - transforms between local and world space introducing slight error FIXMESTEVE - discuss with engine team: just skip the transforms if no rotation?
				FVector BaseMoveDelta = NewBaseLocation - OldBaseLocation;
				if (!bRotationChanged && (BaseMoveDelta.X == 0.f) && (BaseMoveDelta.Y == 0.f))
				{
					DeltaPosition.X = 0.f;
					DeltaPosition.Y = 0.f;
				}

				FHitResult MoveOnBaseHit(1.f);
				const FVector OldLocation = UpdatedComponent->GetComponentLocation();
				MoveUpdatedComponent(DeltaPosition, FinalQuat, true, &MoveOnBaseHit);
				if ((UpdatedComponent->GetComponentLocation() - (OldLocation + DeltaPosition)).IsNearlyZero() == false)
				{
					//OnUnableToFollowBaseMove(DeltaPosition, OldLocation, MoveOnBaseHit);
				}
			}
		}

		/*
		if (MovementBase->IsSimulatingPhysics() && CharacterOwner->GetMesh())
		{
			CharacterOwner->GetMesh()->ApplyDeltaToAllPhysicsTransforms(DeltaPosition, DeltaQuat);
		}
		*/
	}
}

// 2031
void UUSBMovementComponent::UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation)
{
	AController* Controller = PawnOwner ? PawnOwner->Controller : NULL;
	float ControllerRoll = 0.f;
	if (Controller && !bIgnoreBaseRotation)
	{
		FRotator const ControllerRot = Controller->GetControlRotation();
		ControllerRoll = ControllerRot.Roll;
		Controller->SetControlRotation(ControllerRot + ReducedRotation);
	}

	// Remove roll
	FinalRotation.Roll = 0.f;
	if (Controller)
	{
		FinalRotation.Roll = UpdatedComponent->GetComponentRotation().Roll;
		FRotator NewRotation = Controller->GetControlRotation();
		NewRotation.Roll = ControllerRoll;
		Controller->SetControlRotation(NewRotation);
	}
}

// 2066
void UUSBMovementComponent::PerformMovement(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_CharacterMovementPerformMovement);

	const UWorld* MyWorld = GetWorld();
	if (!HasValidData() || MyWorld == nullptr)
	{
		return;
	}



	// no movement if we can't move, or if currently doing physical simulation on UpdatedComponent
	if (MovementMode == MOVE_None || UpdatedComponent->Mobility != EComponentMobility::Movable || UpdatedComponent->IsSimulatingPhysics())
	{
		// Clear pending physics forces
		ClearAccumulatedForces();
		return;
	}

	// Force floor update if we've moved outside of CharacterMovement since last update.
	bForceNextFloorCheck |= (IsMovingOnGround() && UpdatedComponent->GetComponentLocation() != LastUpdateLocation);

	FVector OldVelocity;
	FVector OldLocation;

	// Scoped updates can improve performance of multiple MoveComponent calls.
	{
		FScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);

		MaybeUpdateBasedMovement(DeltaSeconds);	// 현재 일부 주석처리 차후 확인

		OldVelocity = Velocity;
		OldLocation = UpdatedComponent->GetComponentLocation();

		ApplyAccumulatedForces(DeltaSeconds);

		// Update the character state before we do our movement 
		// Crouching(웅크린) 상태 관련. 현재 주석처리
		UpdateCharacterStateBeforeMovement(DeltaSeconds);

		if (MovementMode == MOVE_NavWalking && bWantsToLeaveNavWalking)
		{
			//TryToLeaveNavWalking();
		}

		// Character::LaunchCharacter() has been deferred until now.
		HandlePendingLaunch();
		ClearAccumulatedForces();

		// NaN tracking
		devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("UUSBMovementComponent::PerformMovement: Velocity contains NaN (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

		// Clear jump input now, to allow movement events to trigger it for next update.
		ClearJumpInput(DeltaSeconds);

		// change position
		StartNewPhysics(DeltaSeconds, 0);

		if (!HasValidData())
		{
			return;
		}

		// Update character state based on change from movement
		// Crouching(웅크린) 상태 관련. 현재 주석처리
		UpdateCharacterStateAfterMovement(DeltaSeconds);

		if (!PawnOwner->IsMatineeControlled())
		{
			PhysicsRotation(DeltaSeconds);
		}

		// consume path following requested velocity
		bHasRequestedVelocity = false;

		OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	} // End scoped movement update

	// Call external post-movement events. These happen after the scoped movement completes in case the events want to use the current state of overlaps etc.
	CallMovementUpdateDelegate(DeltaSeconds, OldLocation, OldVelocity);

	SaveBaseLocation();
	UpdateComponentVelocity();

	const FVector NewLocation = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
	const FQuat NewRotation = UpdatedComponent ? UpdatedComponent->GetComponentQuat() : FQuat::Identity;

	LastUpdateLocation = NewLocation;
	LastUpdateRotation = NewRotation;
	LastUpdateVelocity = Velocity;

}

// 2430
void UUSBMovementComponent::CallMovementUpdateDelegate(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity)
{
	SCOPE_CYCLE_COUNTER(STAT_CharMoveUpdateDelegate);

	// Update component velocity in case events want to read it
	UpdateComponentVelocity();

	// Delegate (for blueprints)
	if (PawnOwner)
	{
		//PawnOwner->OnPawnMovementUpdated.Broadcast(DeltaTime, OldLocation, OldVelocity);	// OnPawnMovementUpdated가 없어서 에러남 191004
	}

}

// 2445
void UUSBMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity)
{
	// empty base implementation, intended for derived classes to override.
}

// 2451
void UUSBMovementComponent::SaveBaseLocation()
{
	if (!HasValidData())
	{
		return;
	}

	const UPrimitiveComponent* MovementBase = GetMovementBase();
	if (MovementBaseUtility::UseRelativeLocation(MovementBase) && !PawnOwner->IsMatineeControlled())
	{
		// Read transforms into OldBaseLocation, OldBaseQuat
		MovementBaseUtility::GetMovementBaseTransform(MovementBase, GetBasedMovement().BoneName, OldBaseLocation, OldBaseQuat);

		// Location
		const FVector RelativeLocation = UpdatedComponent->GetComponentLocation() - OldBaseLocation;

		// Rotation
		if (bIgnoreBaseRotation)
		{
			// Absolute rotation
			SaveRelativeBasedMovement(RelativeLocation, UpdatedComponent->GetComponentRotation(), false);
		}
		else
		{
			// Relative rotation
			const FRotator RelativeRotation = (FQuatRotationMatrix(UpdatedComponent->GetComponentQuat()) * FQuatRotationMatrix(OldBaseQuat).GetTransposed()).Rotator();
			SaveRelativeBasedMovement(RelativeLocation, RelativeRotation, true);
		}
	}

}

// 2731
void UUSBMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	// Check for a change in crouch state. Players toggle crouch by changing bWantsToCrouch.
	/*
	const bool bIsCrouching = IsCrouching();
	if (bIsCrouching && (!bWantsToCrouch || !CanCrouchInCurrentState()))
	{
		UnCrouch(false);
	}
	else if (!bIsCrouching && bWantsToCrouch && CanCrouchInCurrentState())
	{
		Crouch(false);
	}
	*/
}

// 2745
void UUSBMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	/*
	// Uncrouch if no longer allowed to be crouched
	if (IsCrouching() && !CanCrouchInCurrentState())
	{
		UnCrouch(false);
	}
	*/
}

// 2754
void UUSBMovementComponent::StartNewPhysics(float deltaTime, int32 Iterations)
{
	if ((deltaTime < MIN_TICK_TIME) || (Iterations >= MaxSimulationIterations) || !HasValidData())
	{
		return;
	}

	if (UpdatedComponent->IsSimulatingPhysics())
	{
		//UE_LOG(LogCharacterMovement, Log, TEXT("UUSBMovementComponent::StartNewPhysics: UpdateComponent (%s) is simulating physics - aborting."), *UpdatedComponent->GetPathName());
		return;
	}

	const bool bSavedMovementInProgress = bMovementInProgress;
	bMovementInProgress = true;

	switch (MovementMode)
	{
	case MOVE_None:
		break;
	case MOVE_Walking:
		PhysWalking(deltaTime, Iterations);
		break;
	case MOVE_NavWalking:
		//PhysNavWalking(deltaTime, Iterations);
		break;
	case MOVE_Falling:
		PhysFalling(deltaTime, Iterations);
		break;
	case MOVE_Flying:
		//PhysFlying(deltaTime, Iterations);
		break;
	case MOVE_Swimming:
		//PhysSwimming(deltaTime, Iterations);
		break;
	case MOVE_Custom:
		//PhysCustom(deltaTime, Iterations);
		break;
	default:
		//UE_LOG(LogCharacterMovement, Warning, TEXT("%s has unsupported movement mode %d"), *CharacterOwner->GetName(), int32(MovementMode));
		SetMovementMode(MOVE_None);
		break;
	}

	bMovementInProgress = bSavedMovementInProgress;
	if (bDeferUpdateMoveComponent)
	{
		SetUpdatedComponent(DeferredUpdatedMoveComponent);
	}
}

// 2805
float UUSBMovementComponent::GetGravityZ() const
{
	return (bEnableGravity ? (Super::GetGravityZ() * GravityScale) : 0.f);
}

// 2810
float UUSBMovementComponent::GetMaxSpeed() const
{
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		return IsCrouching() ? MaxWalkSpeedCrouched : MaxWalkSpeed;
	case MOVE_Falling:
		return MaxWalkSpeed;
	case MOVE_Swimming:
		return MaxSwimSpeed;
	case MOVE_Flying:
		return MaxFlySpeed;
	case MOVE_Custom:
		return MaxCustomMovementSpeed;
	case MOVE_None:
	default:
		return 0.f;
	}
}

// 2831
float UUSBMovementComponent::GetMinAnalogSpeed() const
{
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
	case MOVE_Falling:
		return MinAnalogWalkSpeed;
	default:
		return 0.f;
	}
}

// 2873
float UUSBMovementComponent::SlideAlongSurface(const FVector& Delta, float Time, const FVector& InNormal, FHitResult& Hit, bool bHandleImpact)
{
	if (!Hit.bBlockingHit)
	{
		return 0.f;
	}

	FVector Normal(InNormal);
	if (IsMovingOnGround())
	{
		// We don't want to be pushed up an unwalkable surface.
		if (Normal.Z > 0.f)
		{
			if (!IsWalkable(Hit))
			{
				Normal = CustomGetSafeNormal2D(Normal);//Normal.GetSafeNormal2D();
			}
		}
		else if (Normal.Z < -KINDA_SMALL_NUMBER)
		{
			// Don't push down into the floor when the impact is on the upper portion of the capsule.
			if (CurrentFloor.FloorDist < MIN_FLOOR_DIST && CurrentFloor.bBlockingHit)
			{
				const FVector FloorNormal = CurrentFloor.HitResult.Normal;
				const bool bFloorOpposedToMovement = (Delta | FloorNormal) < 0.f && (FloorNormal.Z < 1.f - DELTA);
				if (bFloorOpposedToMovement)
				{
					Normal = FloorNormal;
				}

				Normal = CustomGetSafeNormal2D(Normal);// Normal.GetSafeNormal2D();
			}
		}
	}

	return Super::SlideAlongSurface(Delta, Time, Normal, Hit, bHandleImpact);
}

// 2912
void UUSBMovementComponent::TwoWallAdjust(FVector& Delta, const FHitResult& Hit, const FVector& OldHitNormal) const
{
	const FVector InDelta = Delta;
	Super::TwoWallAdjust(Delta, Hit, OldHitNormal);

	if (IsMovingOnGround())
	{
		// Allow slides up walkable surfaces, but not unwalkable ones (treat those as vertical barriers).
		if (Delta.Z > 0.f)
		{
			if ((Hit.Normal.Z >= WalkableFloorZ || IsWalkable(Hit)) && Hit.Normal.Z > KINDA_SMALL_NUMBER)
			{
				// Maintain horizontal velocity
				const float Time = (1.f - Hit.Time);
				const FVector ScaledDelta = CustomGetSafeNormal(Delta) * InDelta.Size();
				Delta = FVector(InDelta.X, InDelta.Y, ScaledDelta.Z / Hit.Normal.Z) * Time;

				// Should never exceed MaxStepHeight in vertical component, so rescale if necessary.
				// This should be rare (Hit.Normal.Z above would have been very small) but we'd rather lose horizontal velocity than go too high.
				if (Delta.Z > MaxStepHeight)
				{
					const float Rescale = MaxStepHeight / Delta.Z;
					Delta *= Rescale;
				}
			}
			else
			{
				Delta.Z = 0.f;
			}
		}
		else if (Delta.Z < 0.f)
		{
			// Don't push down into the floor.
			if (CurrentFloor.FloorDist < MIN_FLOOR_DIST && CurrentFloor.bBlockingHit)
			{
				Delta.Z = 0.f;
			}
		}
	}
}

// 2954
FVector UUSBMovementComponent::ComputeSlideVector(const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const
{
	FVector Result = Super::ComputeSlideVector(Delta, Time, Normal, Hit);

	// prevent boosting up slopes
	if (IsFalling())
	{
		Result = HandleSlopeBoosting(Result, Delta, Time, Normal, Hit);
	}

	return Result;
}

// 2968
FVector UUSBMovementComponent::HandleSlopeBoosting(const FVector& SlideResult, const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const
{
	FVector Result = SlideResult;

	if (Result.Z > 0.f)
	{
		// Don't move any higher than we originally intended.
		const float ZLimit = Delta.Z * Time;
		if (Result.Z - ZLimit > KINDA_SMALL_NUMBER)
		{
			if (ZLimit > 0.f)
			{
				// Rescale the entire vector (not just the Z component) otherwise we change the direction and likely head right back into the impact.
				const float UpPercent = ZLimit / Result.Z;
				Result *= UpPercent;
			}
			else
			{
				// We were heading down but were going to deflect upwards. Just make the deflection horizontal.
				Result = FVector::ZeroVector;
			}

			// Make remaining portion of original result horizontal and parallel to impact normal.
			const FVector RemainderXY = (SlideResult - Result) * FVector(1.f, 1.f, 0.f);
			const FVector NormalXY = CustomGetSafeNormal2D(Normal);	//Normal.GetSafeNormal2D();
			const FVector Adjust = Super::ComputeSlideVector(RemainderXY, 1.f, NormalXY, Hit);
			Result += Adjust;
		}
	}

	return Result;
}

// 3001
FVector UUSBMovementComponent::NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const
{
	FVector Result = InitialVelocity;

	if (DeltaTime > 0.f)
	{
		// Apply gravity.
		Result += Gravity * DeltaTime;

		// Don't exceed terminal velocity.
		const float TerminalLimit = FMath::Abs(GetPhysicsVolume()->TerminalVelocity);
		if (Result.SizeSquared() > FMath::Square(TerminalLimit))
		{
			const FVector GravityDir = CustomGetSafeNormal(Gravity);
			if ((Result | GravityDir) > TerminalLimit)
			{
				Result = FVector::PointPlaneProject(Result, FVector::ZeroVector, GravityDir) + GravityDir * TerminalLimit;
			}
		}
	}

	return Result;
}

// 3057
/*bool UUSBMovementComponent::IsFlying() const
{
	return (MovementMode == MOVE_Flying) && UpdatedComponent;
}*/

// 3062
bool UUSBMovementComponent::IsMovingOnGround() const
{
	return ((MovementMode == MOVE_Walking) || (MovementMode == MOVE_NavWalking)) && UpdatedComponent;
}

// 3067
bool UUSBMovementComponent::IsFalling() const
{
	return (MovementMode == MOVE_Falling) && UpdatedComponent;
}

// 3072
bool UUSBMovementComponent::IsSwimming() const
{
	return (MovementMode == MOVE_Swimming) && UpdatedComponent;
}

// 3077
/*bool UUSBMovementComponent::IsCrouching() const
{
	return PawnOwner && PawnOwner->bIsCrouched;
}*/

// 3082
void UUSBMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{

	//UE_LOG(LogTemp, Log, TEXT("1. Begin CalcVelocity"));
	// Do not update velocity when using root motion or when SimulatedProxy - SimulatedProxy are repped their Velocity
	if (!HasValidData() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	Friction = FMath::Max(0.f, Friction);
	const float MaxAccel = GetMaxAcceleration();
	float MaxSpeed = GetMaxSpeed();

	// Check if path following requested movement
	bool bZeroRequestedAcceleration = true;
	FVector RequestedAcceleration = FVector::ZeroVector;
	float RequestedSpeed = 0.0f;


	if (ApplyRequestedMove(DeltaTime, MaxAccel, MaxSpeed, Friction, BrakingDeceleration, RequestedAcceleration, RequestedSpeed))
	{
		bZeroRequestedAcceleration = false;
	}

	//UE_LOG(LogTemp, Log, TEXT("2. If bForceMaxAccel"));


	if (bForceMaxAccel)
	{
		// Force acceleration at full speed.
		// In consideration order for direction: Acceleration, then Velocity, then Pawn's rotation.
		if (Acceleration.SizeSquared() > SMALL_NUMBER)
		{
			Acceleration = CustomGetSafeNormal(Acceleration) * MaxAccel;
		}
		else
		{
			Acceleration = MaxAccel * (Velocity.SizeSquared() < SMALL_NUMBER ? UpdatedComponent->GetForwardVector() : CustomGetSafeNormal(Velocity));
		}

		AnalogInputModifier = 1.f;
	}


	// Path following above didn't care about the analog modifier, but we do for everything else below, so get the fully modified value.
	// Use max of requested speed and max speed if we modified the speed in ApplyRequestedMove above.
	const float MaxInputSpeed = FMath::Max(MaxSpeed * AnalogInputModifier, GetMinAnalogSpeed());
	MaxSpeed = FMath::Max(RequestedSpeed, MaxInputSpeed);


	// Apply braking or deceleration
	const bool bZeroAcceleration = Acceleration.IsZero();
	const bool bVelocityOverMax = IsExceedingMaxSpeed(MaxSpeed);


	// Only apply braking if there is no acceleration, or we are over our max speed and need to slow down to it.
	if ((bZeroAcceleration && bZeroRequestedAcceleration) || bVelocityOverMax)
	{
		const FVector OldVelocity = Velocity;
		const float ActualBrakingFriction = (bUseSeparateBrakingFriction ? BrakingFriction : Friction);
		//UE_LOG(LogTemp, Log, TEXT("****"));
		ApplyVelocityBraking(DeltaTime, ActualBrakingFriction, BrakingDeceleration);

		// Don't allow braking to lower us below max speed if we started above it.
		if (bVelocityOverMax && Velocity.SizeSquared() < FMath::Square(MaxSpeed) && FVector::DotProduct(Acceleration, OldVelocity) > 0.0f)
		{
			Velocity = CustomGetSafeNormal(OldVelocity) * MaxSpeed;
		}
	}
	else if (!bZeroAcceleration)
	{
		// Friction affects our ability to change direction. This is only done for input acceleration, not path following.
		const FVector AccelDir = (Acceleration.Size() == 0) ? FVector::ZeroVector : CustomGetSafeNormal(Acceleration);
		const float VelSize = Velocity.Size();
		Velocity = Velocity - (Velocity - AccelDir * VelSize) * FMath::Min(DeltaTime * Friction, 1.f);
	}

	// Apply fluid friction
	if (bFluid)
	{
		Velocity = Velocity * (1.f - FMath::Min(Friction * DeltaTime, 1.f));
	}

	// Apply input acceleration
	if (!bZeroAcceleration)
	{
		const float NewMaxInputSpeed = IsExceedingMaxSpeed(MaxInputSpeed) ? Velocity.Size() : MaxInputSpeed;
		Velocity += Acceleration * DeltaTime;
		Velocity = CustomGetClampedToMaxSize(Velocity, NewMaxInputSpeed);
	}

	// Apply additional requested acceleration
	if (!bZeroRequestedAcceleration)
	{
		const float NewMaxRequestedSpeed = IsExceedingMaxSpeed(RequestedSpeed) ? Velocity.Size() : RequestedSpeed;
		Velocity += RequestedAcceleration * DeltaTime;
		Velocity = CustomGetClampedToMaxSize(Velocity, NewMaxRequestedSpeed);
	}

	/*

	RVO 알고리즘을 통해 Ai 캐릭터가 장애물을 알아서 회피하며 이동할 수 있드록 하는 부분이지만
	서버에서만 동작한다고 하고, 우리 프로젝트에선 의미 없는거 같으니 사용하지 않는다.
	if (bUseRVOAvoidance)
	{
		CalcAvoidanceVelocity(DeltaTime);
	}
	*/
}

// 3179
bool UUSBMovementComponent::ApplyRequestedMove(float DeltaTime, float MaxAccel, float MaxSpeed, float Friction, float BrakingDeceleration, FVector& OutAcceleration, float& OutRequestedSpeed)
{
	if (bHasRequestedVelocity)
	{
		const float RequestedSpeedSquared = RequestedVelocity.SizeSquared();
		if (RequestedSpeedSquared < KINDA_SMALL_NUMBER)
		{
			return false;
		}

		// Compute requested speed from path following
		float RequestedSpeed = FMath::Sqrt(RequestedSpeedSquared);
		const FVector RequestedMoveDir = RequestedVelocity / RequestedSpeed;
		RequestedSpeed = (bRequestedMoveWithMaxSpeed ? MaxSpeed : FMath::Min(MaxSpeed, RequestedSpeed));

		// Compute actual requested velocity
		const FVector MoveVelocity = RequestedMoveDir * RequestedSpeed;

		// Compute acceleration. Use MaxAccel to limit speed increase, 1% buffer.
		FVector NewAcceleration = FVector::ZeroVector;
		const float CurrentSpeedSq = Velocity.SizeSquared();
		if (bRequestedMoveUseAcceleration && CurrentSpeedSq < FMath::Square(RequestedSpeed * 1.01f))
		{
			// Turn in the same manner as with input acceleration.
			const float VelSize = FMath::Sqrt(CurrentSpeedSq);
			Velocity = Velocity - (Velocity - RequestedMoveDir * VelSize) * FMath::Min(DeltaTime * Friction, 1.f);

			// How much do we need to accelerate to get to the new velocity?
			NewAcceleration = ((MoveVelocity - Velocity) / DeltaTime);
			NewAcceleration = CustomGetClampedToMaxSize(NewAcceleration, MaxAccel);
		}
		else
		{
			// Just set velocity directly.
			// If decelerating we do so instantly, so we don't slide through the destination if we can't brake fast enough.
			Velocity = MoveVelocity;
		}

		// Copy to out params
		OutRequestedSpeed = RequestedSpeed;
		OutAcceleration = NewAcceleration;
		return true;
	}

	return false;
}

// 3503
float UUSBMovementComponent::GetMaxJumpHeight() const
{
	const float Gravity = GetGravityZ();
	if (FMath::Abs(Gravity) > KINDA_SMALL_NUMBER)
	{
		return FMath::Square(JumpZVelocity) / (-2.f * Gravity);
	}
	else
	{
		return 0.f;
	}
}

// 3516
float UUSBMovementComponent::GetMaxJumpHeightWithJumpTime() const
{
	const float MaxJumpHeight = GetMaxJumpHeight();

	if (PawnOwner)
	{
		// When bApplyGravityWhileJumping is true, the actual max height will be lower than this.
		// However, it will also be dependent on framerate (and substep iterations) so just return this
		// to avoid expensive calculations.

		// This can be imagined as the character being displaced to some height, then jumping from that height.
		return (JumpMaxHoldTime * JumpZVelocity) + MaxJumpHeight;
	}

	return MaxJumpHeight;
}

// 3552
float UUSBMovementComponent::GetMaxAcceleration() const
{
	return MaxAcceleration;
}

// 3557
float UUSBMovementComponent::GetMaxBrakingDeceleration() const
{
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		return BrakingDecelerationWalking;
	case MOVE_Falling:
		return BrakingDecelerationFalling;
	case MOVE_Swimming:
		return BrakingDecelerationSwimming;
	case MOVE_Flying:
		return BrakingDecelerationFlying;
	case MOVE_Custom:
		return 0.f;
	case MOVE_None:
	default:
		return 0.f;
	}
}

// 3578
FVector UUSBMovementComponent::GetCurrentAcceleration() const
{
	return Acceleration;
}

// 3583
void UUSBMovementComponent::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration)
{
	if (Velocity.IsZero() || !HasValidData() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	const float FrictionFactor = FMath::Max(0.f, BrakingFrictionFactor);
	Friction = FMath::Max(0.f, Friction * FrictionFactor);
	BrakingDeceleration = FMath::Max(0.f, BrakingDeceleration);
	const bool bZeroFriction = (Friction == 0.f);
	const bool bZeroBraking = (BrakingDeceleration == 0.f);

	if (bZeroFriction && bZeroBraking)
	{
		return;
	}

	const FVector OldVel = Velocity;

	// subdivide braking to get reasonably consistent results at lower frame rates
	// (important for packet loss situations w/ networking)
	float RemainingTime = DeltaTime;
	const float MaxTimeStep = FMath::Clamp(BrakingSubStepTime, 1.0f / 75.0f, 1.0f / 20.0f);

	// Decelerate to brake to a stop
	const FVector RevAccel = (bZeroBraking ? FVector::ZeroVector : (-BrakingDeceleration * CustomGetSafeNormal(Velocity)));
	while (RemainingTime >= MIN_TICK_TIME)
	{
		// Zero friction uses constant deceleration, so no need for iteration.
		const float dt = ((RemainingTime > MaxTimeStep && !bZeroFriction) ? FMath::Min(MaxTimeStep, RemainingTime * 0.5f) : RemainingTime);
		RemainingTime -= dt;

		// apply friction and braking
		Velocity = Velocity + ((-Friction) * Velocity + RevAccel) * dt;

		// Don't reverse direction
		if ((Velocity | OldVel) <= 0.f)
		{
			Velocity = FVector::ZeroVector;
			return;
		}
	}

	// Clamp to zero if nearly zero, or if below min threshold and braking.
	const float VSizeSq = Velocity.SizeSquared();
	if (VSizeSq <= KINDA_SMALL_NUMBER || (!bZeroBraking && VSizeSq <= FMath::Square(BRAKE_TO_STOP_VELOCITY)))
	{
		Velocity = FVector::ZeroVector;
	}
}

// 4008
FVector UUSBMovementComponent::GetFallingLateralAcceleration(float DeltaTime)
{
	// No acceleration in Z
	FVector FallAcceleration = FVector(Acceleration.X, Acceleration.Y, 0.f);

	// bound acceleration, falling object has minimal ability to impact acceleration
	if (FallAcceleration.SizeSquared2D() > 0.f)
	{
		FallAcceleration = GetAirControl(DeltaTime, AirControl, FallAcceleration);
		FallAcceleration = CustomGetClampedToMaxSize(FallAcceleration, GetMaxAcceleration());
	}

	return FallAcceleration;
}

// 4024
FVector UUSBMovementComponent::GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration)
{
	// Boost
	if (TickAirControl != 0.f)
	{
		TickAirControl = BoostAirControl(DeltaTime, TickAirControl, FallAcceleration);
	}

	return TickAirControl * FallAcceleration;
}

// 4036
float UUSBMovementComponent::BoostAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration)
{
	// Allow a burst of initial acceleration
	if (AirControlBoostMultiplier > 0.f && Velocity.SizeSquared2D() < FMath::Square(AirControlBoostVelocityThreshold))
	{
		TickAirControl = FMath::Min(1.f, AirControlBoostMultiplier * TickAirControl);
	}

	return TickAirControl;
}

// 4048
void UUSBMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	SCOPE_CYCLE_COUNTER(STAT_CharPhysFalling);

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	FVector FallAcceleration = GetFallingLateralAcceleration(deltaTime);
	FallAcceleration.Z = 0.f;
	const bool bHasAirControl = (FallAcceleration.SizeSquared2D() > 0.f);

	float remainingTime = deltaTime;
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
	{
		Iterations++;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
		bJustTeleported = false;

		// RestorePreAdditiveRootMotionVelocity();

		FVector OldVelocity = Velocity;
		FVector VelocityNoAirControl = Velocity;

		// Apply input
		const float MaxDecel = GetMaxBrakingDeceleration();
		// Compute VelocityNoAirControl
		if (bHasAirControl)
		{
			// Find velocity *without* acceleration.
			TGuardValue<FVector> RestoreAcceleration(Acceleration, FVector::ZeroVector);
			TGuardValue<FVector> RestoreVelocity(Velocity, Velocity);
			Velocity.Z = 0.f;
			CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
			VelocityNoAirControl = FVector(Velocity.X, Velocity.Y, OldVelocity.Z);
		}

		// Compute Velocity
		{
			// Acceleration = FallAcceleration for CalcVelocity(), but we restore it after using it.
			TGuardValue<FVector> RestoreAcceleration(Acceleration, FallAcceleration);
			Velocity.Z = 0.f;
			CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
			Velocity.Z = OldVelocity.Z;
		}

		// Just copy Velocity to VelocityNoAirControl if they are the same (ie no acceleration).
		if (!bHasAirControl)
		{
			VelocityNoAirControl = Velocity;
		}

		// Apply gravity
		const FVector Gravity(0.f, 0.f, GetGravityZ());
		float GravityTime = timeTick;

		// If jump is providing force, gravity may be affected.
		if (JumpForceTimeRemaining > 0.0f)
		{
			// Consume some of the force time. Only the remaining time (if any) is affected by gravity when bApplyGravityWhileJumping=false.
			const float JumpForceTime = FMath::Min(JumpForceTimeRemaining, timeTick);
			GravityTime = bApplyGravityWhileJumping ? timeTick : FMath::Max(0.0f, timeTick - JumpForceTime);

			// Update Character state
			JumpForceTimeRemaining -= JumpForceTime;
			if (JumpForceTimeRemaining <= 0.0f)
			{
				ResetJumpState();
			}
		}

		Velocity = NewFallVelocity(Velocity, Gravity, GravityTime);
		VelocityNoAirControl = bHasAirControl ? NewFallVelocity(VelocityNoAirControl, Gravity, GravityTime) : Velocity;
		const FVector AirControlAccel = (Velocity - VelocityNoAirControl) / timeTick;

		// Move
		FHitResult Hit(1.f);
		FVector Adjusted = 0.5f * (OldVelocity + Velocity) * timeTick;
		SafeMoveUpdatedComponent(Adjusted, PawnRotation, true, Hit);

		if (!HasValidData())
		{
			return;
		}

		float LastMoveTimeSlice = timeTick;
		float subTimeTickRemaining = timeTick * (1.f - Hit.Time);

		if (IsSwimming()) //just entered water
		{
			// 수영수영
			//remainingTime += subTimeTickRemaining;
			//StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
			return;
		}
		else if (Hit.bBlockingHit)
		{
			if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
			{
				remainingTime += subTimeTickRemaining;
				ProcessLanded(Hit, remainingTime, Iterations);
				return;
			}
			else
			{
				// Compute impact deflection based on final velocity, not integration step.
				// This allows us to compute a new velocity from the deflected vector, and ensures the full gravity effect is included in the slide result.
				Adjusted = Velocity * timeTick;

				// See if we can convert a normally invalid landing spot (based on the hit result) to a usable one.
				if (!Hit.bStartPenetrating && ShouldCheckForValidLandingSpot(timeTick, Adjusted, Hit))
				{
					const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
					FFindFloorResult FloorResult;
					FindFloor(PawnLocation, FloorResult, false);
					if (FloorResult.IsWalkableFloor() && IsValidLandingSpot(PawnLocation, FloorResult.HitResult))
					{
						remainingTime += subTimeTickRemaining;
						ProcessLanded(FloorResult.HitResult, remainingTime, Iterations);
						return;
					}
				}

				HandleImpact(Hit, LastMoveTimeSlice, Adjusted);

				// If we've changed physics mode, abort.
				if (!HasValidData() || !IsFalling())
				{
					return;
				}

				// Limit air control based on what we hit.
				// We moved to the impact point using air control, but may want to deflect from there based on a limited air control acceleration.
				if (bHasAirControl)
				{
					const bool bCheckLandingSpot = false; // we already checked above.
					const FVector AirControlDeltaV = LimitAirControl(LastMoveTimeSlice, AirControlAccel, Hit, bCheckLandingSpot) * LastMoveTimeSlice;
					Adjusted = (VelocityNoAirControl + AirControlDeltaV) * LastMoveTimeSlice;
				}

				const FVector OldHitNormal = Hit.Normal;
				const FVector OldHitImpactNormal = Hit.ImpactNormal;
				FVector Delta = ComputeSlideVector(Adjusted, 1.f - Hit.Time, OldHitNormal, Hit);

				// Compute velocity after deflection (only gravity component for RootMotion)
				if (subTimeTickRemaining > KINDA_SMALL_NUMBER && !bJustTeleported)
				{
					const FVector NewVelocity = (Delta / subTimeTickRemaining);
					Velocity = NewVelocity;
				}

				if (subTimeTickRemaining > KINDA_SMALL_NUMBER && (Delta | Adjusted) > 0.f)
				{
					// Move in deflected direction.
					SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);

					if (Hit.bBlockingHit)
					{
						// hit second wall
						LastMoveTimeSlice = subTimeTickRemaining;
						subTimeTickRemaining = subTimeTickRemaining * (1.f - Hit.Time);

						if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
						{
							remainingTime += subTimeTickRemaining;
							ProcessLanded(Hit, remainingTime, Iterations);
							return;
						}

						HandleImpact(Hit, LastMoveTimeSlice, Delta);

						// If we've changed physics mode, abort.
						if (!HasValidData() || !IsFalling())
						{
							return;
						}

						// Act as if there was no air control on the last move when computing new deflection.
						if (bHasAirControl && Hit.Normal.Z > VERTICAL_SLOPE_NORMAL_Z)
						{
							const FVector LastMoveNoAirControl = VelocityNoAirControl * LastMoveTimeSlice;
							Delta = ComputeSlideVector(LastMoveNoAirControl, 1.f, OldHitNormal, Hit);
						}

						FVector PreTwoWallDelta = Delta;
						TwoWallAdjust(Delta, Hit, OldHitNormal);

						// Limit air control, but allow a slide along the second wall.
						if (bHasAirControl)
						{
							const bool bCheckLandingSpot = false; // we already checked above.
							const FVector AirControlDeltaV = LimitAirControl(subTimeTickRemaining, AirControlAccel, Hit, bCheckLandingSpot) * subTimeTickRemaining;

							// Only allow if not back in to first wall
							if (FVector::DotProduct(AirControlDeltaV, OldHitNormal) > 0.f)
							{
								Delta += (AirControlDeltaV * subTimeTickRemaining);
							}
						}

						// Compute velocity after deflection (only gravity component for RootMotion)
						if (subTimeTickRemaining > KINDA_SMALL_NUMBER && !bJustTeleported)
						{
							const FVector NewVelocity = (Delta / subTimeTickRemaining);
							Velocity = NewVelocity;
						}

						// bDitch=true means that pawn is straddling two slopes, neither of which he can stand on
						bool bDitch = ((OldHitImpactNormal.Z > 0.f) && (Hit.ImpactNormal.Z > 0.f) && (FMath::Abs(Delta.Z) <= KINDA_SMALL_NUMBER) && ((Hit.ImpactNormal | OldHitImpactNormal) < 0.f));
						SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
						if (Hit.Time == 0.f)
						{
							// if we are stuck then try to side step
							FVector SideDelta = CustomGetSafeNormal2D(OldHitNormal + Hit.ImpactNormal);//.GetSafeNormal2D();
							if (SideDelta.IsNearlyZero())
							{
								SideDelta = CustomGetSafeNormal(FVector(OldHitNormal.Y, -OldHitNormal.X, 0));
							}
							SafeMoveUpdatedComponent(SideDelta, PawnRotation, true, Hit);
						}

						if (bDitch || IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit) || Hit.Time == 0.f)
						{
							remainingTime = 0.f;
							ProcessLanded(Hit, remainingTime, Iterations);
							return;
						}
						else if (GetPerchRadiusThreshold() > 0.f && Hit.Time == 1.f && OldHitImpactNormal.Z >= WalkableFloorZ)
						{
							// We might be in a virtual 'ditch' within our perch radius. This is rare.
							const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
							const float ZMovedDist = FMath::Abs(PawnLocation.Z - OldLocation.Z);
							const float MovedDist2DSq = (PawnLocation - OldLocation).SizeSquared2D();
							if (ZMovedDist <= 0.2f * timeTick && MovedDist2DSq <= 4.f * timeTick)
							{
								Velocity.X += 0.25f * GetMaxSpeed() * (FMath::FRand() - 0.5f);
								Velocity.Y += 0.25f * GetMaxSpeed() * (FMath::FRand() - 0.5f);
								Velocity.Z = FMath::Max<float>(JumpZVelocity * 0.25f, 1.f);
								Delta = Velocity * timeTick;
								SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
							}
						}
					}
				}
			}
		}

		if (Velocity.SizeSquared2D() <= KINDA_SMALL_NUMBER * 10.f)
		{
			Velocity.X = 0.f;
			Velocity.Y = 0.f;
		}
	}
}

// 4320
FVector UUSBMovementComponent::LimitAirControl(float DeltaTime, const FVector& FallAcceleration, const FHitResult& HitResult, bool bCheckForValidLandingSpot)
{
	FVector Result(FallAcceleration);

	if (HitResult.IsValidBlockingHit() && HitResult.Normal.Z > VERTICAL_SLOPE_NORMAL_Z)
	{
		if (!bCheckForValidLandingSpot || !IsValidLandingSpot(HitResult.Location, HitResult))
		{
			// If acceleration is into the wall, limit contribution.
			if (FVector::DotProduct(FallAcceleration, HitResult.Normal) < 0.f)
			{
				// Allow movement parallel to the wall, but not into it because that may push us up.
				const FVector Normal2D = CustomGetSafeNormal2D(HitResult.Normal);
				Result = FVector::VectorPlaneProject(FallAcceleration, Normal2D);
			}
		}
	}
	else if (HitResult.bStartPenetrating)
	{
		// Allow movement out of penetration.
		return (FVector::DotProduct(Result, HitResult.Normal) > 0.f ? Result : FVector::ZeroVector);
	}

	return Result;
}

// 4346
bool UUSBMovementComponent::CheckLedgeDirection(const FVector& OldLocation, const FVector& SideStep, const FVector& GravDir) const
{
	const FVector SideDest = OldLocation + SideStep;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CheckLedgeDirection), false, PawnOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);
	const FCollisionShape CapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_None);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	FHitResult Result(1.f);
	GetWorld()->SweepSingleByChannel(Result, OldLocation, SideDest, FQuat::Identity, CollisionChannel, CapsuleShape, CapsuleParams, ResponseParam);

	if (!Result.bBlockingHit || IsWalkable(Result))
	{
		if (!Result.bBlockingHit)
		{
			UE_LOG(LogTemp, Log, TEXT("No Blocking Hit"));
			//GetWorld()->SweepSingleByChannel(Result, SideDest, SideDest + GravDir * (MaxStepHeight + LedgeCheckThreshold), FQuat::Identity, CollisionChannel, CapsuleShape, CapsuleParams, ResponseParam);
		}
		if ((Result.Time < 1.f) && IsWalkable(Result))
		{
			UE_LOG(LogTemp, Log, TEXT("Result.Time : %f"), Result.Time);
			return true;
		}
	}
	return false;
}

// 4372
FVector UUSBMovementComponent::GetLedgeMove(const FVector& OldLocation, const FVector& Delta, const FVector& GravDir) const
{
	if (!HasValidData() || Delta.IsZero())
	{
		return FVector::ZeroVector;
	}

	FVector SideDir(Delta.Y, -1.f * Delta.X, 0.f);

	// try left
	if (CheckLedgeDirection(OldLocation, SideDir, GravDir))
	{
		return SideDir;
	}

	// try right
	SideDir *= -1.f;
	if (CheckLedgeDirection(OldLocation, SideDir, GravDir))
	{
		return SideDir;
	}

	return FVector::ZeroVector;
}

// 4398 모서리 부분 넘어로 걸을 수 있는지
bool UUSBMovementComponent::CanWalkOffLedges() const
{
	/*
	if (!bCanWalkOffLedgesWhenCrouching && IsCrouching())
	{
		return false;
	}
	*/

	return bCanWalkOffLedges;
}

// 4408
bool UUSBMovementComponent::CheckFall(const FFindFloorResult& OldFloor, const FHitResult& Hit, const FVector& Delta, const FVector& OldLocation, float remainingTime, float timeTick, int32 Iterations, bool bMustJump)
{
	if (!HasValidData())
	{
		return false;
	}

	if (bMustJump || CanWalkOffLedges())
	{
		//HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
		if (IsMovingOnGround())
		{
			// If still walking, then fall. If not, assume the user set a different mode they want to keep.
			StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
		}
		return true;
	}

	return false;
}

// 4428
void UUSBMovementComponent::StartFalling(int32 Iterations, float remainingTime, float timeTick, const FVector& Delta, const FVector& subLoc)
{
	// start falling
	const float DesiredDist = Delta.Size();
	const float ActualDist = (UpdatedComponent->GetComponentLocation() - subLoc).Size2D();
	remainingTime = (DesiredDist < KINDA_SMALL_NUMBER)
		? 0.f
		: remainingTime + timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));

	if (IsMovingOnGround())
	{
		// This is to catch cases where the first frame of PIE is executed, and the
		// level is not yet visible. In those cases, the player will fall out of the
		// world... So, don't set MOVE_Falling straight away.
		if (!GIsEditor || (GetWorld()->HasBegunPlay() && (GetWorld()->GetTimeSeconds() >= 1.f)))
		{
			SetMovementMode(MOVE_Falling); //default behavior if script didn't change physics
		}
		else
		{
			// Make sure that the floor check code continues processing during this delay.
			bForceNextFloorCheck = true;
		}
	}

	StartNewPhysics(remainingTime, Iterations);
}

// 4456
void UUSBMovementComponent::RevertMove(const FVector& OldLocation, UPrimitiveComponent* OldBase, const FVector& PreviousBaseLocation, const FFindFloorResult& OldFloor, bool bFailMove)
{
	//UE_LOG(LogCharacterMovement, Log, TEXT("RevertMove from %f %f %f to %f %f %f"), CharacterOwner->Location.X, CharacterOwner->Location.Y, CharacterOwner->Location.Z, OldLocation.X, OldLocation.Y, OldLocation.Z);
	UpdatedComponent->SetWorldLocation(OldLocation, false, nullptr, GetTeleportType());

	//UE_LOG(LogCharacterMovement, Log, TEXT("Now at %f %f %f"), CharacterOwner->Location.X, CharacterOwner->Location.Y, CharacterOwner->Location.Z);
	bJustTeleported = false;
	// if our previous base couldn't have moved or changed in any physics-affecting way, restore it
	if (IsValid(OldBase) &&
		(!MovementBaseUtility::IsDynamicBase(OldBase) ||
		(OldBase->Mobility == EComponentMobility::Static) ||
			(OldBase->GetComponentLocation() == PreviousBaseLocation)
			)
		)
	{
		CurrentFloor = OldFloor;
		SetBase(OldBase, OldFloor.HitResult.BoneName);
	}
	else
	{
		SetBase(NULL);
	}

	if (bFailMove)
	{
		// end movement now
		Velocity = FVector::ZeroVector;
		Acceleration = FVector::ZeroVector;
		//UE_LOG(LogCharacterMovement, Log, TEXT("%s FAILMOVE RevertMove"), *CharacterOwner->GetName());
	}
}

// 4489
FVector UUSBMovementComponent::ComputeGroundMovementDelta(const FVector& Delta, const FHitResult& RampHit, const bool bHitFromLineTrace) const
{
	const FVector FloorNormal = RampHit.ImpactNormal;
	const FVector ContactNormal = RampHit.Normal;

	if (FloorNormal.Z < (1.f - KINDA_SMALL_NUMBER) && FloorNormal.Z > KINDA_SMALL_NUMBER&& ContactNormal.Z > KINDA_SMALL_NUMBER && !bHitFromLineTrace && IsWalkable(RampHit))
	{
		// Compute a vector that moves parallel to the surface, by projecting the horizontal movement direction onto the ramp.
		const float FloorDotDelta = (FloorNormal | Delta);
		FVector RampMovement(Delta.X, Delta.Y, -FloorDotDelta / FloorNormal.Z);

		if (bMaintainHorizontalGroundVelocity)
		{
			return RampMovement;
		}
		else
		{
			return CustomGetSafeNormal(RampMovement) * Delta.Size();
		}
	}

	return Delta;
}

// 4552
void UUSBMovementComponent::MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDownResult)
{

	if (!CurrentFloor.IsWalkableFloor())
	{
		return;
	}

	// Move along the current floor
	const FVector Delta = FVector(InVelocity.X, InVelocity.Y, 0.f) * DeltaSeconds;
	FHitResult Hit(1.f);
	FVector RampVector = ComputeGroundMovementDelta(Delta, CurrentFloor.HitResult, CurrentFloor.bLineTrace);
	SafeMoveUpdatedComponent(RampVector, UpdatedComponent->GetComponentQuat(), true, Hit);
	float LastMoveTimeSlice = DeltaSeconds;

	if (Hit.bStartPenetrating)	// SafeMoveUpdatedComponent를 통해 이동한 결과가 다른 콜리젼이랑 겹친 상황이라면
	{
		// Allow this hit to be used as an impact we can deflect off, otherwise we do nothing the rest of the update and appear to hitch.
		HandleImpact(Hit);	// 물리력을 가하는 메소드
		SlideAlongSurface(Delta, 1.f, Hit.Normal, Hit, true);

		if (Hit.bStartPenetrating)
		{
			//OnCharacterStuckInGeometry(&Hit);	// 끼었을 때 bJustTeleported = true 아직 복사 안함
		}
	}
	else if (Hit.IsValidBlockingHit())	// 일반적인 BlockingHit일 때
	{
		// We impacted something (most likely another ramp, but possibly a barrier).
		float PercentTimeApplied = Hit.Time;
		if ((Hit.Time > 0.f) && (Hit.Normal.Z > KINDA_SMALL_NUMBER) && IsWalkable(Hit))	// Hit된 지면이 경사면으로 추정된다면 경사면을 따라 걸음
		{
			// Another walkable ramp.
			const float InitialPercentRemaining = 1.f - PercentTimeApplied;
			RampVector = ComputeGroundMovementDelta(Delta * InitialPercentRemaining, Hit, false);
			LastMoveTimeSlice = InitialPercentRemaining * LastMoveTimeSlice;
			SafeMoveUpdatedComponent(RampVector, UpdatedComponent->GetComponentQuat(), true, Hit);

			const float SecondHitPercent = Hit.Time * InitialPercentRemaining;
			PercentTimeApplied = FMath::Clamp(PercentTimeApplied + SecondHitPercent, 0.f, 1.f);
		}


		if (Hit.IsValidBlockingHit())	// Hit 여부에 따라서 어떻게 이동할지 정함
		{
			if (CanStepUp(Hit) || (GetMovementBase() != NULL && GetMovementBase()->GetOwner() == Hit.GetActor()))
			{
				// hit a barrier, try to step up
				const FVector GravDir(0.f, 0.f, -1.f);
				if (!StepUp(GravDir, Delta * (1.f - PercentTimeApplied), Hit, OutStepDownResult))
				{
					//UE_LOG(LogCharacterMovement, Verbose, TEXT("- StepUp (ImpactNormal %s, Normal %s"), *Hit.ImpactNormal.ToString(), *Hit.Normal.ToString());
					HandleImpact(Hit, LastMoveTimeSlice, RampVector);
					SlideAlongSurface(Delta, 1.f - PercentTimeApplied, Hit.Normal, Hit, true);
				}
				else
				{
					// Don't recalculate velocity based on this height adjustment, if considering vertical adjustments.
					//UE_LOG(LogCharacterMovement, Verbose, TEXT("+ StepUp (ImpactNormal %s, Normal %s"), *Hit.ImpactNormal.ToString(), *Hit.Normal.ToString());
					bJustTeleported |= !bMaintainHorizontalGroundVelocity;
				}
			}
			else if (Hit.Component.IsValid() && !Hit.Component.Get()->CanCharacterStepUp(PawnOwner))
			{
				HandleImpact(Hit, LastMoveTimeSlice, RampVector);
				SlideAlongSurface(Delta, 1.f - PercentTimeApplied, Hit.Normal, Hit, true);
			}
		}
	}
}

// 4622
void UUSBMovementComponent::MaintainHorizontalGroundVelocity()
{
	if (Velocity.Z != 0.f)
	{
		if (bMaintainHorizontalGroundVelocity)
		{
			// Ramp movement already maintained the velocity, so we just want to remove the vertical component.
			Velocity.Z = 0.f;
		}
		else
		{
			// Rescale velocity to be horizontal but maintain magnitude of last update.
			Velocity = CustomGetSafeNormal2D(Velocity) * Velocity.Size();	// Velocity.GetSafeNormal2D()* Velocity.Size();
		}
	}
}

// 4640
void UUSBMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	SCOPE_CYCLE_COUNTER(STAT_CharPhysWalking);


	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!PawnOwner || (!PawnOwner->Controller && !bRunPhysicsWithNoController))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	if (!UpdatedComponent->IsQueryCollisionEnabled())
	{
		SetMovementMode(MOVE_Walking);
		return;
	}

	devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN before Iteration (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && PawnOwner && (PawnOwner->Controller || bRunPhysicsWithNoController))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent* const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;
		Acceleration.Z = 0.f;

		// Apply acceleration
		CalcVelocity(timeTick, GroundFriction, false, GetMaxBrakingDeceleration());
		devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if (bZeroDelta)
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if (IsFalling())
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
			else if (IsSwimming()) //just entered water
			{
				//StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		// 모서리 부분을 걸어다닐 수 있다면
		const bool bCheckLedges = CanWalkOffLedges();
		if (bCheckLedges && !CurrentFloor.IsWalkableFloor())
		{

			// calculate possible alternate movement
			const FVector GravDir = FVector(0.f, 0.f, -1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir);
			if (!NewDelta.IsZero())
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;

				UE_LOG(LogTemp, Log, TEXT("Try Ledge Move!!"));
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;


				UE_LOG(LogTemp, Log, TEXT("Fail Ledge Move!!"));
				break;
			}

		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{

				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					//HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}


				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if (IsSwimming())
			{
				//StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}

				bCheckedFall = true;
			}
		}


		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround())
		{
			// Make velocity reflect actual move
			if (!bJustTeleported && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}

	if (IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}
}

// 5190
bool UUSBMovementComponent::ShouldCatchAir(const FFindFloorResult& OldFloor, const FFindFloorResult& NewFloor)
{
	return false;
}

// 5203
void UUSBMovementComponent::AdjustFloorHeight()
{
	SCOPE_CYCLE_COUNTER(STAT_CharAdjustFloorHeight);

	// If we have a floor check that hasn't hit anything, don't adjust height.

	if (!CurrentFloor.IsWalkableFloor())
	{
		return;
	}

	float OldFloorDist = CurrentFloor.FloorDist;
	if (CurrentFloor.bLineTrace)
	{
		if (OldFloorDist < MIN_FLOOR_DIST && CurrentFloor.LineDist >= MIN_FLOOR_DIST)
		{
			// This would cause us to scale unwalkable walls
			//UE_LOG(LogCharacterMovement, VeryVerbose, TEXT("Adjust floor height aborting due to line trace with small floor distance (line: %.2f, sweep: %.2f)"), CurrentFloor.LineDist, CurrentFloor.FloorDist);
			return;
		}
		else
		{
			// Falling back to a line trace means the sweep was unwalkable (or in penetration). Use the line distance for the vertical adjustment.
			OldFloorDist = CurrentFloor.LineDist;
		}
	}

	// Move up or down to maintain floor height.
	if (OldFloorDist < MIN_FLOOR_DIST || OldFloorDist > MAX_FLOOR_DIST)
	{
		FHitResult AdjustHit(1.f);
		const float InitialZ = UpdatedComponent->GetComponentLocation().Z;
		const float AvgFloorDist = (MIN_FLOOR_DIST + MAX_FLOOR_DIST) * 0.5f;
		const float MoveDist = AvgFloorDist - OldFloorDist;
		SafeMoveUpdatedComponent(FVector(0.f, 0.f, MoveDist), UpdatedComponent->GetComponentQuat(), true, AdjustHit);
		//UE_LOG(LogCharacterMovement, VeryVerbose, TEXT("Adjust floor height %.3f (Hit = %d)"), MoveDist, AdjustHit.bBlockingHit);

		if (!AdjustHit.IsValidBlockingHit())
		{
			CurrentFloor.FloorDist += MoveDist;
		}
		else if (MoveDist > 0.f)
		{
			const float CurrentZ = UpdatedComponent->GetComponentLocation().Z;
			CurrentFloor.FloorDist += CurrentZ - InitialZ;
		}
		else
		{
			//checkSlow(MoveDist < 0.f);
			const float CurrentZ = UpdatedComponent->GetComponentLocation().Z;
			CurrentFloor.FloorDist = CurrentZ - AdjustHit.Location.Z;
			if (IsWalkable(AdjustHit))
			{
				CurrentFloor.SetFromSweep(AdjustHit, CurrentFloor.FloorDist, true);
			}
		}

		// Don't recalculate velocity based on this height adjustment, if considering vertical adjustments.
		// Also avoid it if we moved out of penetration
		bJustTeleported |= !bMaintainHorizontalGroundVelocity || (OldFloorDist < 0.f);

		// If something caused us to adjust our height (especially a depentration) we should ensure another check next frame or we will keep a stale result.
		bForceNextFloorCheck = true;
	}
}

// 5278
void UUSBMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	SCOPE_CYCLE_COUNTER(STAT_CharProcessLanded);

	if (IsFalling())
	{
		if (GroundMovementMode == MOVE_NavWalking)
		{
			/*
			// verify navmesh projection and current floor
			// otherwise movement will be stuck in infinite loop:
			// navwalking -> (no navmesh) -> falling -> (standing on something) -> navwalking -> ....

			const FVector TestLocation = GetActorFeetLocation();
			FNavLocation NavLocation;

			const bool bHasNavigationData = FindNavFloor(TestLocation, NavLocation);
			if (!bHasNavigationData || NavLocation.NodeRef == INVALID_NAVNODEREF)
			{
				GroundMovementMode = MOVE_Walking;
				//UE_LOG(LogNavMeshMovement, Verbose, TEXT("ProcessLanded(): %s tried to go to NavWalking but couldn't find NavMesh! Using Walking instead."), *GetNameSafe(CharacterOwner));
			}
			*/
		}

		SetPostLandedPhysics(Hit);
	}

	// AI를 위한 부분
	/*
	IPathFollowingAgentInterface* PFAgent = GetPathFollowingAgent();
	if (PFAgent)
	{
		PFAgent->OnLanded();
	}*/

	StartNewPhysics(remainingTime, Iterations);
}

// 5317
void UUSBMovementComponent::SetPostLandedPhysics(const FHitResult& Hit)
{
	if (PawnOwner)
	{
		if (CanEverSwim() && IsInWater())
		{
			SetMovementMode(MOVE_Swimming);
		}
		else
		{
			const FVector PreImpactAccel = Acceleration + (IsFalling() ? FVector(0.f, 0.f, GetGravityZ()) : FVector::ZeroVector);
			const FVector PreImpactVelocity = Velocity;

			if (DefaultLandMovementMode == MOVE_Walking ||
				DefaultLandMovementMode == MOVE_NavWalking ||
				DefaultLandMovementMode == MOVE_Falling)
			{
				SetMovementMode(GroundMovementMode);
				OnLandedGround(Hit);
			}
			else
			{
				SetDefaultMovementMode();
			}

			// 충돌한 물체에 힘을 가하는 함수이다.
			ApplyImpactPhysicsForces(Hit, PreImpactAccel, PreImpactVelocity);
		}
	}
}

// 5415
void UUSBMovementComponent::OnTeleported()
{
	UE_LOG(LogTemp, Log, TEXT("OnTeleported"));
	if (!HasValidData())
	{
		return;
	}

	Super::OnTeleported();

	bJustTeleported = true;
	UE_LOG(LogTemp, Log, TEXT("bJustTeleported = True"));

	// Find floor at current location
	UpdateFloorFromAdjustment();

	// Validate it. We don't want to pop down to walking mode from very high off the ground, but we'd like to keep walking if possible.
	UPrimitiveComponent* OldBase = PawnOwner->GetMovementBase();
	UPrimitiveComponent* NewBase = NULL;

	if (OldBase && CurrentFloor.IsWalkableFloor() && CurrentFloor.FloorDist <= MAX_FLOOR_DIST && Velocity.Z <= 0.f)
	{
		// Close enough to land or just keep walking.
		NewBase = CurrentFloor.HitResult.Component.Get();
	}
	else
	{
		CurrentFloor.Clear();
	}

	const bool bWasFalling = (MovementMode == MOVE_Falling);
	const bool bWasSwimming = (MovementMode == DefaultWaterMovementMode) || (MovementMode == MOVE_Swimming);

	if (CanEverSwim() && IsInWater())
	{
		if (!bWasSwimming)
		{
			SetMovementMode(DefaultWaterMovementMode);
		}
	}
	else if (!CurrentFloor.IsWalkableFloor() || (OldBase && !NewBase))
	{
		if (!bWasFalling && MovementMode != MOVE_Flying && MovementMode != MOVE_Custom)
		{
			SetMovementMode(MOVE_Falling);
		}
	}
	else if (NewBase)
	{
		if (bWasSwimming)
		{
			SetMovementMode(DefaultLandMovementMode);
		}
		else if (bWasFalling)
		{
			ProcessLanded(CurrentFloor.HitResult, 0.f, 0);
		}
	}

	SaveBaseLocation();
}

// 5475
float GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime)
{
	return (InAxisRotationRate >= 0.f) ? (InAxisRotationRate * DeltaTime) : 360.f;
}

// 5480
FRotator UUSBMovementComponent::GetDeltaRotation(float DeltaTime) const
{
	return FRotator(GetAxisDeltaRotation(RotationRate.Pitch, DeltaTime), GetAxisDeltaRotation(RotationRate.Yaw, DeltaTime), GetAxisDeltaRotation(RotationRate.Roll, DeltaTime));
}

// 5485
FRotator UUSBMovementComponent::ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const
{
	if (Acceleration.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		// AI path following request can orient us in that direction (it's effectively an acceleration)
		if (bHasRequestedVelocity && RequestedVelocity.SizeSquared() > KINDA_SMALL_NUMBER)
		{
			return CustomGetSafeNormal(RequestedVelocity).Rotation();
		}

		// Don't change rotation if there is no acceleration.
		return CurrentRotation;
	}

	if (bReverseRotationYaw) {
		FVector ReverseAcceleration = FVector(-Acceleration.X, -Acceleration.Y, Acceleration.Z);
		return CustomGetSafeNormal(ReverseAcceleration).Rotation();
	}

	// Rotate toward direction of acceleration.
	return CustomGetSafeNormal(Acceleration).Rotation();
}

// 5503
bool UUSBMovementComponent::ShouldRemainVertical() const
{
	// Always remain vertical when walking or falling.
	return IsMovingOnGround() || IsFalling();
}

// 5509
void UUSBMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (!(bOrientRotationToMovement || bUseControllerDesiredRotation))
	{
		return;
	}

	if (!HasValidData() || (!PawnOwner->Controller && !bRunPhysicsWithNoController))
	{
		return;
	}

	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
	CurrentRotation.DiagnosticCheckNaN(TEXT("USBMovementComponent::PhysicsRotation(): CurrentRotation"));

	FRotator DeltaRot = GetDeltaRotation(DeltaTime);
	DeltaRot.DiagnosticCheckNaN(TEXT("USBMovementComponent::PhysicsRotation(): GetDeltaRotation"));

	FRotator DesiredRotation = CurrentRotation;
	if (bOrientRotationToMovement)
	{
		DesiredRotation = ComputeOrientToMovementRotation(CurrentRotation, DeltaTime, DeltaRot);
		/*
		if (bReverseRotationYaw) {
			if (DesiredRotation.Yaw > KINDA_SMALL_NUMBER) {
				DesiredRotation.Add(0, 180, 0);
			}
		}*/
	}
	else if (PawnOwner->Controller && bUseControllerDesiredRotation)
	{
		DesiredRotation = PawnOwner->Controller->GetDesiredRotation();
	}
	else
	{
		return;
	}

	if (ShouldRemainVertical())
	{
		DesiredRotation.Pitch = 0.f;
		DesiredRotation.Yaw = FRotator::NormalizeAxis(DesiredRotation.Yaw);
		DesiredRotation.Roll = 0.f;
	}
	else
	{
		DesiredRotation.Normalize();
	}

	// Accumulate a desired new rotation.
	const float AngleTolerance = 1e-3f;

	if (!CurrentRotation.Equals(DesiredRotation, AngleTolerance))
	{
		// PITCH
		if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
		{
			DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
		}

		// YAW
		if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
		{
			DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
		}

		// ROLL
		if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
		{
			DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
		}

		// Set the new rotation.
		DesiredRotation.DiagnosticCheckNaN(TEXT("USBMovementComponent::PhysicsRotation(): DesiredRotation"));
		MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, /*bSweep*/ false);
	}
}


// 5681
void UUSBMovementComponent::AddImpulse(FVector Impulse, bool bVelocityChange)
{
	if (!Impulse.IsZero() && (MovementMode != MOVE_None) && IsActive() && HasValidData())
	{
		// handle scaling by mass
		FVector FinalImpulse = Impulse;
		if (!bVelocityChange)
		{
			if (Mass > SMALL_NUMBER)
			{
				FinalImpulse = FinalImpulse / Mass;
			}
			else
			{
				//UE_LOG(LogCharacterMovement, Warning, TEXT("Attempt to apply impulse to zero or negative Mass in CharacterMovement"));
			}
		}

		PendingImpulseToApply += FinalImpulse;
	}
}

// 5703
void UUSBMovementComponent::AddForce(FVector Force)
{
	if (!Force.IsZero() && (MovementMode != MOVE_None) && IsActive() && HasValidData())
	{
		if (Mass > SMALL_NUMBER)
		{
			PendingForceToApply += Force / Mass;
		}
		else
		{
			//UE_LOG(LogCharacterMovement, Warning, TEXT("Attempt to apply force to zero or negative Mass in CharacterMovement"));
		}
	}
}

// 5718
void UUSBMovementComponent::MoveSmooth(const FVector& InVelocity, const float DeltaSeconds, FStepDownResult* OutStepDownResult)
{
	/*
	if (!HasValidData())
	{
		return;
	}

	// Custom movement mode.
	// Custom movement may need an update even if there is zero velocity.
	if (MovementMode == MOVE_Custom)
	{
		FScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);
		PhysCustom(DeltaSeconds, 0);
		return;
	}

	FVector Delta = InVelocity * DeltaSeconds;
	if (Delta.IsZero())
	{
		return;
	}

	FScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);

	if (IsMovingOnGround())
	{
		MoveAlongFloor(InVelocity, DeltaSeconds, OutStepDownResult);
	}
	else
	{
		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			bool bSteppedUp = false;

			if (IsFlying())
			{
				if (CanStepUp(Hit))
				{
					OutStepDownResult = NULL; // No need for a floor when not walking.
					if (FMath::Abs(Hit.ImpactNormal.Z) < 0.2f)
					{
						const FVector GravDir = FVector(0.f, 0.f, -1.f);
						const FVector DesiredDir = CustomGetSafeNormal(Delta);
						const float UpDown = GravDir | DesiredDir;
						if ((UpDown < 0.5f) && (UpDown > -0.2f))
						{
							bSteppedUp = StepUp(GravDir, Delta * (1.f - Hit.Time), Hit, OutStepDownResult);
						}
					}
				}
			}

			// If StepUp failed, try sliding.
			if (!bSteppedUp)
			{
				SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, false);
			}
		}
	}
	*/
}

// 5783
void UUSBMovementComponent::UpdateProxyAcceleration()
{
	// Not currently replicated for simulated movement, but make it non-zero for animations that may want it, based on velocity.
	Acceleration = CustomGetSafeNormal(Velocity);
	AnalogInputModifier = 1.0f;
}

// 5790
bool UUSBMovementComponent::IsWalkable(const FHitResult& Hit) const
{
	if (!Hit.IsValidBlockingHit())
	{
		// No hit, or starting in penetration
		return false;
	}

	// Never walk up vertical surfaces.
	if (Hit.ImpactNormal.Z < KINDA_SMALL_NUMBER)
	{
		return false;
	}

	float TestWalkableZ = WalkableFloorZ;

	// See if this component overrides the walkable floor z.
	const UPrimitiveComponent* HitComponent = Hit.Component.Get();
	if (HitComponent)
	{
		const FWalkableSlopeOverride& SlopeOverride = HitComponent->GetWalkableSlopeOverride();
		TestWalkableZ = SlopeOverride.ModifyWalkableFloorZ(TestWalkableZ);
	}

	// Can't walk on this surface if it is too steep.
	if (Hit.ImpactNormal.Z < TestWalkableZ)
	{
		return false;
	}

	return true;
}

// 5823
void UUSBMovementComponent::SetWalkableFloorAngle(float InWalkableFloorAngle)
{
	WalkableFloorAngle = FMath::Clamp(InWalkableFloorAngle, 0.f, 90.0f);
	WalkableFloorZ = FMath::Cos(FMath::DegreesToRadians(WalkableFloorAngle));
}

// 5829
void UUSBMovementComponent::SetWalkableFloorZ(float InWalkableFloorZ)
{
	WalkableFloorZ = FMath::Clamp(InWalkableFloorZ, 0.f, 1.f);
	WalkableFloorAngle = FMath::RadiansToDegrees(FMath::Acos(WalkableFloorZ));
}

// 5835
float UUSBMovementComponent::K2_GetWalkableFloorAngle() const
{
	return GetWalkableFloorAngle();
}

// 5840
float UUSBMovementComponent::K2_GetWalkableFloorZ() const
{
	return GetWalkableFloorZ();
}

// 5847
bool UUSBMovementComponent::IsWithinEdgeTolerance(const FVector& CapsuleLocation, const FVector& TestImpactPoint, const float CapsuleRadius) const
{
	const float DistFromCenterSq = (TestImpactPoint - CapsuleLocation).SizeSquared2D();
	const float ReducedRadiusSq = FMath::Square(FMath::Max(SWEEP_EDGE_REJECT_DISTANCE + KINDA_SMALL_NUMBER, CapsuleRadius - SWEEP_EDGE_REJECT_DISTANCE));
	return DistFromCenterSq < ReducedRadiusSq;
}

// 5855
void UUSBMovementComponent::ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance, FFindFloorResult& OutFloorResult, float SweepRadius, const FHitResult* DownwardSweepResult) const
{
	//UE_LOG(LogCharacterMovement, VeryVerbose, TEXT("[Role:%d] ComputeFloorDist: %s at location %s"), (int32)PawnOwner->Role, *GetNameSafe(PawnOwner), *CapsuleLocation.ToString());
	OutFloorResult.Clear();

	float PawnRadius, PawnHalfHeight;	// 원래는 Capsule 기반
	//PawnOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
	PawnRadius = PawnHalfHeight = GetMovementSphere()->GetScaledSphereRadius();

	bool bSkipSweep = false;
	if (DownwardSweepResult != NULL && DownwardSweepResult->IsValidBlockingHit())
	{
		// Only if the supplied sweep was vertical and downward.
		if ((DownwardSweepResult->TraceStart.Z > DownwardSweepResult->TraceEnd.Z) &&
			(DownwardSweepResult->TraceStart - DownwardSweepResult->TraceEnd).SizeSquared2D() <= KINDA_SMALL_NUMBER)
		{
			// Reject hits that are barely on the cusp of the radius of the capsule
			if (IsWithinEdgeTolerance(DownwardSweepResult->Location, DownwardSweepResult->ImpactPoint, PawnRadius))
			{
				// Don't try a redundant sweep, regardless of whether this sweep is usable.
				bSkipSweep = true;

				const bool bIsWalkable = IsWalkable(*DownwardSweepResult);
				const float FloorDist = (CapsuleLocation.Z - DownwardSweepResult->Location.Z);
				OutFloorResult.SetFromSweep(*DownwardSweepResult, FloorDist, bIsWalkable);

				if (bIsWalkable)
				{
					// Use the supplied downward sweep as the floor hit result.			
					return;
				}
			}
		}
	}

	// We require the sweep distance to be >= the line distance, otherwise the HitResult can't be interpreted as the sweep result.
	if (SweepDistance < LineDistance)
	{
		ensure(SweepDistance >= LineDistance);
		return;
	}

	bool bBlockingHit = false;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ComputeFloorDist), false, PawnOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(QueryParams, ResponseParam);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();

	// Sweep test
	if (!bSkipSweep && SweepDistance > 0.f && SweepRadius > 0.f)
	{
		// Use a shorter height to avoid sweeps giving weird results if we start on a surface.
		// This also allows us to adjust out of penetrations.
		const float ShrinkScale = 0.9f;
		const float ShrinkScaleOverlap = 0.1f;
		float ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScale);
		float TraceDist = SweepDistance + ShrinkHeight;
		FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(SweepRadius, PawnHalfHeight - ShrinkHeight);

		FHitResult Hit(1.f);
		bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + FVector(0.f, 0.f, -TraceDist), CollisionChannel, CapsuleShape, QueryParams, ResponseParam);

		if (bBlockingHit)
		{
			// Reject hits adjacent to us, we only care about hits on the bottom portion of our capsule.
			// Check 2D distance to impact point, reject if within a tolerance from radius.
			if (Hit.bStartPenetrating || !IsWithinEdgeTolerance(CapsuleLocation, Hit.ImpactPoint, CapsuleShape.Capsule.Radius))
			{
				// Use a capsule with a slightly smaller radius and shorter height to avoid the adjacent object.
				// Capsule must not be nearly zero or the trace will fall back to a line trace from the start point and have the wrong length.
				CapsuleShape.Capsule.Radius = FMath::Max(0.f, CapsuleShape.Capsule.Radius - SWEEP_EDGE_REJECT_DISTANCE - KINDA_SMALL_NUMBER);
				if (!CapsuleShape.IsNearlyZero())
				{
					ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScaleOverlap);
					TraceDist = SweepDistance + ShrinkHeight;
					CapsuleShape.Capsule.HalfHeight = FMath::Max(PawnHalfHeight - ShrinkHeight, CapsuleShape.Capsule.Radius);
					Hit.Reset(1.f, false);

					bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + FVector(0.f, 0.f, -TraceDist), CollisionChannel, CapsuleShape, QueryParams, ResponseParam);
				}
			}

			// Reduce hit distance by ShrinkHeight because we shrank the capsule for the trace.
			// We allow negative distances here, because this allows us to pull out of penetrations.
			const float MaxPenetrationAdjust = FMath::Max(MAX_FLOOR_DIST, PawnRadius);
			const float SweepResult = FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight);

			OutFloorResult.SetFromSweep(Hit, SweepResult, false);
			if (Hit.IsValidBlockingHit() && IsWalkable(Hit))
			{
				if (SweepResult <= SweepDistance)
				{
					// Hit within test distance.
					OutFloorResult.bWalkableFloor = true;
					return;
				}
			}
		}
	}

	// Since we require a longer sweep than line trace, we don't want to run the line trace if the sweep missed everything.
	// We do however want to try a line trace if the sweep was stuck in penetration.
	if (!OutFloorResult.bBlockingHit && !OutFloorResult.HitResult.bStartPenetrating)
	{
		OutFloorResult.FloorDist = SweepDistance;
		return;
	}

	// Line trace
	if (LineDistance > 0.f)
	{
		const float ShrinkHeight = PawnHalfHeight;
		const FVector LineTraceStart = CapsuleLocation;
		const float TraceDist = LineDistance + ShrinkHeight;
		const FVector Down = FVector(0.f, 0.f, -TraceDist);
		QueryParams.TraceTag = SCENE_QUERY_STAT_NAME_ONLY(FloorLineTrace);

		FHitResult Hit(1.f);
		bBlockingHit = GetWorld()->LineTraceSingleByChannel(Hit, LineTraceStart, LineTraceStart + Down, CollisionChannel, QueryParams, ResponseParam);

		if (bBlockingHit)
		{
			if (Hit.Time > 0.f)
			{
				// Reduce hit distance by ShrinkHeight because we started the trace higher than the base.
				// We allow negative distances here, because this allows us to pull out of penetrations.
				const float MaxPenetrationAdjust = FMath::Max(MAX_FLOOR_DIST, PawnRadius);
				const float LineResult = FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight);

				OutFloorResult.bBlockingHit = true;
				if (LineResult <= LineDistance && IsWalkable(Hit))
				{
					OutFloorResult.SetFromLineTrace(Hit, OutFloorResult.FloorDist, LineResult, true);
					return;
				}
			}
		}
	}

	// No hits were acceptable.
	OutFloorResult.bWalkableFloor = false;
	OutFloorResult.FloorDist = SweepDistance;
}

// 5999
void UUSBMovementComponent::FindFloor(const FVector& CapsuleLocation, FFindFloorResult& OutFloorResult, bool bCanUseCachedLocation, const FHitResult* DownwardSweepResult) const
{
	SCOPE_CYCLE_COUNTER(STAT_CharFindFloor);

	// No collision, no floor...
	if (!HasValidData() || !UpdatedComponent->IsQueryCollisionEnabled())
	{
		OutFloorResult.Clear();
		return;
	}

	//UE_LOG(LogCharacterMovement, VeryVerbose, TEXT("[Role:%d] FindFloor: %s at location %s"), (int32)PawnOwner->Role, *GetNameSafe(PawnOwner), *CapsuleLocation.ToString());
	//check(PawnOwner->GetCapsuleComponent());

	// Increase height check slightly if walking, to prevent floor height adjustment from later invalidating the floor result.
	// 바닥이 위로 움직여 Floor Result가 무효화 되는걸 방지하기 위해 Walking상태에서의 높이를 지면보다 약간 높여둔다.
	const float HeightCheckAdjust = (IsMovingOnGround() ? MAX_FLOOR_DIST + KINDA_SMALL_NUMBER : -MAX_FLOOR_DIST);

	float FloorSweepTraceDist = FMath::Max(MAX_FLOOR_DIST, MaxStepHeight + HeightCheckAdjust);
	float FloorLineTraceDist = FloorSweepTraceDist;
	bool bNeedToValidateFloor = true;

	// Sweep floor
	if (FloorLineTraceDist > 0.f || FloorSweepTraceDist > 0.f)
	{
		UUSBMovementComponent* MutableThis = const_cast<UUSBMovementComponent*>(this);

		if (bAlwaysCheckFloor || !bCanUseCachedLocation || bForceNextFloorCheck || bJustTeleported)
		{
			MutableThis->bForceNextFloorCheck = false;												//PawnOwner->GetCapsuleComponent()->GetScaledCapsuleRadius()
			ComputeFloorDist(CapsuleLocation, FloorLineTraceDist, FloorSweepTraceDist, OutFloorResult, GetMovementSphere()->GetScaledSphereRadius(), DownwardSweepResult);
		}
		else
		{
			// Force floor check if base has collision disabled or if it does not block us.
			UPrimitiveComponent* MovementBase = GetMovementBase();
			const AActor* BaseActor = MovementBase ? MovementBase->GetOwner() : NULL;
			const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();

			if (MovementBase != NULL)
			{
				MutableThis->bForceNextFloorCheck = !MovementBase->IsQueryCollisionEnabled()
					|| MovementBase->GetCollisionResponseToChannel(CollisionChannel) != ECR_Block
					|| MovementBaseUtility::IsDynamicBase(MovementBase);
			}

			const bool IsActorBasePendingKill = BaseActor && BaseActor->IsPendingKill();

			if (!bForceNextFloorCheck && !IsActorBasePendingKill && MovementBase)
			{
				//UE_LOG(LogCharacterMovement, Log, TEXT("%s SKIP check for floor"), *PawnOwner->GetName());
				OutFloorResult = CurrentFloor;
				bNeedToValidateFloor = false;
			}
			else
			{
				MutableThis->bForceNextFloorCheck = false;												// PawnOwner->GetCapsuleComponent()->GetScaledCapsuleRadius()
				ComputeFloorDist(CapsuleLocation, FloorLineTraceDist, FloorSweepTraceDist, OutFloorResult, GetMovementSphere()->GetScaledSphereRadius(), DownwardSweepResult);
			}
		}
	}

	/***********************************************************************/

	// OutFloorResult.HitResult is now the result of the vertical floor check.
	// See if we should try to "perch" at this location.
	if (bNeedToValidateFloor && OutFloorResult.bBlockingHit && !OutFloorResult.bLineTrace)
	{
		const bool bCheckRadius = true;
		if (ShouldComputePerchResult(OutFloorResult.HitResult, bCheckRadius))
		{
			float MaxPerchFloorDist = FMath::Max(MAX_FLOOR_DIST, MaxStepHeight + HeightCheckAdjust);
			if (IsMovingOnGround())
			{
				MaxPerchFloorDist += FMath::Max(0.f, PerchAdditionalHeight);
			}

			FFindFloorResult PerchFloorResult;
			if (ComputePerchResult(GetValidPerchRadius(), OutFloorResult.HitResult, MaxPerchFloorDist, PerchFloorResult))
			{
				// Don't allow the floor distance adjustment to push us up too high, or we will move beyond the perch distance and fall next time.
				const float AvgFloorDist = (MIN_FLOOR_DIST + MAX_FLOOR_DIST) * 0.5f;
				const float MoveUpDist = (AvgFloorDist - OutFloorResult.FloorDist);
				if (MoveUpDist + PerchFloorResult.FloorDist >= MaxPerchFloorDist)
				{
					OutFloorResult.FloorDist = AvgFloorDist;
				}

				// If the regular capsule is on an unwalkable surface but the perched one would allow us to stand, override the normal to be one that is walkable.
				if (!OutFloorResult.bWalkableFloor)
				{
					OutFloorResult.SetFromLineTrace(PerchFloorResult.HitResult, OutFloorResult.FloorDist, FMath::Min(PerchFloorResult.FloorDist, PerchFloorResult.LineDist), true);
				}
			}
			else
			{
				// We had no floor (or an invalid one because it was unwalkable), and couldn't perch here, so invalidate floor (which will cause us to start falling).
				OutFloorResult.bWalkableFloor = false;
			}
		}
	}

}

// 6100
void UUSBMovementComponent::K2_FindFloor(FVector CapsuleLocation, FFindFloorResult& FloorResult) const
{
	const bool SavedForceNextFloorCheck(bForceNextFloorCheck);
	FindFloor(CapsuleLocation, FloorResult, false);

	// FindFloor clears this, but this is only a test not done during normal movement.
	UUSBMovementComponent* MutableThis = const_cast<UUSBMovementComponent*>(this);
	MutableThis->bForceNextFloorCheck = SavedForceNextFloorCheck;
}

// 6110
void UUSBMovementComponent::K2_ComputeFloorDist(FVector CapsuleLocation, float LineDistance, float SweepDistance, float SweepRadius, FFindFloorResult& FloorResult) const
{
	if (HasValidData())
	{
		SweepDistance = FMath::Max(SweepDistance, 0.f);
		LineDistance = FMath::Clamp(LineDistance, 0.f, SweepDistance);
		SweepRadius = FMath::Max(SweepRadius, 0.f);

		ComputeFloorDist(CapsuleLocation, LineDistance, SweepDistance, FloorResult, SweepRadius);
	}
}

// 6161
bool UUSBMovementComponent::IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const
{
	if (!Hit.bBlockingHit)
	{
		return false;
	}

	// Skip some checks if penetrating. Penetration will be handled by the FindFloor call (using a smaller capsule)
	if (!Hit.bStartPenetrating)
	{
		// Reject unwalkable floor normals.
		if (!IsWalkable(Hit))
		{
			return false;
		}

		float PawnRadius, PawnHalfHeight;
		PawnRadius = PawnHalfHeight = GetMovementSphere()->GetScaledSphereRadius();//(PawnRadius, PawnHalfHeight);

		// Reject hits that are above our lower hemisphere (can happen when sliding down a vertical surface).
		const float LowerHemisphereZ = Hit.Location.Z - PawnHalfHeight + PawnRadius;
		if (Hit.ImpactPoint.Z >= LowerHemisphereZ)
		{
			return false;
		}

		// Reject hits that are barely on the cusp of the radius of the capsule
		if (!IsWithinEdgeTolerance(Hit.Location, Hit.ImpactPoint, PawnRadius))
		{
			return false;
		}
	}
	else
	{
		// Penetrating
		if (Hit.Normal.Z < KINDA_SMALL_NUMBER)
		{
			// Normal is nearly horizontal or downward, that's a penetration adjustment next to a vertical or overhanging wall. Don't pop to the floor.
			return false;
		}
	}

	FFindFloorResult FloorResult;
	FindFloor(CapsuleLocation, FloorResult, false, &Hit);

	if (!FloorResult.IsWalkableFloor())
	{
		return false;
	}

	return true;
}

// 6123
bool UUSBMovementComponent::FloorSweepTest(
	FHitResult& OutHit,
	const FVector& Start,
	const FVector& End,
	ECollisionChannel TraceChannel,
	const struct FCollisionShape& CollisionShape,
	const struct FCollisionQueryParams& Params,
	const struct FCollisionResponseParams& ResponseParam
) const
{
	bool bBlockingHit = false;

	if (!bUseFlatBaseForFloorChecks)
	{
		bBlockingHit = GetWorld()->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, TraceChannel, CollisionShape, Params, ResponseParam);
	}
	else
	{
		// Test with a box that is enclosed by the capsule.
		const float CapsuleRadius = CollisionShape.GetCapsuleRadius();
		const float CapsuleHeight = CollisionShape.GetCapsuleHalfHeight();
		const FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(CapsuleRadius * 0.707f, CapsuleRadius * 0.707f, CapsuleHeight));

		// First test with the box rotated so the corners are along the major axes (ie rotated 45 degrees).
		bBlockingHit = GetWorld()->SweepSingleByChannel(OutHit, Start, End, FQuat(FVector(0.f, 0.f, -1.f), PI * 0.25f), TraceChannel, BoxShape, Params, ResponseParam);

		if (!bBlockingHit)
		{
			// Test again with the same box, not rotated.
			OutHit.Reset(1.f, false);
			bBlockingHit = GetWorld()->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, TraceChannel, BoxShape, Params, ResponseParam);
		}
	}

	return bBlockingHit;
}

// 6215
bool UUSBMovementComponent::ShouldCheckForValidLandingSpot(float DeltaTime, const FVector& Delta, const FHitResult& Hit) const
{
	// See if we hit an edge of a surface on the lower portion of the capsule.
	// In this case the normal will not equal the impact normal, and a downward sweep may find a walkable surface on top of the edge.
	if (Hit.Normal.Z > KINDA_SMALL_NUMBER && !Hit.Normal.Equals(Hit.ImpactNormal))
	{
		const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
		if (IsWithinEdgeTolerance(PawnLocation, Hit.ImpactPoint, GetMovementSphere()->GetScaledSphereRadius()))
		{
			return true;
		}
	}

	return false;
}

// 6232
float UUSBMovementComponent::GetPerchRadiusThreshold() const
{
	// Don't allow negative values.
	return FMath::Max(0.f, PerchRadiusThreshold);
}

// 6239
float UUSBMovementComponent::GetValidPerchRadius() const
{
	if (PawnOwner)
	{
		const float PawnRadius = GetMovementSphere()->GetScaledSphereRadius();
		return FMath::Clamp(PawnRadius - GetPerchRadiusThreshold(), 0.1f, PawnRadius);
	}
	return 0.f;
}

// 6250
bool UUSBMovementComponent::ShouldComputePerchResult(const FHitResult& InHit, bool bCheckRadius) const
{
	if (!InHit.IsValidBlockingHit())
	{
		return false;
	}

	// Don't try to perch if the edge radius is very small.
	if (GetPerchRadiusThreshold() <= SWEEP_EDGE_REJECT_DISTANCE)
	{
		return false;
	}

	if (bCheckRadius)
	{
		const float DistFromCenterSq = (InHit.ImpactPoint - InHit.Location).SizeSquared2D();
		const float StandOnEdgeRadius = GetValidPerchRadius();
		if (DistFromCenterSq <= FMath::Square(StandOnEdgeRadius))
		{
			// Already within perch radius.
			return false;
		}
	}

	return true;
}

// 6278
bool UUSBMovementComponent::ComputePerchResult(const float TestRadius, const FHitResult& InHit, const float InMaxFloorDist, FFindFloorResult& OutPerchFloorResult) const
{
	if (InMaxFloorDist <= 0.f)
	{
		return 0.f;
	}

	// Sweep further than actual requested distance, because a reduced capsule radius means we could miss some hits that the normal radius would contact.
	float PawnRadius, PawnHalfHeight;
	PawnRadius = PawnHalfHeight = GetMovementSphere()->GetScaledSphereRadius();

	const float InHitAboveBase = FMath::Max(0.f, InHit.ImpactPoint.Z - (InHit.Location.Z - PawnHalfHeight));
	const float PerchLineDist = FMath::Max(0.f, InMaxFloorDist - InHitAboveBase);
	const float PerchSweepDist = FMath::Max(0.f, InMaxFloorDist);

	const float ActualSweepDist = PerchSweepDist + PawnRadius;
	ComputeFloorDist(InHit.Location, PerchLineDist, ActualSweepDist, OutPerchFloorResult, TestRadius);

	if (!OutPerchFloorResult.IsWalkableFloor())
	{
		return false;
	}
	else if (InHitAboveBase + OutPerchFloorResult.FloorDist > InMaxFloorDist)
	{
		// Hit something past max distance
		OutPerchFloorResult.bWalkableFloor = false;
		return false;
	}

	return true;
}

// 6311
bool UUSBMovementComponent::CanStepUp(const FHitResult& Hit) const
{
	if (!Hit.IsValidBlockingHit() || !HasValidData() || MovementMode == MOVE_Falling)
	{
		return false;
	}

	// No component for "fake" hits when we are on a known good base.
	const UPrimitiveComponent* HitComponent = Hit.Component.Get();
	if (!HitComponent)
	{
		return true;
	}

	if (!HitComponent->CanCharacterStepUp(PawnOwner))
	{
		return false;
	}

	// No actor for "fake" hits when we are on a known good base.
	const AActor* HitActor = Hit.GetActor();
	if (!HitActor)
	{
		return true;
	}

	if (!HitActor->CanBeBaseForCharacter(PawnOwner))
	{
		return false;
	}

	return true;
}

// 6346
bool UUSBMovementComponent::StepUp(const FVector& GravDir, const FVector& Delta, const FHitResult& InHit, FStepDownResult* OutStepDownResult)
{
	SCOPE_CYCLE_COUNTER(STAT_CharStepUp);

	if (!CanStepUp(InHit) || MaxStepHeight <= 0.f)
	{
		return false;
	}

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	float PawnRadius, PawnHalfHeight;
	PawnRadius = PawnHalfHeight = GetMovementSphere()->GetScaledSphereRadius();

	// Don't bother stepping up if top of capsule is hitting something.
	const float InitialImpactZ = InHit.ImpactPoint.Z;
	if (InitialImpactZ > OldLocation.Z + (PawnHalfHeight - PawnRadius))
	{
		return false;
	}

	if (GravDir.IsZero())
	{
		return false;
	}

	// Gravity should be a normalized direction
	ensure(GravDir.IsNormalized());

	float StepTravelUpHeight = MaxStepHeight;
	float StepTravelDownHeight = StepTravelUpHeight;
	const float StepSideZ = -1.f * FVector::DotProduct(InHit.ImpactNormal, GravDir);
	float PawnInitialFloorBaseZ = OldLocation.Z - PawnHalfHeight;
	float PawnFloorPointZ = PawnInitialFloorBaseZ;

	if (IsMovingOnGround() && CurrentFloor.IsWalkableFloor())
	{
		// Since we float a variable amount off the floor, we need to enforce max step height off the actual point of impact with the floor.
		const float FloorDist = FMath::Max(0.f, CurrentFloor.GetDistanceToFloor());
		PawnInitialFloorBaseZ -= FloorDist;
		StepTravelUpHeight = FMath::Max(StepTravelUpHeight - FloorDist, 0.f);
		StepTravelDownHeight = (MaxStepHeight + MAX_FLOOR_DIST * 2.f);

		const bool bHitVerticalFace = !IsWithinEdgeTolerance(InHit.Location, InHit.ImpactPoint, PawnRadius);
		if (!CurrentFloor.bLineTrace && !bHitVerticalFace)
		{
			PawnFloorPointZ = CurrentFloor.HitResult.ImpactPoint.Z;
		}
		else
		{
			// Base floor point is the base of the capsule moved down by how far we are hovering over the surface we are hitting.
			PawnFloorPointZ -= CurrentFloor.FloorDist;
		}
	}

	// Don't step up if the impact is below us, accounting for distance from floor.
	if (InitialImpactZ <= PawnInitialFloorBaseZ)
	{
		return false;
	}

	// Scope our movement updates, and do not apply them until all intermediate moves are completed.
	FScopedMovementUpdate ScopedStepUpMovement(UpdatedComponent, EScopedUpdate::DeferredUpdates);

	// step up - treat as vertical wall
	FHitResult SweepUpHit(1.f);
	const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
	MoveUpdatedComponent(-GravDir * StepTravelUpHeight, PawnRotation, true, &SweepUpHit);

	if (SweepUpHit.bStartPenetrating)
	{
		// Undo movement
		ScopedStepUpMovement.RevertMove();
		return false;
	}

	// step fwd
	FHitResult Hit(1.f);
	MoveUpdatedComponent(Delta, PawnRotation, true, &Hit);

	// Check result of forward movement
	if (Hit.bBlockingHit)
	{
		if (Hit.bStartPenetrating)
		{
			// Undo movement
			ScopedStepUpMovement.RevertMove();
			return false;
		}

		// If we hit something above us and also something ahead of us, we should notify about the upward hit as well.
		// The forward hit will be handled later (in the bSteppedOver case below).
		// In the case of hitting something above but not forward, we are not blocked from moving so we don't need the notification.
		if (SweepUpHit.bBlockingHit && Hit.bBlockingHit)
		{
			HandleImpact(SweepUpHit);
		}

		// pawn ran into a wall
		HandleImpact(Hit);
		if (IsFalling())
		{
			return true;
		}

		// adjust and try again
		const float ForwardHitTime = Hit.Time;
		const float ForwardSlideAmount = SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);

		if (IsFalling())
		{
			ScopedStepUpMovement.RevertMove();
			return false;
		}

		// If both the forward hit and the deflection got us nowhere, there is no point in this step up.
		if (ForwardHitTime == 0.f && ForwardSlideAmount == 0.f)
		{
			ScopedStepUpMovement.RevertMove();
			return false;
		}
	}

	// Step down
	MoveUpdatedComponent(GravDir * StepTravelDownHeight, UpdatedComponent->GetComponentQuat(), true, &Hit);

	// If step down was initially penetrating abort the step up
	if (Hit.bStartPenetrating)
	{
		ScopedStepUpMovement.RevertMove();
		return false;
	}

	FStepDownResult StepDownResult;
	if (Hit.IsValidBlockingHit())
	{
		// See if this step sequence would have allowed us to travel higher than our max step height allows.
		const float DeltaZ = Hit.ImpactPoint.Z - PawnFloorPointZ;
		if (DeltaZ > MaxStepHeight)
		{
			//UE_LOG(LogCharacterMovement, VeryVerbose, TEXT("- Reject StepUp (too high Height %.3f) up from floor base %f to %f"), DeltaZ, PawnInitialFloorBaseZ, NewLocation.Z);
			ScopedStepUpMovement.RevertMove();
			return false;
		}

		// Reject unwalkable surface normals here.
		if (!IsWalkable(Hit))
		{
			// Reject if normal opposes movement direction
			const bool bNormalTowardsMe = (Delta | Hit.ImpactNormal) < 0.f;
			if (bNormalTowardsMe)
			{
				//UE_LOG(LogCharacterMovement, VeryVerbose, TEXT("- Reject StepUp (unwalkable normal %s opposed to movement)"), *Hit.ImpactNormal.ToString());
				ScopedStepUpMovement.RevertMove();
				return false;
			}

			// Also reject if we would end up being higher than our starting location by stepping down.
			// It's fine to step down onto an unwalkable normal below us, we will just slide off. Rejecting those moves would prevent us from being able to walk off the edge.
			if (Hit.Location.Z > OldLocation.Z)
			{
				//UE_LOG(LogCharacterMovement, VeryVerbose, TEXT("- Reject StepUp (unwalkable normal %s above old position)"), *Hit.ImpactNormal.ToString());
				ScopedStepUpMovement.RevertMove();
				return false;
			}
		}

		// Reject moves where the downward sweep hit something very close to the edge of the capsule. This maintains consistency with FindFloor as well.
		if (!IsWithinEdgeTolerance(Hit.Location, Hit.ImpactPoint, PawnRadius))
		{
			//UE_LOG(LogCharacterMovement, VeryVerbose, TEXT("- Reject StepUp (outside edge tolerance)"));
			ScopedStepUpMovement.RevertMove();
			return false;
		}

		// Don't step up onto invalid surfaces if traveling higher.
		if (DeltaZ > 0.f && !CanStepUp(Hit))
		{
			//UE_LOG(LogCharacterMovement, VeryVerbose, TEXT("- Reject StepUp (up onto surface with !CanStepUp())"));
			ScopedStepUpMovement.RevertMove();
			return false;
		}

		// See if we can validate the floor as a result of this step down. In almost all cases this should succeed, and we can avoid computing the floor outside this method.
		if (OutStepDownResult != NULL)
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), StepDownResult.FloorResult, false, &Hit);

			// Reject unwalkable normals if we end up higher than our initial height.
			// It's fine to walk down onto an unwalkable surface, don't reject those moves.
			if (Hit.Location.Z > OldLocation.Z)
			{
				// We should reject the floor result if we are trying to step up an actual step where we are not able to perch (this is rare).
				// In those cases we should instead abort the step up and try to slide along the stair.
				if (!StepDownResult.FloorResult.bBlockingHit && StepSideZ < MAX_STEP_SIDE_Z)
				{
					ScopedStepUpMovement.RevertMove();
					return false;
				}
			}

			StepDownResult.bComputedFloor = true;
		}
	}

	// Copy step down result.
	if (OutStepDownResult != NULL)
	{
		*OutStepDownResult = StepDownResult;
	}

	// Don't recalculate velocity based on this height adjustment, if considering vertical adjustments.
	bJustTeleported |= !bMaintainHorizontalGroundVelocity;

	return true;
}

// 6562
void UUSBMovementComponent::HandleImpact(const FHitResult& Impact, float TimeSlice, const FVector& MoveDelta)
{
	SCOPE_CYCLE_COUNTER(STAT_CharHandleImpact);

	/*
	if (CharacterOwner)
	{
		CharacterOwner->MoveBlockedBy(Impact);		// MoveBlockeBy 무엇에 Block 되었는지 알려주기만 하는 것 같음. Character에 정의가 안보이넹
	}

	IPathFollowingAgentInterface* PFAgent = GetPathFollowingAgent();
	if (PFAgent)
	{
		// Also notify path following!
		PFAgent->OnMoveBlockedBy(Impact);
	}
	*/

	APawn* OtherPawn = Cast<APawn>(Impact.GetActor());
	if (OtherPawn)
	{
		NotifyBumpedPawn(OtherPawn);
	}

	if (bEnablePhysicsInteraction)
	{
		const FVector ForceAccel = Acceleration + (IsFalling() ? FVector(0.f, 0.f, GetGravityZ()) : FVector::ZeroVector);
		ApplyImpactPhysicsForces(Impact, ForceAccel, Velocity);
	}
}

void UUSBMovementComponent::ApplyImpactPhysicsForces(const FHitResult& Impact, const FVector& ImpactAcceleration, const FVector& ImpactVelocity)
{
	if (bEnablePhysicsInteraction && Impact.bBlockingHit)
	{
		if (UPrimitiveComponent* ImpactComponent = Impact.GetComponent())
		{
			FBodyInstance* BI = ImpactComponent->GetBodyInstance(Impact.BoneName);
			if (BI != nullptr && BI->IsInstanceSimulatingPhysics())
			{
				FVector ForcePoint = Impact.ImpactPoint;

				const float BodyMass = FMath::Max(BI->GetBodyMass(), 1.0f);

				if (bPushForceUsingZOffset)
				{
					FBox Bounds = BI->GetBodyBounds();

					FVector Center, Extents;
					Bounds.GetCenterAndExtents(Center, Extents);

					if (!Extents.IsNearlyZero())
					{
						ForcePoint.Z = Center.Z + Extents.Z * PushForcePointZOffsetFactor;
					}
				}

				FVector Force = Impact.ImpactNormal * -1.0f;

				float PushForceModificator = 1.0f;

				const FVector ComponentVelocity = ImpactComponent->GetPhysicsLinearVelocity();
				const FVector VirtualVelocity = ImpactAcceleration.IsZero() ? ImpactVelocity : CustomGetSafeNormal(ImpactAcceleration) * GetMaxSpeed();

				float Dot = 0.0f;

				if (bScalePushForceToVelocity && !ComponentVelocity.IsNearlyZero())
				{
					Dot = ComponentVelocity | VirtualVelocity;

					if (Dot > 0.0f && Dot < 1.0f)
					{
						PushForceModificator *= Dot;
					}
				}

				if (bPushForceScaledToMass)
				{
					PushForceModificator *= BodyMass;
				}

				Force *= PushForceModificator;

				if (ComponentVelocity.IsNearlyZero())
				{
					Force *= InitialPushForceFactor;
					ImpactComponent->AddImpulseAtLocation(Force, ForcePoint, Impact.BoneName);
				}
				else
				{
					Force *= PushForceFactor;
					ImpactComponent->AddForceAtLocation(Force, ForcePoint, Impact.BoneName);
				}
			}
		}
	}
}

// 6821
FVector UUSBMovementComponent::ConstrainInputAcceleration(const FVector& InputAcceleration) const
{
	// walking or falling pawns ignore up/down sliding
	if (InputAcceleration.Z != 0.f && (IsMovingOnGround() || IsFalling()))
	{
		return FVector(InputAcceleration.X, InputAcceleration.Y, 0.f);
	}

	return InputAcceleration;
}




// 6833
FVector UUSBMovementComponent::ScaleInputAcceleration(const FVector& InputAcceleration) const
{
	return GetMaxAcceleration() * CustomGetClampedToMaxSize(InputAcceleration, 1.f);
}

// 6839
FVector UUSBMovementComponent::RoundAcceleration(FVector InAccel) const
{
	// Match FVector_NetQuantize10 (1 decimal place of precision).
	InAccel.X = FMath::RoundToFloat(InAccel.X * 10.f) / 10.f;
	InAccel.Y = FMath::RoundToFloat(InAccel.Y * 10.f) / 10.f;
	InAccel.Z = FMath::RoundToFloat(InAccel.Z * 10.f) / 10.f;
	return InAccel;
}

// 6849
float UUSBMovementComponent::ComputeAnalogInputModifier() const
{
	const float MaxAccel = GetMaxAcceleration();
	if (Acceleration.SizeSquared() > 0.f && MaxAccel > SMALL_NUMBER)
	{
		return FMath::Clamp(Acceleration.Size() / MaxAccel, 0.f, 1.f);
	}

	return 0.f;
}

// 6860
float UUSBMovementComponent::GetAnalogInputModifier() const
{
	return AnalogInputModifier;
}

// 6855
float UUSBMovementComponent::GetSimulationTimeStep(float RemainingTime, int32 Iterations) const
{
	static uint32 s_WarningCount = 0;
	if (RemainingTime > MaxSimulationTimeStep)
	{
		if (Iterations < MaxSimulationIterations)
		{
			// Subdivide moves to be no longer than MaxSimulationTimeStep seconds
			RemainingTime = FMath::Min(MaxSimulationTimeStep, RemainingTime * 0.5f);
		}
		else
		{
			// If this is the last iteration, just use all the remaining time. This is usually better than cutting things short, as the simulation won't move far enough otherwise.
			// Print a throttled warning.
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			if ((s_WarningCount++ < 100) || (GFrameCounter & 15) == 0)
			{
				//UE_LOG(LogCharacterMovement, Warning, TEXT("GetSimulationTimeStep() - Max iterations %d hit while remaining time %.6f > MaxSimulationTimeStep (%.3f) for '%s', movement '%s'"), MaxSimulationIterations, RemainingTime, MaxSimulationTimeStep, *GetNameSafe(CharacterOwner), *GetMovementName());
			}
#endif
		}
	}

	// no less than MIN_TICK_TIME (to avoid potential divide-by-zero during simulation).
	return FMath::Max(MIN_TICK_TIME, RemainingTime);
}

// 8843
void UUSBMovementComponent::UpdateFloorFromAdjustment()
{
	if (!HasValidData())
	{
		return;
	}

	// If walking, try to update the cached floor so it is current. This is necessary for UpdateBasedMovement() and MoveAlongFloor() to work properly.
	// If base is now NULL, presumably we are no longer walking. If we had a valid floor but don't find one now, we'll likely start falling.
	// Walking 상태에서 현재 Floor를 갱신한다. UpdateBasedMovement()와 MoveAlongFloor() 메소드가 정확하게 동작하기 위해선 필수적으로 호출되어야 한다.
	if (GetMovementBase())
	{
		// 만약 CurrentFloor가 유효하더라도 다음 바닥을 찾지 못한다면 Falling이 시작된다.
		FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, false);
	}
	else
	{
		// 만약 Base가 없다면 Walking할 수 없다. 
		CurrentFloor.Clear();
	}
}

// 9357
void UUSBMovementComponent::CapsuleTouched(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bEnablePhysicsInteraction)
	{
		return;
	}

	if (OtherComp != NULL && OtherComp->IsAnySimulatingPhysics())
	{
		const FVector OtherLoc = OtherComp->GetComponentLocation();
		const FVector Loc = UpdatedComponent->GetComponentLocation();
		FVector ImpulseDir = CustomGetSafeNormal(FVector(OtherLoc.X - Loc.X, OtherLoc.Y - Loc.Y, 0.25f));
		ImpulseDir = (ImpulseDir + CustomGetSafeNormal2D(Velocity)) * 0.5f;
		ImpulseDir.Normalize();

		FName BoneName = NAME_None;
		if (OtherBodyIndex != INDEX_NONE)
		{
			BoneName = ((USkinnedMeshComponent*)OtherComp)->GetBoneName(OtherBodyIndex);
		}

		float TouchForceFactorModified = TouchForceFactor;

		if (bTouchForceScaledToMass)
		{
			FBodyInstance* BI = OtherComp->GetBodyInstance(BoneName);
			TouchForceFactorModified *= BI ? BI->GetBodyMass() : 1.0f;
		}

		float ImpulseStrength = FMath::Clamp(Velocity.Size2D() * TouchForceFactorModified,
			MinTouchForce > 0.0f ? MinTouchForce : -FLT_MAX,
			MaxTouchForce > 0.0f ? MaxTouchForce : FLT_MAX);

		FVector Impulse = ImpulseDir * ImpulseStrength;

		OtherComp->AddImpulse(Impulse, BoneName);
	}
}

// 9449
void UUSBMovementComponent::ApplyDownwardForce(float DeltaSeconds)
{
	if (StandingDownwardForceScale != 0.0f && CurrentFloor.HitResult.IsValidBlockingHit())
	{
		UPrimitiveComponent* BaseComp = CurrentFloor.HitResult.GetComponent();
		const FVector Gravity = FVector(0.0f, 0.0f, GetGravityZ());

		if (BaseComp && BaseComp->IsAnySimulatingPhysics() && !Gravity.IsZero())
		{
			BaseComp->AddForceAtLocation(Gravity * Mass * StandingDownwardForceScale, CurrentFloor.HitResult.ImpactPoint, CurrentFloor.HitResult.BoneName);
		}
	}
}


// 9463
void UUSBMovementComponent::ApplyRepulsionForce(float DeltaSeconds)
{
	if (UpdatedPrimitive && RepulsionForce > 0.0f && PawnOwner != nullptr)
	{
		const TArray<FOverlapInfo>& Overlaps = UpdatedPrimitive->GetOverlapInfos();
		if (Overlaps.Num() > 0)
		{
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CMC_ApplyRepulsionForce));
			QueryParams.bReturnFaceIndex = false;
			QueryParams.bReturnPhysicalMaterial = false;

			float CapsuleRadius = 0.f;
			float CapsuleHalfHeight = 0.f;
			CapsuleRadius = CapsuleHalfHeight = GetMovementSphere()->GetScaledSphereRadius();
			const float RepulsionForceRadius = CapsuleRadius * 1.2f;
			const float StopBodyDistance = 2.5f;
			const FVector MyLocation = UpdatedPrimitive->GetComponentLocation();

			for (int32 i = 0; i < Overlaps.Num(); i++)
			{
				const FOverlapInfo& Overlap = Overlaps[i];

				UPrimitiveComponent* OverlapComp = Overlap.OverlapInfo.Component.Get();
				if (!OverlapComp || OverlapComp->Mobility < EComponentMobility::Movable)
				{
					continue;
				}

				// Use the body instead of the component for cases where we have multi-body overlaps enabled
				FBodyInstance* OverlapBody = nullptr;
				const int32 OverlapBodyIndex = Overlap.GetBodyIndex();
				const USkeletalMeshComponent* SkelMeshForBody = (OverlapBodyIndex != INDEX_NONE) ? Cast<USkeletalMeshComponent>(OverlapComp) : nullptr;
				if (SkelMeshForBody != nullptr)
				{
					OverlapBody = SkelMeshForBody->Bodies.IsValidIndex(OverlapBodyIndex) ? SkelMeshForBody->Bodies[OverlapBodyIndex] : nullptr;
				}
				else
				{
					OverlapBody = OverlapComp->GetBodyInstance();
				}

				if (!OverlapBody)
				{
					//UE_LOG(LogCharacterMovement, Warning, TEXT("%s could not find overlap body for body index %d"), *GetName(), OverlapBodyIndex);
					continue;
				}

				if (!OverlapBody->IsInstanceSimulatingPhysics())
				{
					continue;
				}

				FTransform BodyTransform = OverlapBody->GetUnrealWorldTransform();

				FVector BodyVelocity = OverlapBody->GetUnrealWorldVelocity();
				FVector BodyLocation = BodyTransform.GetLocation();

				// Trace to get the hit location on the capsule
				FHitResult Hit;
				bool bHasHit = UpdatedPrimitive->LineTraceComponent(Hit, BodyLocation,
					FVector(MyLocation.X, MyLocation.Y, BodyLocation.Z),
					QueryParams);

				FVector HitLoc = Hit.ImpactPoint;
				bool bIsPenetrating = Hit.bStartPenetrating || Hit.PenetrationDepth > StopBodyDistance;

				// If we didn't hit the capsule, we're inside the capsule
				if (!bHasHit)
				{
					HitLoc = BodyLocation;
					bIsPenetrating = true;
				}

				const float DistanceNow = (HitLoc - BodyLocation).SizeSquared2D();
				const float DistanceLater = (HitLoc - (BodyLocation + BodyVelocity * DeltaSeconds)).SizeSquared2D();

				if (bHasHit && DistanceNow < StopBodyDistance && !bIsPenetrating)
				{
					OverlapBody->SetLinearVelocity(FVector(0.0f, 0.0f, 0.0f), false);
				}
				else if (DistanceLater <= DistanceNow || bIsPenetrating)
				{
					FVector ForceCenter = MyLocation;

					if (bHasHit)
					{
						ForceCenter.Z = HitLoc.Z;
					}
					else
					{
						ForceCenter.Z = FMath::Clamp(BodyLocation.Z, MyLocation.Z - CapsuleHalfHeight, MyLocation.Z + CapsuleHalfHeight);
					}

					OverlapBody->AddRadialForceToBody(ForceCenter, RepulsionForceRadius, RepulsionForce * Mass, ERadialImpulseFalloff::RIF_Constant);
				}
			}
		}
	}
}

// 9563
void UUSBMovementComponent::ApplyAccumulatedForces(float DeltaSeconds)
{
	// 사용자 입력과 별도로 주어진 힘을 Velocity에 적용함. ex) AddForces
	if (PendingImpulseToApply.Z != 0.f || PendingForceToApply.Z != 0.f)
	{
		// check to see if applied momentum is enough to overcome gravity
		if (IsMovingOnGround() && (PendingImpulseToApply.Z + (PendingForceToApply.Z * DeltaSeconds) + (GetGravityZ() * DeltaSeconds) > SMALL_NUMBER))
		{
			SetMovementMode(MOVE_Falling);
		}
	}

	Velocity += PendingImpulseToApply + (PendingForceToApply * DeltaSeconds);

	// Don't call ClearAccumulatedForces() because it could affect launch velocity
	PendingImpulseToApply = FVector::ZeroVector;
	PendingForceToApply = FVector::ZeroVector;
}

// 9581
void UUSBMovementComponent::ClearAccumulatedForces()
{
	PendingImpulseToApply = FVector::ZeroVector;
	PendingForceToApply = FVector::ZeroVector;
	PendingLaunchVelocity = FVector::ZeroVector;
}

// 9632
void UUSBMovementComponent::RegisterComponentTickFunctions(bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);

	if (bRegister)
	{
		if (SetupActorComponentTickFunction(&PostPhysicsTickFunction))
		{
			PostPhysicsTickFunction.Target = this;
			PostPhysicsTickFunction.AddPrerequisite(this, this->PrimaryComponentTick);
		}
	}
	else
	{
		if (PostPhysicsTickFunction.IsTickFunctionRegistered())
		{
			PostPhysicsTickFunction.UnRegisterTickFunction();
		}
	}
}

// 10900
ETeleportType UUSBMovementComponent::GetTeleportType() const
{
	return bJustTeleported ? ETeleportType::TeleportPhysics : ETeleportType::None;
}


























/******************************
* Character.cpp 점프 관련 요소
 ******************************/

void UUSBMovementComponent::Jump()
{
	bPressedJump = true;
	JumpKeyHoldTime = 0.0f;
}

void UUSBMovementComponent::StopJumping()
{
	bPressedJump = false;
	ResetJumpState();
}

void UUSBMovementComponent::CheckJumpInput(float DeltaTime)
{
	if (bPressedJump)
	{
		// If this is the first jump and we're already falling,
		// then increment the JumpCount to compensate.
		const bool bFirstJump = JumpCurrentCount == 0;
		if (bFirstJump && IsFalling())
		{
			JumpCurrentCount++;
		}
		const bool bDidJump = CanJump() && DoJump();
		if (bDidJump)
		{
			// Transition from not (actively) jumping to jumping.
			if (!bWasJumping)
			{
				JumpCurrentCount++;
				JumpForceTimeRemaining = GetJumpMaxHoldTime();
				//OnJumped();
			}
		}
		bWasJumping = bDidJump;
	}
}

void UUSBMovementComponent::ClearJumpInput(float DeltaTime)
{
	if (bPressedJump)
	{
		JumpKeyHoldTime += DeltaTime;

		// Don't disable bPressedJump right away if it's still held.
		// Don't modify JumpForceTimeRemaining because a frame of update may be remaining.
		if (JumpKeyHoldTime >= GetJumpMaxHoldTime())
		{
			bPressedJump = false;
		}
	}
	else
	{
		JumpForceTimeRemaining = 0.0f;
		bWasJumping = false;
	}
}

float UUSBMovementComponent::GetJumpMaxHoldTime() const
{
	return JumpMaxHoldTime;
}

bool UUSBMovementComponent::CanJump() const
{
	return CanJumpInternal();
}

bool UUSBMovementComponent::CanJumpInternal_Implementation() const
{
	// Ensure the character isn't currently crouched.
	bool bCanJump = true;// = !bIsCrouched;

	// Ensure that the CharacterMovement state is valid
	bCanJump &= IsJumpAllowed() &&
		// Can only jump from the ground, or multi-jump if already falling.
		(IsMovingOnGround() || IsFalling());

	if (bCanJump)
	{
		// Ensure JumpHoldTime and JumpCount are valid.
		if (!bWasJumping || GetJumpMaxHoldTime() <= 0.0f)
		{
			//UE_LOG(LogTemp, Log, TEXT("Before Process : %s"), bCanJump ? TEXT("True") : TEXT("False"));
			if (JumpCurrentCount == 0 && IsFalling())
			{
				bCanJump = JumpCurrentCount + 1 < JumpMaxCount;
			}
			else
			{
				bCanJump = JumpCurrentCount < JumpMaxCount;
			}
		}
		else
		{
			// Only consider JumpKeyHoldTime as long as:
			// A) The jump limit hasn't been met OR
			// B) The jump limit has been met AND we were already jumping
			const bool bJumpKeyHeld = (bPressedJump && JumpKeyHoldTime < GetJumpMaxHoldTime());
			bCanJump = bJumpKeyHeld &&
				((JumpCurrentCount < JumpMaxCount) || (bWasJumping && JumpCurrentCount == JumpMaxCount));
		}
	}

	return bCanJump;
}

void UUSBMovementComponent::ResetJumpState()
{
	bPressedJump = false;
	bWasJumping = false;
	JumpKeyHoldTime = 0.0f;
	JumpForceTimeRemaining = 0.0f;

	if (!IsFalling())
	{
		JumpCurrentCount = 0;
	}
}

bool UUSBMovementComponent::IsJumpProvidingForce() const
{
	if (JumpForceTimeRemaining > 0.0f)
	{
		return true;
	}

	return false;
}

/******************************
* Character.cpp Base 관련 요소
 ******************************/
void UUSBMovementComponent::SetBase(UPrimitiveComponent* NewBaseComponent, const FName InBoneName, bool bNotifyPawn) {
	// If NewBaseComponent is nullptr, ignore bone name.
	const FName BoneName = (NewBaseComponent ? InBoneName : NAME_None);

	// See what changed.
	const bool bBaseChanged = (NewBaseComponent != BasedMovement.MovementBase);
	const bool bBoneChanged = (BoneName != BasedMovement.BoneName);

	if (bBaseChanged || bBoneChanged)
	{
		// Verify no recursion.
		APawn* Loop = (NewBaseComponent ? Cast<APawn>(NewBaseComponent->GetOwner()) : nullptr);
		while (Loop)
		{
			if (Loop == PawnOwner)
			{
				//UE_LOG(LogCharacter, Warning, TEXT(" SetBase failed! Recursion detected. Pawn %s already based on %s."), *GetName(), *NewBaseComponent->GetName()); //-V595
				return;
			}
			if (UPrimitiveComponent* LoopBase = Loop->GetMovementBase())
			{
				Loop = Cast<APawn>(LoopBase->GetOwner());
			}
			else
			{
				break;
			}
		}

		// Set base.
		UPrimitiveComponent* OldBase = BasedMovement.MovementBase;
		BasedMovement.MovementBase = NewBaseComponent;
		BasedMovement.BoneName = BoneName;


		const bool bBaseIsSimulating = NewBaseComponent && NewBaseComponent->IsSimulatingPhysics();
		if (bBaseChanged)
		{
			MovementBaseUtility::RemoveTickDependency(PrimaryComponentTick, OldBase);
			// We use a special post physics function if simulating, otherwise add normal tick prereqs.
			if (!bBaseIsSimulating)
			{
				MovementBaseUtility::AddTickDependency(PrimaryComponentTick, NewBaseComponent);
			}
		}

		if (NewBaseComponent)
		{
			// Update OldBaseLocation/Rotation as those were referring to a different base
			// ... but not when handling replication for proxies (since they are going to copy this data from the replicated values anyway)
			if (!bInBaseReplication)
			{
				// Force base location and relative position to be computed since we have a new base or bone so the old relative offset is meaningless.
				SaveBaseLocation();
			}

			// Enable PostPhysics tick if we are standing on a physics object, as we need to to use post-physics transforms
			PostPhysicsTickFunction.SetTickFunctionEnable(bBaseIsSimulating);
		}
		else
		{
			BasedMovement.BoneName = NAME_None; // None, regardless of whether user tried to set a bone name, since we have no base component.
			BasedMovement.bRelativeRotation = false;
			CurrentFloor.Clear();
			PostPhysicsTickFunction.SetTickFunctionEnable(false);
		}

		if (PawnOwner->Role == ROLE_Authority || PawnOwner->Role == ROLE_AutonomousProxy)
		{
			BasedMovement.bServerHasBaseComponent = (BasedMovement.MovementBase != nullptr); // Also set on proxies for nicer debugging.
			//UE_LOG(LogCharacter, Verbose, TEXT("Setting base on %s for '%s' to '%s'"), Role == ROLE_Authority ? TEXT("Server") : TEXT("AutoProxy"), *GetName(), *GetFullNameSafe(NewBaseComponent));
		}
		else
		{
			//UE_LOG(LogCharacter, Verbose, TEXT("Setting base on Client for '%s' to '%s'"), *GetName(), *GetFullNameSafe(NewBaseComponent));
		}

		// Notify this actor of his new floor.
		if (bNotifyPawn)
		{
			BaseChange();
		}
	}
}

void UUSBMovementComponent::BaseChange()
{
	if (MovementMode != MOVE_None)
	{
		AActor* ActualMovementBase = PawnOwner->GetMovementBaseActor(PawnOwner);
		if ((ActualMovementBase != nullptr) && !ActualMovementBase->CanBeBaseForCharacter(PawnOwner))
		{
			JumpOff(ActualMovementBase);
		}
	}
}

void UUSBMovementComponent::SaveRelativeBasedMovement(const FVector& NewRelativeLocation, const FRotator& NewRotation, bool bRelativeRotation)
{
	BasedMovement.Location = NewRelativeLocation;
	BasedMovement.Rotation = NewRotation;
	BasedMovement.bRelativeRotation = bRelativeRotation;
}