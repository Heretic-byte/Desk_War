// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/MeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/ShapeComponent.h"
#include "PolarityObject.generated.h"

/**
 * 극성의 상태를 나타내는 열거형이다.
 * @E_Neutral	중성
 * @E_Red		양극
 * @E_Blue		음극
 */
UENUM(BlueprintType)
enum EPolarity
{
	E_Neutral	UMETA(DisplayName = "Neutral"),
	E_Red		UMETA(DisplayName = "Red"),
	E_Blue		UMETA(DisplayName = "Blue")
};

UCLASS()
class DESK_WAR_API APolarityObject : public AActor
{
	GENERATED_BODY()
	
public:	
	APolarityObject();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;



public:
	static const float STD_POWER;

	UPROPERTY(Category = "Polarity", VisibleDefaultsOnly, BlueprintReadWrite)
	USceneComponent* Root;

	/** 오브젝트가 보유하고 있는 양극의 수치를 나타낸다. */
	UPROPERTY(Category = "Polarity", VisibleDefaultsOnly, BlueprintReadOnly)
	int32 RedPolar;

	/** 오브젝트가 보유하고 있는 음극의 수치를 나타낸다. */
	UPROPERTY(Category = "Polarity", VisibleDefaultsOnly, BlueprintReadOnly)
	int32 BluePolar;

	/**
	 * 현재 극성과 가중치에 따라 영향을 주게되는 힘의 값이다.
	 * CalcPolarityPower()로 계산되며 값의 범위는 0 ~ 2330이다.
	 */
	UPROPERTY(Category = "Polarity", VisibleDefaultsOnly, BlueprintReadOnly)
	float PolarityPower;

	UPROPERTY(Category = "Polarity", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bCanPolarize"))
	int32 InitialPolarity;
#if WITH_EDITOR
	/**
	 * 에디터에서 초기 극성값을 설정하는 변수이다.
	 * 0이면 중성이므로 RedPolar와 BluePolar를 각각 0으로 초기화하고
	 * 양수이면 RedPolar에, 음수이면 BluePolar에 절대값을 초기화하고 상대극을 0으로 초기화한다.
	 */
#endif // WITH_EDITOR

	/**
	 * 힘에 더해지는 가중치.
	 * 범위는 0~9이며 기본값은 4이다.
	 */
	UPROPERTY(Category = "Polarity", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "9.0", UIMin = "0.0", UIMax = "9.0"))
	float Weight;

	/** 오브젝트가 영향을 줄 수 있는 범위 */
	UPROPERTY(Category = "Polarity", EditAnywhere, BlueprintReadWrite)
	float InfluenceRange;

	/**
	 * 외부에서 극성을 부여하거나 회수할 수 있는지 여부를 나타낸다.
	 * If true, Polarize / Depolarize 메소드를 통해 외부에서 특성 극성을 부여/회수 할 수 있다.
	 * If false, Polarize / Depolarize 메소드를 사용할 수 없으며 오로지 SetPolarity 메소드를
	 * 통해서만 극성을 세팅할 수 있다.
	 */
	UPROPERTY(Category = "Polarity", EditAnywhere, BlueprintReadWrite)
	uint8 bCanPolarize : 1;

	/**
	 * 동적으로 움직이는 Polarity 오브젝트들에 의해 영향을 받은 힘들의 합이다.
	 * AddAffectedForce()를 통해 Force가 더해지고 ConsumeAffectedForce()를 통해 사용할 수 있다.
	 * 또한 사용하지 않더라도 매 Tick마다  ClearAffectedForce() 가 호출되어 ZeroVector로 초기화된다.
	 */
	UPROPERTY(Category = "Polarity", EditDefaultsOnly, BlueprintReadWrite)
	FVector AffectedForce;

