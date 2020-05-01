#include "TwinDoor.h"
#include "ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "USB_PlayerController.h"
#include "FuncLib/USBFunctionLib.h"

// Sets default values
ATwinDoor::ATwinDoor()
{
	m_nMatIndex = 7;
	m_bDidShow = false;
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root00");
	//
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundLeftMesh(TEXT("SkeletalMesh'/Game/Meshes/Prop_205/Tutorial/Gate/SK_Tuto_Door_01_L.SK_Tuto_Door_01_L'"));
	check(FoundLeftMesh.Object);
	m_MeshLeftDoor = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh00");
	m_MeshLeftDoor->SetupAttachment(RootComponent);
	m_MeshLeftDoor->SetSkeletalMesh(FoundLeftMesh.Object);
	m_MeshLeftDoor->SetCollisionProfileName("BlockAllDynamic");
	m_MeshLeftDoor->RelativeLocation = FVector(7.f,206.f,0.f);
	//
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundRightMesh(TEXT("SkeletalMesh'/Game/Meshes/Prop_205/Tutorial/Gate/SK_Tuto_Door_01_R.SK_Tuto_Door_01_R'"));
	check(FoundRightMesh.Object);
	m_MeshRightDoor = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh01");
	m_MeshRightDoor->SetupAttachment(RootComponent);
	m_MeshRightDoor->SetSkeletalMesh(FoundRightMesh.Object);
	m_MeshRightDoor->SetCollisionProfileName("BlockAllDynamic");
	m_MeshRightDoor->RelativeLocation = FVector(0.f, -213.f, 0.f);
	//
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundWireMesh(TEXT("StaticMesh'/Game/Meshes/Prop_205/Tutorial/Gate/SM_Port_Gate_First.SM_Port_Gate_First'"));
	check(FoundWireMesh.Object);
	m_MeshPropWire = CreateDefaultSubobject<UStaticMeshComponent>("Mesh02");
	m_MeshPropWire->SetStaticMesh(FoundWireMesh.Object);
	m_MeshPropWire->SetCollisionProfileName("BlockAll");
	m_MeshPropWire->SetupAttachment(RootComponent);
	m_MeshPropWire->RelativeLocation = FVector(13.f,0.f,0.f);
	//
	m_CamTimer = CreateDefaultSubobject<UCamTimer>("Cam00");
	m_CamTimer->SetupAttachment(RootComponent);
	m_CamTimer->FieldOfView = 100.f;
	m_CamTimer->RelativeLocation = FVector(584.75f,293.7f,112.f);
	m_CamTimer->RelativeRotation = FRotator(-20.f,-154.f,-1.3f);
	//
	m_PuzzleDoor = CreateDefaultSubobject<UPuzzleDoor>("PuzzleDoor00");
	m_ActionManager = CreateDefaultSubobject<UActionManagerComponent>("ActionManager00");
}

// Called when the game starts or when spawned
void ATwinDoor::BeginPlay()
{
	Super::BeginPlay();
	m_InitLeftRot = m_MeshLeftDoor->GetComponentRotation();
	m_InitRightRot = m_MeshRightDoor->GetComponentRotation();

	m_PuzzleDoor->m_OnDoorUnlocked.AddUObject(this,&ATwinDoor::OpenDoor);
	m_PuzzleDoor->m_OnDoorLocked.AddUObject(this, &ATwinDoor::CloseDoor);
	m_Mat = UUSBFunctionLib::CreateSetDynamicMaterial(m_MeshLeftDoor, m_nMatIndex);
}

void ATwinDoor::OpenDoor()
{
	if (!m_bDidShow)
	{
		m_CamTimer->ShowCamera(UGameplayStatics::GetPlayerController(GetWorld(),0),2.f);
		m_bDidShow = true;
	}

	FRotator LwantRot = m_InitLeftRot;
	LwantRot.Yaw += 135.f;
	FRotator RwantRot = m_InitRightRot;
	RwantRot.Yaw += -135.f;

 	m_ActionManager->AddAction(UCActionFactory::MakeRotateComponentToAction(m_MeshLeftDoor,LwantRot,3.f));
	m_ActionManager->AddAction(UCActionFactory::MakeRotateComponentToAction(m_MeshRightDoor, RwantRot, 3.f));
}

void ATwinDoor::CloseDoor()
{
	m_ActionManager->RemoveAllActions();
	m_ActionManager->AddAction(UCActionFactory::MakeRotateComponentToAction(m_MeshLeftDoor, m_InitLeftRot, 3.f));
	m_ActionManager->AddAction(UCActionFactory::MakeRotateComponentToAction(m_MeshRightDoor, m_InitRightRot, 3.f));
}

// Called every frame
void ATwinDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATwinDoor::SetGauge(float gaugePerOne)
{
	m_Mat->SetScalarParameterValue("Gauge", gaugePerOne);
}

