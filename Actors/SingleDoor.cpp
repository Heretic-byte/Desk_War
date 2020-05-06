#include "SingleDoor.h"
#include "ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "USB_PlayerController.h"

ASingleDoor::ASingleDoor()
{
	m_bDidShow = false;
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root00");
	//
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundLeftMesh(TEXT("SkeletalMesh'/Game/Meshes/Prop_205/Tutorial/Gate/SK_Tuto_Door_01_L.SK_Tuto_Door_01_L'"));
	check(FoundLeftMesh.Object);
	m_MeshDoor = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh00");
	m_MeshDoor->SetupAttachment(RootComponent);
	m_MeshDoor->SetSkeletalMesh(FoundLeftMesh.Object);
	m_MeshDoor->SetCollisionProfileName("BlockAllDynamic");
	m_MeshDoor->RelativeLocation = FVector(7.f, 206.f, 0.f);
	//
	m_CamTimer = CreateDefaultSubobject<UCamTimer>("Cam00");
	m_CamTimer->SetupAttachment(RootComponent);
	m_CamTimer->FieldOfView = 100.f;
	m_CamTimer->RelativeLocation = FVector(584.75f, 293.7f, 112.f);
	m_CamTimer->RelativeRotation = FRotator(-20.f, -154.f, -1.3f);
	//
	m_PuzzleDoor = CreateDefaultSubobject<UPuzzleDoor>("PuzzleDoor00");
	m_ActionManager = CreateDefaultSubobject<UActionManagerComponent>("ActionManager00");

	m_AryMatBrightness.Reset();
}

void ASingleDoor::BeginPlay()
{
	Super::BeginPlay();
	m_PuzzleDoor->m_OnDoorUnlocked.AddUObject(this, &ASingleDoor::OpenDoor);
	m_PuzzleDoor->m_OnDoorLocked.AddUObject(this, &ASingleDoor::CloseDoor);

	SetMaterialAry(m_MeshDoor,m_AryMatBrightness,m_AryMatGauge);

	m_InitDoorRot = m_MeshDoor->GetComponentRotation();
}

void ASingleDoor::OpenDoor()
{
	if (!m_bDidShow)
	{
		m_CamTimer->ShowCamera(UGameplayStatics::GetPlayerController(GetWorld(), 0), m_fCamShowTime);
		m_bDidShow = true;
	}

	FRotator RotWant = m_InitDoorRot + m_DoorOpenRot;

	auto* Action = UCActionFactory::MakeRotateComponentToAction(m_MeshDoor, RotWant, m_fOpenTime);

	m_ActionManager->AddAction(UCActionFactory::MakeRotateComponentToAction(m_MeshDoor, RotWant, m_fOpenTime));

	m_MeshDoor->SetCollisionProfileName("IgnoreCamDynamic");

}

void ASingleDoor::CloseDoor()
{
	m_ActionManager->RemoveAllActions();

	auto* Action = UCActionFactory::MakeRotateComponentToAction(m_MeshDoor, m_InitDoorRot, m_fOpenTime);

	m_ActionManager->AddAction(Action);

	m_MeshDoor->SetCollisionProfileName("BlockAllDynamic");
}

void ASingleDoor::SetMaterialAry(USkeletalMeshComponent * meshDoor, TArray<UMaterialInstanceDynamic*>& brgtMatAry, TArray<UMaterialInstanceDynamic*>& gagMatAry)
{
	float NotUse;

	TArray<UMaterialInterface*> MatAry = meshDoor->GetMaterials();

	FMaterialParameterInfo InfoBrightness;
	InfoBrightness.Name = "Brightness";

	FMaterialParameterInfo InfoGauge;
	InfoGauge.Name = "Gauge";

	bool bHasBright = false;
	bool bHasGauge = false;

	for (int i = 0; i < MatAry.Num(); i++)
	{
		bHasBright = MatAry[i]->GetScalarParameterValue(InfoBrightness, NotUse);
		bHasGauge= MatAry[i]->GetScalarParameterValue(InfoGauge, NotUse);

		if (!bHasBright && !bHasGauge)
		{
			continue;
		}

		auto* MatInstanceDynamic = UMaterialInstanceDynamic::Create(MatAry[i], meshDoor);

		if (bHasBright)
		{
			brgtMatAry.Add(MatInstanceDynamic);
		}

		if (bHasGauge)
		{
			gagMatAry.Add(MatInstanceDynamic);
		}

		meshDoor->SetMaterial(i, MatInstanceDynamic);
	}

}

void ASingleDoor::SetGauge(int matIndexFromAry, float gaugePerOne)
{
	m_AryMatGauge[matIndexFromAry]->SetScalarParameterValue("Gauge", gaugePerOne);
}

void ASingleDoor::SetBrightness(int matIndexFromAry, float perOne)
{
	m_AryMatBrightness[matIndexFromAry]->SetScalarParameterValue("Brightness", perOne);
}

