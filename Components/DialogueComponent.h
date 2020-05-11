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

class UDialogueUserWidget;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DESK_WAR_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UDialogueComponent(const FObjectInitializer& ObjectInitializer);
public:
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Dialogue")
	UDialogue* m_DialogueModData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	UDialogue* m_SelfTalkData;
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite ,Category = "Dialogue")
	FVoidVoidBP m_OnDialogueStart;
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite, Category = "Dialogue")
	FVoidVoidBP m_OnDialogueEnd;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player, HUD and UI")
	TSubclassOf<UDialogueUserWidget> m_ClassDial;

	UPROPERTY()
	UDialogueUserWidget* m_Widget;
protected:
	virtual void BeginPlay() override;
public:
	UFUNCTION(BlueprintCallable,Category="Dialogue")
	void StartDialogue();
	
	//UFUNCTION(BlueprintCallable, Category = "Dialogue")
	//void SelfTalk();
	//È¥Àã¸» ÀÖ¾î¾ßÇÔ
	//È¥Àã¸»À» ÀÏ¹Ý ´Ù¾â·Î±× ¿¡¼ÂÀ¸·Î ¾´´Ù¸é?
};
