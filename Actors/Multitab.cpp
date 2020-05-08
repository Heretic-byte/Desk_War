#include "Multitab.h"
#include "ConstructorHelpers.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/PlayerController.h"
#include "Actors/SingleDoor.h"

AMultitab::AMultitab(const FObjectInitializer& objInit)
	:Super(objInit)
{
	PrimaryActorTick.bCanEverTick = false;
	//
	static ConstructorHelpers::FObjectFinder<UAnimSequence> FoundOpenAnim(TEXT("AnimSequence'/Game/AnimBlueprints/Props/SK_Multitap_ButtonDown_Anim.SK_Multitap_ButtonDown_Anim'"));
	check(FoundOpenAnim.Object);
	m_OpenAnim = FoundOpenAnim.Object;

	static ConstructorHelpers::FObjectFinder<UAnimSequence> FoundCloseAnim(TEXT("AnimSequence'/Game/AnimBlueprints/Props/SK_Multitap_ButtonUp_Anim.SK_Multitap_ButtonUp_Anim'"));
	check(FoundCloseAnim.Object);
	m_CloseAnim = FoundCloseAnim.Object;
	//
	m_PuzzleKey = CreateDefaultSubobject<UPuzzleKey>("PuzzleKey00");
	//
	m_MeshMultitabBody = CreateDefaultSubobject<UStaticMeshComponent>("Mesh00");

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundStaticMesh(TEXT("StaticMesh'/Game/Meshes/Prop_205/Tutorial/Gate/SM_Multitap_Wall.SM_Multitap_Wall'"));
	check(FoundStaticMesh.Object);
	m_MeshMultitabBody->SetStaticMesh(FoundStaticMesh.Object);

	m_MeshMultitabBody->SetCollisionProfileName("IgnoreCamStatic");

	RootComponent = m_MeshMultitabBody;
	//
	m_MeshPowerButton = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh01");
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoundSkMesh(TEXT("SkeletalMesh'/Game/Meshes/Prop_205/Tutorial/Gate/SK_Multitap_Button.SK_Multitap_Button'"));
	check(FoundSkMesh.Object);
	m_MeshPowerButton->SetSkeletalMesh(FoundSkMesh.Object);
	m_MeshPowerButton->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	m_MeshPowerButton->OverrideAnimationData(m_OpenAnim,false,false);
	m_MeshPowerButton->RelativeLocation = FVector(163.f,-176.f,-177.f);
	m_MeshPowerButton->RelativeRotation = FRotator(0.f,0.f,15.f);
	m_MeshPowerButton->RelativeScale3D = FVector(1.33f, 1.33f, 1.33f);
	m_MeshPowerButton->SetupAttachment(RootComponent);
	//
	m_AudioComp = CreateDefaultSubobject<UAudioComponent>("Audio00");
	m_AudioComp->SetAutoActivate(false);
	m_AudioComp->SetVolumeMultiplier(0.35f);
	static ConstructorHelpers::FObjectFinder<USoundBase> FoundSound(TEXT("SoundWave'/Game/SFX/USB/Put_in/SE_SFX_BUTTON_PUSH.SE_SFX_BUTTON_PUSH'"));
	if (FoundSound.Succeeded())
		m_AudioComp->SetSound(FoundSound.Object);

	//
	m_CollButtonDown = CreateDefaultSubobject<UBoxComponent>("Box00");
	m_CollButtonDown->RelativeLocation = FVector(163.f,-194.f,-198.f);
	m_CollButtonDown->SetBoxExtent(FVector(32.f, 32.f, 10.f), false);
	m_CollButtonDown->SetCollisionProfileName("USBOverlap");
	m_CollButtonDown->SetupAttachment(RootComponent);
	//
	m_CollButtonUp = CreateDefaultSubobject<UBoxComponent>("Box01");
	m_CollButtonUp->RelativeLocation = FVector(163.f, -194.f, -155.f);
	m_CollButtonUp->SetBoxExtent(FVector(32.f,32.f,10.f),false);
	m_CollButtonUp->SetCollisionProfileName("USBOverlap");
	m_CollButtonUp->SetupAttachment(RootComponent);
	//
	m_bButtonOff = true;
}//down is open

// Called when the game starts or when spawned
void AMultitab::BeginPlay()
{
	Super::BeginPlay();
	m_MatForSwitch = UUSBFunctionLib::CreateSetDynamicMaterial(m_MeshPowerButton,0);
	m_CollButtonDown->OnComponentBeginOverlap.AddDynamic(this,&AMultitab::OverlapOpen);
	m_CollButtonUp->OnComponentBeginOverlap.AddDynamic(this, &AMultitab::OverlapClose);
}

void AMultitab::OpenSwitch()
{
	if (!m_bButtonOff)
	{
		return;
	}

	m_bButtonOff = false;

	m_MeshPowerButton->PlayAnimation(m_OpenAnim, false);

	m_PuzzleKey->UnlockPuzzle();

	m_MatForSwitch->SetScalarParameterValue("Brightness",0.4f);

	m_PuzzleKey->SetPuzzleDoorGauge(1.0f);
	m_PuzzleKey->SetPuzzleDoorBrightness(4.f);

	m_AudioComp->Play();

	PRINTF("Open");
}

void AMultitab::CloseSwitch()
{
	if (m_bButtonOff)
	{
		return;
	}

	m_bButtonOff = true;

	m_MeshPowerButton->PlayAnimation(m_CloseAnim, false);

	m_PuzzleKey->LockPuzzle();

	m_MatForSwitch->SetScalarParameterValue("Brightness", 0.f);

	m_PuzzleKey->SetPuzzleDoorBrightness(0.f);
	m_PuzzleKey->SetPuzzleDoorGauge(0.f);

	m_AudioComp->Play();

	PRINTF("Close");
}
// UPrimitiveComponent*, OverlappedComponent, AActor*, OtherActor, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex, bool, bFromSweep, const FHitResult &, SweepResult);
void AMultitab::OverlapOpen(UPrimitiveComponent * overlappedSelf, AActor * otherActor, UPrimitiveComponent * otherComp, int32 bodyIndex, bool fromSweep,const FHitResult & sweepResult)
{
	if (!Cast<APlayerController>(Cast<APawn>(otherActor)->GetController()))
	{
		return;
	}

	OpenSwitch();
}

void AMultitab::OverlapClose(UPrimitiveComponent * overlappedSelf, AActor * otherActor, UPrimitiveComponent * otherComp, int32 bodyIndex, bool fromSweep,const FHitResult & sweepResult)
{
	if (!Cast<APlayerController>(Cast<APawn>(otherActor)->GetController()))
	{
		return;
	}

	CloseSwitch();
}

