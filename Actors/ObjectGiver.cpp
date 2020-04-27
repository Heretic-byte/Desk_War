#include "ObjectGiver.h"
#include "Engine/World.h"
// Sets default values
AObjectGiver::AObjectGiver()
{
	PrimaryActorTick.bCanEverTick = false;
	m_nCurrentIndex = 0;
}

void AObjectGiver::BeginPlay()
{
	Super::BeginPlay();
	CreatePoolObject();
}

void AObjectGiver::CreatePoolObject()
{
	int Count = m_nPoolCount;

	m_AryCreateActor.Reserve(Count);

	FVector MyPos = GetActorLocation();
	FRotator MyRot = GetActorRotation();

	while (Count--)//순서풀이랑 비순서 풀 생각
	{
		auto* Created=GetWorld()->SpawnActor<AActor>(m_cActorWantSpawn, MyPos, MyRot);
		Created->SetActorHiddenInGame(true);
		m_AryCreateActor.Emplace(Created);
	}
}

void AObjectGiver::ShowActor(FVector pos)
{
	m_AryCreateActor[m_nCurrentIndex]->SetActorLocation(pos);
	m_AryCreateActor[m_nCurrentIndex]->SetActorHiddenInGame(false);
}

