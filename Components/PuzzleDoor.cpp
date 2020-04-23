


#include "PuzzleDoor.h"
#include "Components/PuzzleKey.h"
#include "GameFramework/Actor.h"


// Sets default values for this component's properties
UPuzzleDoor::UPuzzleDoor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	m_nDoorUnlockCount = -1;
}

// Called when the game starts or when spawned
void UPuzzleDoor::BeginPlay()
{
	Super::BeginPlay();

	if (m_AryKeyActor.Num() < 1)
	{
		PRINTF("%s - Set Key Actor for Puzzle Door !!!!", *GetName());
	}
	for (auto PuzzleKeyActor : m_AryKeyActor)
	{
		auto* PuzzleKeyComp=  Cast<UPuzzleKey>(PuzzleKeyActor->GetComponentByClass(UPuzzleKey::StaticClass()));
		if (!PuzzleKeyComp)
		{
			PRINTF("%s - There is no KeyComponent In Actor:%s !", *PuzzleKeyActor->GetName());
			return;
		}

		m_AryKeyComponent.Add(PuzzleKeyComp);
		PuzzleKeyComp->InitPuzzleKey(this);
	}

}

// Called every frame

void UPuzzleDoor::PuzzleChanged()// call evty key changed
{
	int Count = 0;

	if (!m_AryKeyComponent.Num())
	{
		return;
	}

	for (int i = 0; i < m_AryKeyComponent.Num(); i++)
	{
		if (m_AryKeyComponent[i]->IsKeyUnlocked())
		{
			m_OnPuzzleUnlockCheckedBP.Broadcast(i);
			m_OnPuzzleUnlockChecked.Broadcast(i);
			Count++;
		}
		else
		{
			m_OnPuzzleLockCheckedBP.Broadcast(i);
			m_OnPuzzleLockChecked.Broadcast(i);
		}
	}

	if ((m_nDoorUnlockCount > 0 && m_nDoorUnlockCount < m_AryKeyComponent.Num()) || m_AryKeyComponent.Num() == Count)
	{
		UnlockDoor();
	}
	else
	{
		LockDoor();
	}
}

void UPuzzleDoor::UnlockDoor()
{
	m_OnDoorUnlockedBP.Broadcast();
	m_OnDoorUnlocked.Broadcast();
}

void UPuzzleDoor::LockDoor()
{
	m_OnDoorLockedBP.Broadcast();
	m_OnDoorLocked.Broadcast();
}
