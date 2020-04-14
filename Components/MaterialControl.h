

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MaterialControl.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DESK_WAR_API UMaterialControl : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Color", meta = (ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	float m_fAlphaValueAbs;
	//UPROPERTY(EditDefaultsOnly, Category = "Color", meta = (ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	//float m_fAlphaValueRate;
	UPROPERTY(EditAnywhere,Category="Color")
	FName m_NameLinearColor;
	// Sets default values for this component's properties
	UMaterialControl();
	UPROPERTY()
	UMeshComponent* m_MeshComp;
	UPROPERTY()
	TArray< UMaterialInstanceDynamic*> m_AryMats;
	//UPROPERTY()
	TArray<FLinearColor*> m_AryMatInitColors;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
public:
	void SetAlpha();
	void SetInitAlpha();
};
