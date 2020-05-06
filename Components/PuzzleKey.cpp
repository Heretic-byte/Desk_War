#include "PuzzleKey.h"
#include "Components/PuzzleDoor.h"
#include "Datas/USB_Macros.h"
#include "Actors/SingleDoor.h"
// Sets default values for this component's properties



UPuzzleKey::UPuzzleKey()
{
	PrimaryComponentTick.bCanEverTick = false;
	m_nPuzzleIndex = -1;
	m_LinkedDoor = nullptr;
}


void UPuzzleKey::InitPuzzleKey(UPuzzleDoor * door, int index)
{
	m_LinkedDoor = door;

	m_bPuzzleUnlocked = false;

	if (!m_LinkedDoor)
	{
		PRINTF("%s Doesnt Have Linked Door ! 1", *GetName());
	}

	m_nPuzzleIndex = index;
}

void UPuzzleKey::UnlockPuzzle()
{
	m_bPuzzleUnlocked = true;
	if (!m_LinkedDoor)
	{
		PRINTF("%s Doesnt Have Linked Door ! 2", *GetName());
		return;
	}
	m_LinkedDoor->PuzzleChanged();
}

void UPuzzleKey::LockPuzzle()
{
	m_bPuzzleUnlocked = false;
	if (!m_LinkedDoor)
	{
		PRINTF("%s Doesnt Have Linked Door ! 3", *GetName());
		return;
	}
	m_LinkedDoor->PuzzleChanged();
}

bool UPuzzleKey::IsKeyUnlocked()
{
	return m_bPuzzleUnlocked;
}

UPuzzleDoor * UPuzzleKey::GetLinkedDoor()
{
	return m_LinkedDoor;
}

void UPuzzleKey::SetPuzzleDoorBrightness(float v)
{
	Cast<ASingleDoor>(GetLinkedDoor()->GetOwner())->SetBrightness(m_nPuzzleIndex,v);
}

void UPuzzleKey::SetPuzzleDoorGauge(float v)
{
	Cast<ASingleDoor>(GetLinkedDoor()->GetOwner())->SetGauge(m_nPuzzleIndex, v);
}

