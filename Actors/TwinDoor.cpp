#include "TwinDoor.h"
#include "ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "USB_PlayerController.h"
#include "FuncLib/USBFunctionLib.h"

// Sets default values
ATwinDoor::ATwinDoor()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundRightMesh(TEXT("SkeletalMesh'/Game/Meshes/Prop_205/Tutorial/Gate/SK_Tuto_Door_01_R.SK_Tuto_Door_01_R'"));
	check(FoundRightMesh.Object);
	m_MeshSecondDoor = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh01");
	m_MeshSecondDoor->SetupAttachment(RootComponent);
	m_MeshSecondDoor->SetSkeletalMesh(FoundRightMesh.Object);
	m_MeshSecondDoor->SetCollisionProfileName("BlockAllDynamic");
	m_MeshSecondDoor->RelativeLocation = FVector(0.f, -213.f, 0.f);
	//
	m_AryMatBrightnessSecond.Reset();
	m_AryMatGaugeSecond.Reset();
}

// Called when the game starts or when spawned
void ATwinDoor::BeginPlay()
{
	Super::BeginPlay();

	SetMaterialAry(m_MeshDoor, m_AryMatBrightnessSecond, m_AryMatGaugeSecond);

	m_InitDoorRotSecond = m_MeshSecondDoor->GetComponentRotation();
}

void ATwinDoor::OpenDoor()
{
	Super::OpenDoor();

	FRotator RotSecond = m_InitDoorRotSecond + m_DoorOpenRotSecond;

	m_ActionManager->AddAction(UCActionFactory::MakeRotateComponentToAction(m_MeshSecondDoor, RotSecond, m_fOpenTime));
}

void ATwinDoor::CloseDoor()
{
	Super::CloseDoor();
	m_ActionManager->AddAction(UCActionFactory::MakeRotateComponentToAction(m_MeshSecondDoor, m_InitDoorRotSecond, m_fOpenTime));
}

