// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableComponent.h"
#include "Actors/USB_PlayerCharacter.h"

UInteractableComponent::UInteractableComponent(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	m_bUnlocked = true;


}

bool UInteractableComponent::CheckUnlock()
{
	return m_bUnlocked;
}

void UInteractableComponent::SetLock()
{
	m_bUnlocked = false;
	m_OnLocked.Broadcast();
}

void UInteractableComponent::SetUnlock()
{
	m_bUnlocked = true;
	m_OnUnlocked.Broadcast();
}

void UInteractableComponent::SetFocusIn()
{
	m_OnFocusIn.Broadcast();
}

void UInteractableComponent::SetFocusOut()
{
	m_OnFocusOut.Broadcast();
}

void UInteractableComponent::Interact(AUSB_PlayerCharacter* interMan)
{
	if (!m_bUnlocked)
	{
		m_OnCantInteract.Broadcast(interMan);
		return;
	}
	m_OnInteract.Broadcast(interMan);
}

void UInteractableComponent::SetActive(bool bNewActive, bool bReset)
{
	Super::SetActive(bNewActive, bReset);
	if (bNewActive)
	{
		SetUnlock();
		return;
	}
	SetLock();
}

