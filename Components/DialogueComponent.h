// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialogue.h"
#include "Datas/USB_Macros.h"
#include "DialogueComponent.generated.h"

/**
 * 
 */


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DESK_WAR_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Dialogue")
	UDialogue* m_DialogueData;
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite ,Category = "Dialogue")
	FVoidVoidBP m_OnDialogueStart;
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite, Category = "Dialogue")
	FVoidVoidBP m_OnDialogueEnd;
protected:
	virtual void BeginPlay() override;
public:
	UFUNCTION(BlueprintCallable,Category="Dialogue")
	void StartDialogue();
	

	//È¥Àã¸» ÀÖ¾î¾ßÇÔ
};
