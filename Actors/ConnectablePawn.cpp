#include "ConnectablePawn.h"
#include "Kismet/GameplayStatics.h"
#include "Datas/ConnectablePawnData.h"
#include "UObjects/ConnectionBehavior.h"
#include "ConstructorHelpers.h"
#include "Datas/USB_Macros.h"
#include "Managers/USB_GameManager.h"
#include "Components/PhysicsMovement.h"
#include "Perception/PawnSensingComponent.h"
#include "NavigationPath.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"

#include "AIController.h"
#include "UObjects/IdleBehavior.h"
#include "UObjects/SawPlayerBehavior.h"
#include "UObjects/ReturnBehavior.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/USB_PlayerPawn.h"
#include "UObjects/Connectable.h"

// Sets default values
AConnectablePawn::AConnectablePawn()
{
	AutoPossessAI = EAutoPossessAI::Spawned;
	m_CurrentState = EFSM::Idle;

	PrimaryActorTick.bCanEverTick = true;
	m_PawnID = NAME_None;
	
	m_MeshMainBody = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh00");
	RootComponent = m_MeshMainBody;
	m_MeshMainBody->SetCollisionProfileName("PhysicsActor");
	m_MeshMainBody->SetSimulatePhysics(false);
	m_MeshMainBody->SetUseCCD(false);
	m_MeshMainBody->CastShadow = false;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundMesh(TEXT("SkeletalMesh'/Game/Meshes/Characters/Player_USB/SK_USB_Head.SK_USB_Head'"));

	if (FoundMesh.Succeeded())
		m_MeshMainBody->SetSkeletalMesh(FoundMesh.Object);
	else
		check(FoundMesh.Object);
//
	m_MeshPort = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh01");
	m_MeshPort->SetupAttachment(RootComponent);
	m_MeshPort->CastShadow = false;
	m_MeshPort->SetGenerateOverlapEvents(true);
	m_MeshPort->SetCollisionProfileName("Port");
	//
	m_PhysicsCons = CreateDefaultSubobject<UPhysicsConstraintComponent>("PhyCon02");
	m_PhysicsCons->SetupAttachment(RootComponent);
	m_PhysicsCons->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhysicsCons->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhysicsCons->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.f);
	m_PhysicsCons->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhysicsCons->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhysicsCons->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
	m_PhysicsCons->SetDisableCollision(true);
	//
	m_Sphere = CreateDefaultSubobject<USphereComponent>("Sphere03");
	m_Sphere->SetupAttachment(RootComponent);
	m_Sphere->SetSphereRadius(300.f);
	m_Sphere->SetCollisionProfileName("USBOverlap");
	//
	m_Audio = CreateDefaultSubobject<UAudioComponent>("Audio04");
	m_Audio->SetupAttachment(RootComponent);
	m_Audio->SetAutoActivate(false);
	//
	static ConstructorHelpers::FObjectFinder<USoundBase> FoundSound(TEXT("SoundWave'/Game/SFX/USB/Put_in/SE_SFX_BUTTON_PUSH.SE_SFX_BUTTON_PUSH'"));

	if (FoundSound.Succeeded())
		m_Audio->SetSound(FoundSound.Object);
	else
		check(FoundSound.Object);


	m_AryStateFunction[(int)EFSM::Idle] = &AConnectablePawn::OnIdle;
	m_AryStateFunction[(int)EFSM::Detect] = &AConnectablePawn::OnDetectPlayer;
	m_AryStateFunction[(int)EFSM::Return] = &AConnectablePawn::OnReturn;
}

void AConnectablePawn::BeginPlay()
{
	Super::BeginPlay();
	
	if (m_PawnID != NAME_None)
	{
		SetConnectPawn(m_PawnID);
		//SetActorTickEnabled(true);
	}
}

void AConnectablePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	m_fDeltaTime = DeltaTime;

	if (!GetNav())
	{
		SetActorTickEnabled(false);
		return;
	}

	CountingTimer();
	ExecuteFSM();

}

AUSB_PlayerPawn * AConnectablePawn::GetPlayer() const
{
	return Cast<AUSB_PlayerPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}


void AConnectablePawn::SetUpSceneComponent(USceneComponent * compo, USceneComponent * parent, FTransform trans)
{
	compo->SetupAttachment(parent);
	SetUpActorComponent(compo);
}

void AConnectablePawn::SetUpActorComponent(UActorComponent * compo)
{
	AddInstanceComponent(compo);
	compo->RegisterComponent();
}

void AConnectablePawn::CountingTimer()
{
}

void AConnectablePawn::ExecuteFSM()
{
	if (!m_bUseFSM)
	{
		return;
	}

	(this->*m_AryStateFunction[(int)m_CurrentState])();
}

void AConnectablePawn::OnIdle()
{
	m_IdleBehavior->Execute(this,m_fDeltaTime);
}

void AConnectablePawn::OnDetectPlayer()
{
	m_DetectBehavior->Execute(this, m_fDeltaTime);
}

