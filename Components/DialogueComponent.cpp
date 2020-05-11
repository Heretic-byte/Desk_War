#include "DialogueComponent.h"
#include "DialogueUserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

UDialogueComponent::UDialogueComponent(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	m_Widget = nullptr;
	m_ClassDial = UDialogueUserWidget::StaticClass();//ref bind
}

void UDialogueComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDialogueComponent::StartDialogue()
{
	if (!m_DialogueModData)
	{
		return;
	}
	auto* PController= UGameplayStatics::GetPlayerController(GetWorld(),0);
	m_Widget=CreateWidget<UDialogueUserWidget>(PController,m_ClassDial);
	m_Widget->NPCActor = GetOwner();
	m_Widget->InDialogue = m_DialogueModData;
	m_Widget->AddToViewport();

}