	UPROPERTY(Category = "Polarity", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	uint8 bInteractingWithInfluenceAreas : 1;

	UPROPERTY(Category = "Polarity", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	uint8 bInteractingWithAppointedGroup : 1;

	UPROPERTY(Category = "Polarity", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (EditCondition = "bInteractingWithAppointedGroup", DisplayAfter = "bInteractingWithAppointedGroup && AppointedGroup"))
	uint8 bSameInteractionWithInfluenceAreas : 1;

	UPROPERTY(Category = "Polarity", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (EditCondition = "bInteractingWithAppointedGroup", DisplayAfter = "bInteractingWithAppointedGroup"))
	TArray<APolarityObject*> AppointedGroup;

	UPROPERTY(Category = "Polarity", EditDefaultsOnly, BlueprintReadWrite)
	TArray<UShapeComponent*> InfluenceAreas;





protected:

	/**
	 * 오브젝트에서 극성을 보유하게될 Mesh Component이다.
	 * 해당 매쉬는 플레이어가 PolarityMode 진입시 극성에 따라 다른 색상을 띄며
	 * 플레이어가 극성을 부여/회수 할 수 있는 Mesh이다.
	 * 게임 시작 전 생성자를 통해 InitMesh()로 초기화를 시켜주어야 한다.
	 */
	UPROPERTY(Category = "Polarity", EditDefaultsOnly, BlueprintReadWrite)
	UMeshComponent* PolarityMesh;

	/** 상호작용할 가능성을 가능 Polarity 오브젝트들의 그룹 */
	UPROPERTY(Category = "Polarity", EditDefaultsOnly, BlueprintReadWrite)
	TArray<APolarityObject*> PotentialGroup;

	/** 만약 True라면 PotentialGroup을 갱신한다. */
	UPROPERTY(Category = "Polarity", EditDefaultsOnly, BlueprintReadWrite)
	uint8 bCheckPotentialGroup : 1;

private:
	/** CheckPotentialGroup()를 호출하고 경과한 시간 */
	float CheckPotentialGroupTimeStack;

	/** CheckPotentialGroup()를 호출할 주기. 호출할 때마다 랜덤하게 바뀐다. */
	float CheckPotentialGroupPeriod;
	


/** 에디터에서 초기 극성값을 설정하기 위해 필요한 메소드들이다. */
#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
private:

	/**
	 * InitialPolarity 값에 따라 극성을 초기화하는 메소드이다.
	 * 에디터에서 InitialPolarity 값을 수정하면 PostEditChangeProperty()를 통해 호출된다.
	 */
	void InitPolarity();
#endif // WITH_EDITOR


public:
	/**
	 * 양극성을 부여하는 메소드
	 * bCanPolarize가 true라면 RedPolar++
	 */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	void PolarizeRed();

	/**
	 * 음극성을 부여하는 메소드
	 * bCanPolarize가 true라면 BluePolar++
	 */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	void PolarizeBlue();

	/**
	 * 양극성을 회수하는 메소드
	 * bCanPolarize가 true라면 RedPolar--
	 */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	bool DepolarizeRed();

	/**
	 * 음극성을 회수하는 메소드
	 * bCanPolarize가 true라면 BluePolar--
	 */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	bool DepolarizeBlue();

	/**
	 * 극성간 차와 가중치를 통해 영향을 줄 힘을 계산하여 PolarityPower를 초기화한다.
	 * 극성간 차를 y = log2(x+1) 공식을 통해 계산하며 극성간 차가 5 이상 난다면 이를 5로 고정하여 계산한다.
	 * 결과값의 범위는 0 ~ 2330이며, 극성간 차가 1일 땐 0~900이다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	void CalcPolarityPower();

	/**
	 * 극성에 변화가 생기면 호출되는 이벤트
	 * Implementation에 정의된 기본형은 UpdateCustomDepthStencilValue()와 CalcPolarityPower()만 호출하고 있으며
	 * Implementation을 오버라이딩하여 사용하고 경우에 따라
	 * 블루프린트에서도 오버라이딩할 수 있도록하였다.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Polarity")
	void OnPolarityValueChanged();
	virtual void OnPolarityValueChanged_Implementation();

	/**
	 * RedPolar와 BluePolar를 비교하여 현재 극성 상태를 반환한다.
	 * @return 현재 극성 상태를 반환한다.
	 */
	UFUNCTION(BlueprintPure, Category = "Polarity")
	EPolarity GetPolarity();

	/**
	 * RedPolar와 BluePolar를 비교하여 현재 극성이 양극인지 확인한다.
	 * @return 양극이라면 True 반환
	 */
	UFUNCTION(BlueprintPure, Category = "Polarity")
	bool IsRed();

	/**
	 * RedPolar와 BluePolar를 비교하여 현재 극성이 음극인지 확인한다.
	 * @return 음극이라면 True 반환
	 */
	UFUNCTION(BlueprintPure, Category = "Polarity")
	bool IsBlue();

	/**
	 * RedPolar와 BluePolar를 비교하여 현재 극성이 중성인지 확인한다.
	 * @return 중성이라면 True 반환
	 */
	UFUNCTION(BlueprintPure, Category = "Polarity")
	bool IsNeutral();

	/**
	 * 극성값을 보유하고있지 않은 상태인지 확인한다.
	 * @return RedPolar, BluePolar 모두 0이라면 True 반환
	 */
	UFUNCTION(BlueprintPure, Category = "Polarity")
	bool IsEmpty();

	/**
	 * 자신의 극성과 Target의 극성이 반대인지 확인한다.
	 * @return Target과 극성이 반대라면 True를 반환
	 */
	UFUNCTION(BlueprintPure, Category = "Polarity")
	bool IsOppositePolarity(APolarityObject* Target);

	/**
	 * 자신의 극성과 Target의 극성이 같은지 확인한다.
	 * @param Target 자신과 극성을 비교할 대상
	 * @return Target과 극성이 같다면 True를 반환
	 */
	UFUNCTION(BlueprintPure, Category = "Polarity")
	bool IsSamePolarity(APolarityObject* Target);

	/**
	 * 극성을 강제로 정하는 메소드
	 * RedPolar와 BluePolar의 값을 수정하여 극성을 정한다.
	 * @param Polarity 세팅할 극성
	 * @param Factor 부여할 극성의 수치 (E_Neutral이라면 생략가능)
	 */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	void SetPolarity(EPolarity Polarity, int32 Factor = 1);

	/**
	 * PolarityMesh를 초기화한다.
	 * @param NewMesh 초기화할 MeshComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	void InitPolarMesh(UMeshComponent* NewMesh);

	/** @return PolarityMesh가 Valid하다면 True를 반환 */
	UFUNCTION(BlueprintPure, Category = "Polarity")
	bool PolarMeshIsValid();

	UFUNCTION(BlueprintPure, Category = "Polarity")
	UMeshComponent* GetPolarMesh();


	/**
	 * Polarity Mode가 켜지고 꺼질 때 설정해야할 옵션 값들을 조정한다.
	 * @param Value PFMode 활성화 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	void SetPFModeRenderingOption(bool Value);
	

	/**
	 * 타 Polarity 오브젝트와 상호작용을 정의하는 메소드이다. 기본적으로 매 틱마다 호출되며 극성에 따라 혹은  AffectedForce 값에 따라 취하게될 행동을 정의하면 된다.
	 * Implementation의 기본형은 빈 함수이며 오버라이딩하여 사용
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Polarity")
	void Interacting(float DeltaTime);
	virtual void Interacting_Implementation(float DeltaTime);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Polarity")
	void AffectAppointedGroup(float DeltaTime);
	virtual void AffectAppointedGroup_Implementation(float DeltaTime);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Polarity")
	void AffectPotentialGroup(float DeltaTime);
	virtual void AffectPotentialGroup_Implementation(float DeltaTime);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Polarity")
	void ApplyAffectedForce(float DeltaTime);
	virtual void ApplyAffectedForce_Implementation(float DeltaTime);

	/** @param Force 영향받는 힘 */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	void AddAffectedForce(FVector Force);

	/** 동적으로 움직이는 Polarity 오브젝트들에 의해 영향을 받은 힘의 값을 ZeroVector로 초기화한다. */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	void ClearAffectedForce();

	UFUNCTION(BlueprintCallable, Category = "Polarity")
	void AffectByDistance(APolarityObject* Target);

	/**
	 * 동적으로 움직이는 Polarity 오브젝트들에 의해 영향을 받은 힘의 값을 반환하고 ZeroVector로 초기화한다.
	 * @return 영향을 받은 Force 값을 반환
	 */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	FVector ConsumeAffectedForce();

	/**
	 * 자신이 Target에게 영향을 끼쳐 상호작용할 수 있는지 여부를 확인한다.
	 * 매 Tick마다 호출되어 확인된 오브젝트들의 AddAffectedForce()를 호출하여 영향을 끼친다.
	 * Implementation을 오버라이딩하여 사용하고 정의된 기본형은 무조건 false를 반환한다.
	 * @param Target 자신과 상호작용 여부를 확인할 대상
	 * @return Target에게 영향을 끼칠 수 있다면 True를 반환
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Polarity")
	bool CanAffect(APolarityObject* Target);
	virtual bool CanAffect_Implementation(APolarityObject* Target);

protected:
	/** 영향을 줄 수 있는 범위에 따라 PotentialGroup을 갱신한다. 오브젝트마다 랜덤한 주기로 호출된다. */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	void CheckPotentialGroup();

private:
	/**
	 * 극성이 변화할 때마다 PolarityMesh의 CustomDepthStencil 값을 조절하여
	 * Polarity Mode에서 특정한 색상을 띄도록 만든다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Polarity")
	void UpdateCustomDepthStencilValue();
};
