#include "PawnSpawner.h"
#include "ConstructorHelpers.h"
#include "Managers/USB_GameManager.h"
#include "Datas/USB_Macros.h"
#include "Datas/ConnectablePawnData.h"
#include "Actors/ConnectablePawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "AIController.h"
// Sets default values
APawnSpawner::APawnSpawner()
{
	m_NameConnectorID = NAME_None;
	m_bCanSpawn = true;
	m_SpawnedPawn = nullptr;
	m_SpawnOffset = FVector(0,0,100.f);
	m_bCanSeeSpawner = false;
	m_MeshBase = CreateDefaultSubobject<UStaticMeshComponent>("Mesh00");
	RootComponent = m_MeshBase;
	m_MeshBase->SetEnableGravity(false);
	m_MeshBase->SetCastShadow(false);
	m_MeshBase->SetCollisionProfileName("NoCollision");
	m_MeshBase->SetGenerateOverlapEvents(false);
	//
	//
#if WITH_EDITOR
	m_MeshSpawnPwnProxy = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh01");
	m_MeshSpawnPwnProxy->SetupAttachment(RootComponent);
	m_MeshSpawnPwnProxy->SetEnableGravity(false);
	m_MeshSpawnPwnProxy->SetCastShadow(false);
	m_MeshSpawnPwnProxy->SetCollisionProfileName("NoCollision");
	m_MeshSpawnPwnProxy->SetHiddenInGame(true);
	//
	m_BilboardForSpawnerLoc = CreateDefaultSubobject<UBillboardComponent>("BillBoard02");
	m_BilboardForSpawnerLoc->SetupAttachment(RootComponent);
	m_BilboardForSpawnerLoc->SetHiddenInGame(true);

	static ConstructorHelpers::FObjectFinder<UTexture2D> FoundTexture1(TEXT("Texture2D'/Game/Texture/T_SpawnPivot.T_SpawnPivot'"));
	if (FoundTexture1.Succeeded())
	{
		m_BilboardForSpawnerLoc->SetSprite(FoundTexture1.Object);
	}
	else
	{
		check(FoundTexture1.Object);
	}
	
	//
	m_BilboardForSpawnOffset = CreateDefaultSubobject<UBillboardComponent>("BillBoard03");
	m_BilboardForSpawnOffset->SetupAttachment(RootComponent);
	m_BilboardForSpawnOffset->SetHiddenInGame(true);

	static ConstructorHelpers::FObjectFinder<UTexture2D> FoundTexture2(TEXT("Texture2D'/Game/Texture/T_Spawner.T_Spawner'"));
	if (FoundTexture2.Succeeded())
	{
		m_BilboardForSpawnOffset->SetSprite(FoundTexture2.Object);
	}
	else
	{
		check(FoundTexture2.Object);
	}

	m_BilboardForSpawnOffset->RelativeLocation = m_SpawnOffset;


	//DataTable'/Game/Datas/ConnectPawnTable.ConnectPawnTable'
	static ConstructorHelpers::FObjectFinder<UDataTable> FoundTable(TEXT("DataTable'/Game/Datas/ConnectPawnTable.ConnectPawnTable'"));
	if (FoundTable.Succeeded())
	{
		m_DataTable=FoundTable.Object;
	}
	else
	{
		check(FoundTable.Object);
	}
#endif
	m_PawnAreaProxy = CreateDefaultSubobject<USphereComponent>("Sphere04");
	m_PawnAreaProxy->SetupAttachment(RootComponent);
	m_PawnAreaProxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	m_PawnAreaProxy->SetHiddenInGame(true);
	m_PawnAreaProxy->SetSphereRadius(300.f);

}


//Texture2D'/Game/Texture/T_SpawnPivot.T_SpawnPivot'
//Texture2D'/Game/Texture/T_Spawner.T_Spawner'
#if WITH_EDITOR
void APawnSpawner::PostInitProperties()
{
	Super::PostInitProperties();
	SetTablePawnProperty();
}
void APawnSpawner::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	SetTablePawnProperty();
}

void APawnSpawner::SetTablePawnProperty()
{
	FVector NewOffset = m_SpawnOffset;
	NewOffset.Z += 130.f;
	m_BilboardForSpawnOffset->SetRelativeLocation(NewOffset);

	if (m_NameConnectorID == NAME_None)
	{
		m_MeshSpawnPwnProxy->SetSkeletalMesh(nullptr);
		m_bCanSpawn = false;
	}

	if (m_ClassPawnToSpawn == nullptr && m_NameConnectorID == NAME_None)
	{
		m_bCanSpawn = false;
		return;
	}

	if (m_NameConnectorID != NAME_None)
	{
		const FConnectablePawn_Data& Data = *m_DataTable->FindRow<FConnectablePawn_Data>(m_NameConnectorID, "");

		if (!&Data)
		{
			PRINTF("NameID Wrong !");
			m_MeshSpawnPwnProxy->SetSkeletalMesh(nullptr);
			m_bCanSpawn = false;
			return;
		}

		m_MeshSpawnPwnProxy->SetSkeletalMesh(Data.m_MeshPawnMainBody);
		m_bCanSpawn = true;
		//m_PawnAreaProxy->SetSphereRadius(Data.m_fAreaRadius);
	}
	else
	{
		USkeletalMeshComponent* Compo = Cast<USkeletalMeshComponent>(m_ClassPawnToSpawn->GetDefaultObject<AActor>()->GetComponentByClass(USkeletalMeshComponent::StaticClass()));

		if (!Compo->SkeletalMesh)
		{
			m_MeshSpawnPwnProxy->SetSkeletalMesh(nullptr);
			m_bCanSpawn = false;
			return;
		}

		m_MeshSpawnPwnProxy->SetSkeletalMesh(Compo->SkeletalMesh);
		m_bCanSpawn = true;
	}


	m_MeshSpawnPwnProxy->SetRelativeRotation(m_SpawnRotateOffset);

	m_MeshSpawnPwnProxy->SetRelativeLocation(m_SpawnOffset);
}


#endif

// Called when the game starts or when spawned
void APawnSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (!m_bCanSpawn)
	{
		return;
	}
	m_MeshBase->SetVisibility(m_bCanSeeSpawner);
	m_MeshBase->SetCanEverAffectNavigation(m_bCanSeeSpawner);
	if (m_bCanSeeSpawner)
	{
		m_MeshBase->SetCollisionProfileName("BlockAll");
	}

	SpawnPawn();
}

void APawnSpawner::SpawnPawn()
{
	if (m_ClassPawnToSpawn == nullptr&&m_NameConnectorID == NAME_None)
	{
		return;
	}

	FActorSpawnParameters Param;
	Param.bNoFail = true;
	Param.Owner = this;

	if (m_NameConnectorID == NAME_None)
	{
		m_SpawnedPawn = GetWorld()->SpawnActor<APawn>(m_ClassPawnToSpawn, GetActorLocation() + m_SpawnOffset, GetActorRotation() + m_SpawnRotateOffset, Param);
	}
	else
	{
		auto* CreatedPawn= GetWorld()->SpawnActor<AConnectablePawn>(AConnectablePawn::StaticClass(), GetActorLocation() + m_SpawnOffset, GetActorRotation() + m_SpawnRotateOffset, Param);
		m_SpawnedPawn = CreatedPawn;
		CreatedPawn->SetConnectPawn(m_NameConnectorID);
		CreatedPawn->m_fSightRadiusSqr = FMath::Square<float>(m_PawnAreaProxy->GetScaledSphereRadius());

	}
}
