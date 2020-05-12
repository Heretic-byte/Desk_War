#include "ConnectablePawn.h"
#include "Kismet/GameplayStatics.h"
#include "Datas/ConnectablePawnData.h"
#include "UObjects/ConnectionBehavior.h"
#include "ConstructorHelpers.h"
#include "Datas/USB_Macros.h"
#include "Managers/USB_GameManager.h"
// Sets default values
AConnectablePawn::AConnectablePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
}

void AConnectablePawn::SetUpSceneComponent(USceneComponent * compo, USceneComponent * parent, FTransform trans)
{
	compo->SetupAttachment(parent);
	SetUpActorComponent(compo);
}

void AConnectablePawn::SetUpActorComponent(UActorComponent * compo)
{
	compo->RegisterComponent();
	AddInstanceComponent(compo);
}

// Called when the game starts or when spawned
void AConnectablePawn::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(false);

	if (m_PawnID != NAME_None)
	{
		SetConnectPawn(m_PawnID);
	}
}

void AConnectablePawn::OnConnected(IConnectable * portTarget)
{

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

	//
	if (PawnData.m_ConnectBehav != nullptr)
	{
		m_ConnectionBehav = NewObject<UConnectionBehavior>( PawnData.m_ConnectBehav->GetDefaultObject(), PawnData.m_ConnectBehav);
	}
	//배터리가 여기 포함,그밖에 주는것들

	if (!PawnData.m_bIsAI)
	{
		return;
	}

	SetActorTickEnabled(true);
	//pawn sensing
	//각도
	//movement
	//이속 점프력

	//얘네 스텟은 어디서?

	//생각해보면 배터리말고는 전부 어댑터아닌가

	//일단은 만들자

	//지금 만드는 시스템ㅇ;

	//커넥션 인터페이스 및 맵을 통한 커넥터 저장

	//커넥션 디커넥션 모든 시전 주체는 플레이어다.
}

