

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/AudioComponent.h"
#include "USBFunctionLib.generated.h"

/**
 * 
 */
UCLASS()
class DESK_WAR_API UUSBFunctionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable,Category="Material")
	static UMaterialInstanceDynamic* CreateSetDynamicMaterial(UMeshComponent* meshComp, int matIndex)
	{
		auto* Mat = meshComp->GetMaterial(matIndex);
		auto* MatInstanceDynamic= UMaterialInstanceDynamic::Create(Mat, meshComp);
		meshComp->SetMaterial(matIndex, MatInstanceDynamic);

		return MatInstanceDynamic;
	}

	UFUNCTION(BlueprintCallable, Category = "Material")
	static void SetAudioPlay(UAudioComponent* audioComp, float pitch = 1.f , float volume = 1.f,USoundBase* soundBase = nullptr)
	{
		if (soundBase)
		{
			audioComp->SetSound(soundBase);
		}
		audioComp->SetPitchMultiplier(pitch);
		audioComp->SetVolumeMultiplier(volume);
		audioComp->Play();
	}
};
