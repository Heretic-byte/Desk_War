


#include "MaterialControl.h"
#include "GameFramework/Actor.h"
#include "Datas/USB_Macros.h"
// Sets default values for this component's properties
UMaterialControl::UMaterialControl()
{
	m_fAlphaValueAbs = 0.f;
	PrimaryComponentTick.bCanEverTick = false;
	m_NameLinearColor = "LinearColor";
}


// Called when the game starts
void UMaterialControl::BeginPlay()
{
	Super::BeginPlay();
	m_MeshComp = Cast<UMeshComponent> (GetOwner()->GetComponentByClass(UMeshComponent::StaticClass()));

	if (!m_MeshComp)
	{
		PRINTF("%s, There is No MeshComp",*GetOwner()->GetName());
		return;
	}

	auto Mats= m_MeshComp->GetMaterials();

	FLinearColor ColorWant;
	for (auto Mat : Mats)
	{
		UMaterialInstanceDynamic* InstanceMat = UMaterialInstanceDynamic::Create(Mat, this);
		m_AryMats.Add(InstanceMat);
		if (InstanceMat->GetVectorParameterValue(m_NameLinearColor, ColorWant))
		{
			m_AryMatInitColors.Add(&ColorWant);
		}
		else
		{
			PRINTF("Fail Mat GetParameter,Check the Param Name!");
		}
	}
}

void UMaterialControl::SetAlpha()
{
	if (!m_MeshComp)
	{
		PRINTF("%s There is No MeshComp 1", *GetOwner()->GetName());
		return;
	}


	int Length = m_AryMatInitColors.Num();
	for (int i = 0; i < Length; i++)
	{
		FLinearColor ColorWant = *m_AryMatInitColors[i];
		ColorWant.A = m_fAlphaValueAbs;
		m_AryMats[i]->SetVectorParameterValue(m_NameLinearColor, ColorWant);
		m_MeshComp->SetMaterial(i, m_AryMats[i]);
	}
}

void UMaterialControl::SetInitAlpha()
{
	if (!m_MeshComp)
	{
		PRINTF("%s There is No MeshComp 2", *GetOwner()->GetName());
		return;
	}
	int Length = m_AryMatInitColors.Num();
	for (int i = 0; i < Length; i++)
	{
		m_AryMats[i]->SetVectorParameterValue(m_NameLinearColor,*m_AryMatInitColors[i]);
		m_MeshComp->SetMaterial(i, m_AryMats[i]);
	}
}
