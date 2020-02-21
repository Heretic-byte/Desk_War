


#include "TestPhysics.h"
#include "Datas/USB_Macros.h"

// Sets default values
ATestPhysics::ATestPhysics()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_fTimer = 0.f;
	OnCalculateCustomPhysics.BindUObject(this, &ATestPhysics::SubstepTick);
}

// Called when the game starts or when spawned
void ATestPhysics::BeginPlay()
{
	Super::BeginPlay();
	UWorld* World = GetWorld();
	if (World)
	{
		FPhysScene* PScene = World->GetPhysicsScene();
		if (PScene)
		{
			OnPhysSceneStepHandle = PScene->OnPhysSceneStep.AddUObject(this, &ATestPhysics::PhysSceneStep);
		}
	}
}

void ATestPhysics::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FPhysScene* PScene = World->GetPhysicsScene();
		if (PScene)
		{
			PScene->OnPhysSceneStep.Remove(OnPhysSceneStepHandle);
			//PScene->OnPhysScenePostTick
		}
	}
	Super::EndPlay(EndPlayReason);

}

// Called every frame
void ATestPhysics::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_StopPrint)
	{
		return;
	}

	if (m_Primi == nullptr)
		return;

	if (m_Primi->GetBodyInstance() == nullptr)
		return;
	m_Primi->GetBodyInstance()->AddCustomPhysics(OnCalculateCustomPhysics);
	m_fTimer += DeltaTime;
}

void ATestPhysics::PrintStopWatch()
{
	if (m_StopPrint)
	{
		return;
	}
	m_StopPrint = true;
	PRINTF("[TimeSpent] : %f",m_fTimer);
}

void ATestPhysics::SubstepTick(float DeltaTime, FBodyInstance * BodyInstance)
{
	if (m_StopPrint)
	{
		return;
	}

	//PRINTF("[SubStepTime] : %f", DeltaTime);
}

void ATestPhysics::PhysSceneStep(FPhysScene * PhysScene, float DeltaTime)
{
	if (m_StopPrint)
	{
		return;
	}

	//PRINTF("[PhysSceneSubStepTime] : %f", DeltaTime);
}

