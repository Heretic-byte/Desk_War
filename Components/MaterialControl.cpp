


#include "MaterialControl.h"
#include "GameFramework/Actor.h"
#include "Datas/USB_Macros.h"
// Sets default values for this component's properties
UMaterialControl::UMaterialControl()
{
	m_fAlphaValueAbs = 0.f;
	PrimaryComponentTick.bCanEverTick = false;
	m_NameAlphaParam = "Opacity";
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

	float AlphaWant;
	for (auto Mat : Mats)
	{
		UMaterialInstanceDynamic* InstanceMat = UMaterialInstanceDynamic::Create(Mat, this);
		m_AryMats.Add(InstanceMat);
		if (InstanceMat->GetScalarParameterValue(m_NameAlphaParam, AlphaWant))
		{
			m_AryMatInitAlphas.Add(AlphaWant);
		}
		else
		{
			PRINTF("%s Fail Mat GetParameter,Check the Param Name! -1 MatControlComp",*GetOwner()->GetName());
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


	int Length = m_AryMatInitAlphas.Num();
	for (int i = 0; i < Length; i++)
	{
		m_AryMats[i]->SetScalarParameterValue(m_NameAlphaParam, m_fAlphaValueAbs);
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
	int Length = m_AryMatInitAlphas.Num();
	for (int i = 0; i < Length; i++)
	{
		m_AryMats[i]->SetScalarParameterValue(m_NameAlphaParam,m_AryMatInitAlphas[i]);
		m_MeshComp->SetMaterial(i, m_AryMats[i]);
	}
}