void AConnectablePawn::OnReturn()
{
	m_ReturnToPosBehavior->Execute(this, m_fDeltaTime);
}

void AConnectablePawn::OnSeePlayer(APawn * player)
{
	m_FoundPlayerPawn = player;
}

AAIController * AConnectablePawn::GetAICon()
{
	return m_AiController;
}

UNavigationSystemV1 * AConnectablePawn::GetNav()
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	if (!NavSys)
	{
		PRINTF("NoNav");
		return nullptr;
	}
	return NavSys;
}

void AConnectablePawn::OnConnected(IConnectable * portTarget)
{
	m_ConnectionBehav->ExecuteConnection(GetPlayer(),this);
}

void AConnectablePawn::OnDisconnected(IConnectable * pinTarget)
{

}

void AConnectablePawn::SetConnectPawn(FName pawnID)
{
	auto* USBManager = GetGameInstance<UUSB_GameManager>();
	const auto& PawnData= USBManager->GetConnectPawnData(pawnID);

	if (!&PawnData)
	{
		PRINTF("Failed Find Data - PawnConnect");
		return;
	}

	m_PinType = PawnData.m_PinType;
	m_PortType = PawnData.m_PortType;
	m_PawnID = PawnData.m_NameID;
	m_PawnName = PawnData.m_ShowingName;

	m_MeshMainBody->SetSkeletalMesh(PawnData.m_MeshPawnMainBody);
	m_MeshPort->SetSkeletalMesh(PawnData.m_MeshPortBody);

	m_MeshPort->SetRelativeLocation(PawnData.m_PortRelativeLoc);
	m_MeshPort->SetRelativeRotation(PawnData.m_PortRelativeRot);

	m_Sphere->SetSphereRadius(PawnData.m_fInteractRadius);

	m_StartLocation = GetActorLocation();

	m_MeshMainBody->SetSimulatePhysics(true);
	//m_MeshMainBody->GetBodyInstance()->bLockYRotation = true;
	//m_MeshMainBody->GetBodyInstance()->bLockXRotation = true;
	//
	if (PawnData.m_ConnectBehav != nullptr)
	{
		m_ConnectionBehav = NewObject<UConnectionBehavior>( PawnData.m_ConnectBehav->GetDefaultObject(), PawnData.m_ConnectBehav);
	}
	//배터리가 여기 포함,그밖에 주는것들

	m_AiController = Cast<AAIController>(GetController());

	if (!PawnData.m_bIsAI)
	{
		return;
	}

	m_bUseFSM = true;

	//
	m_Movement = AddActorComponent<UPhysicsMovement>(UPhysicsMovement::StaticClass());
	m_Movement->m_fMovingForce = PawnData.m_fMovingForce;
	m_Movement->m_fMaxSpeed = PawnData.m_fMaxSpeed;
	m_Movement->m_fMaxBrakingDeceleration = PawnData.m_fMaxBrakingDeceleration;
	m_Movement->m_nJumpMaxCount = PawnData.m_nJumpMaxCount;
	m_Movement->m_fJumpHeight = PawnData.m_fJumpHeight;
	m_Movement->SetMovingComponent(m_MeshMainBody, false);

	m_PawnSensing = AddActorComponent<UPawnSensingComponent>(UPawnSensingComponent::StaticClass());
	m_PawnSensing->OnSeePawn.AddDynamic(this, &AConnectablePawn::OnSeePlayer);

	m_PawnSensing->HearingThreshold = PawnData.m_fHearingThreshold;
	m_PawnSensing->LOSHearingThreshold = PawnData.m_fLOSHearingThreshold;
	m_PawnSensing->SightRadius = PawnData.m_fSightRadius;
	m_PawnSensing->SetPeripheralVisionAngle(PawnData.m_fAngle);	
	//
	m_IdleBehavior = NewObject<UIdleBehavior>(PawnData.m_IdleBehav->GetDefaultObject(), PawnData.m_IdleBehav);

	m_DetectBehavior = NewObject<USawPlayerBehavior>(PawnData.m_SawPlayerBehav->GetDefaultObject(), PawnData.m_SawPlayerBehav);

	m_ReturnToPosBehavior = NewObject<UReturnBehavior>(PawnData.m_ReturnPlayerBehav->GetDefaultObject(), PawnData.m_ReturnPlayerBehav);
	//

}

EPathFollowingRequestResult::Type AConnectablePawn::MoveToLocation(FVector loc)
{
	return GetAICon()->MoveToLocation(loc,10.f);
}

EPathFollowingRequestResult::Type AConnectablePawn::MoveToActor(AActor * target)
{
return	GetAICon()->MoveToActor(target, 10.f);
}

float AConnectablePawn::GetRadius()
{
	return m_Sphere->GetScaledSphereRadius();
}

void AConnectablePawn::SetFSM(EFSM fsm)
{
	m_CurrentState = fsm;
}

//무브먼트타겟을 바꿀것인가

//빙의를 시킬것인가

//배터리는 빙의되선 안되고

//로봇과 자동차는 빙의되야한다.

