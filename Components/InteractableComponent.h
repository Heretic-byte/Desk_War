// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Datas/USB_Macros.h"

#include "InteractableComponent.generated.h"

/**
 * 
 */

class AUSB_PlayerCharacter;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DESK_WAR_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractBP, AUSB_PlayerCharacter*, player);
public:
	UInteractableComponent(const FObjectInitializer& ObjectInitializer);
public:
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FOnInteractBP m_OnInteract;
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FOnInteractBP m_OnCantInteract;
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FVoidVoidBP m_OnFocusIn;
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FVoidVoidBP m_OnFocusOut;
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FVoidVoidBP m_OnLocked;
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FVoidVoidBP m_OnUnlocked;
public:
	bool CheckUnlock();
	void SetLock();
	void SetUnlock();
	void SetFocusIn();
	void SetFocusOut();
	virtual void Interact(AUSB_PlayerCharacter* interMan);
protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Interact")
	bool m_bUnlocked;
	virtual void SetActive(bool bNewActive, bool bReset = false) override;
};
