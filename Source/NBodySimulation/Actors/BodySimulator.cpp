// Fill out your copyright notice in the Description page of Project Settings.


#include "BodySimulator.h"

#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NBodySimulation/Objects/BodyEntity.h"
#include "NBodySimulation/Objects/QuadTree.h"
#include "NBodySimulation/Utils/BodySimulatorFunctionLibrary.h"

constexpr float MAX_TICK = 0.0167; // to have stable simulation steps

// Sets default values
ABodySimulator::ABodySimulator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_DuringPhysics;
	InstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
	SetRootComponent(InstancedMesh);
	// Setup the simulations viewport
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArmComponent"));
	CameraArm->SetRelativeLocationAndRotation(FVector(1648.5f, 2500.0f, 0.0f), FRotator(-90.0f, 0.0f, 0.0f));
	CameraArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
	Camera->SetOrthoWidth(5000);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	SceneBounds = FBox2D({0, 0}, {3296, 5000});
	SetActorTickEnabled(false);
}

void ABodySimulator::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &ThisClass::InitBodies);
	//GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 1.0f, false);
	InitBodies();
}

void ABodySimulator::GetCameraValues()
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	FVector WorldDir;
	FVector WorldPosition;
	FVector2D ViewportSize = FVector2D(1, 1);

	GEngine->GameViewport->GetViewportSize(/*out*/ViewportSize);

	UGameplayStatics::DeprojectScreenToWorld(PlayerController, FVector2D(), WorldPosition, WorldDir);
	WorldPosition += WorldDir * CameraArm->TargetArmLength;

	BottomLeftBounds = FVector2D(WorldPosition.X, WorldPosition.Y);
	UGameplayStatics::DeprojectScreenToWorld(PlayerController, ViewportSize, WorldPosition, WorldDir);
	WorldPosition += WorldDir * CameraArm->TargetArmLength;
	//BottomRightBounds = FVector2D(WorldPosition.X, WorldPosition.Y);
	bCameraViewportReady = true;
	//SceneBounds = FBox2D({BottomRightBounds.X, TopLeftBounds.Y}, {TopLeftBounds.X, BottomRightBounds.Y});
}


// Called when the game starts or when spawned
void ABodySimulator::InitBodies()
{
	Bodies.SetNumUninitialized(BodyNum);
	Transforms.SetNumUninitialized(BodyNum);
	for (int32 Index = 0; Index < BodyNum; ++Index)
	{
		FVector2D RandomPosition(FMath::RandPointInCircle(PlacementRadius));
		RandomPosition += FVector2D(CameraArm->GetComponentLocation());
		const float RadialSpeedFactor = PlacementRadius / RandomPosition.Size();
		const FVector RandomVelocity = UKismetMathLibrary::RandomUnitVector() * BaseInitialSpeed * RadialSpeedFactor;
		const float Mass = FMath::FRandRange(MinMass, MaxMass);
		const float MeshScale = FMath::Sqrt(Mass) * BodyDisplayScale;
		FVector SpawnPosition = UBodySimulatorFunctionLibrary::ConvertFVector2Dto3D(RandomPosition);
		const FTransform MeshTransform(
			FRotator(), SpawnPosition
			,
			FVector(MeshScale, MeshScale, 1.0f)
		);

		Transforms[Index] = MeshTransform;
		Bodies[Index] = NewObject<UBodyEntity>(this);
		Bodies[Index]->Initialize(RandomPosition, FVector2D(RandomVelocity.X, RandomVelocity.Y), Mass, Index, MinimumGravityDistance, G);
	}
	InstancedMesh->AddInstances(Transforms, false, true);
	SetActorTickEnabled(true);
}


void ABodySimulator::GravityStep(const float DeltaTime)
{
	ParallelFor(Bodies.Num(), [&](const int32 Index)
	{
		FVector2D Acceleration(0.0f, 0.0f);
		for (const auto AffectingBody : Bodies)
		{
			if (AffectingBody->Index == Bodies[Index]->Index)
			{
				continue; // exclude self
			}
			float Distance = FVector2D::Distance(Bodies[Index]->Position, AffectingBody->Position);
			Distance = FMath::Max(Distance, MinimumGravityDistance);
			Acceleration += AffectingBody->Mass / Distance * G / Distance * (AffectingBody->Position - Bodies[Index]->Position) / Distance;
		}

		Bodies[Index]->Velocity += Acceleration * DeltaTime;
	});
}


void ABodySimulator::SimulateCompareAllParallel(const float DeltaTime)
{
	for (UBodyEntity* Body : Bodies)
	{
		AdjustPosition(Body->Position);
		Body->Position += Body->Velocity * DeltaTime;

		Transforms[Body->Index].SetTranslation(Body->Get3DPosition());
	}

	InstancedMesh->BatchUpdateInstancesTransforms(0, Transforms, true, true);
}


void ABodySimulator::AdjustPosition(FVector2D& InPosition) const
{
	//goes from to to negative size in X axis
	if (InPosition.X < SceneBounds.Min.X)
	{
		InPosition.X = SceneBounds.Max.X;
	}
	else if (InPosition.X > SceneBounds.Max.X)
	{
		InPosition.X = SceneBounds.Min.X;
	}
	if (InPosition.Y < SceneBounds.Min.Y)
	{
		InPosition.Y = SceneBounds.Max.Y;
	}
	else if (InPosition.Y > SceneBounds.Max.Y)
	{
		InPosition.Y = SceneBounds.Min.Y;
	}
}

void ABodySimulator::Tick(float DeltaTime)
{
	if (DeltaTime > MAX_TICK)
	{
		DeltaTime = MAX_TICK;
	}
	Super::Tick(DeltaTime);

	switch (SimulationType)
	{
	case ESimulationType::None:
		break;
	case ESimulationType::AllVsAll:
		SimulateNaiveMode(DeltaTime);
		SimulateCompareAllParallel(DeltaTime);
		break;
	case ESimulationType::BarnesHut:

		ConstructTree();
		if (bShowDebugGrid && QuadTree)
		{
			UKismetSystemLibrary::FlushDebugStrings(GetWorld());
			UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
			QuadTree->Show();
		}

		SimulateBarnesHut();
		break;
	default: ;
	}
	UE_LOG(LogTemp, Warning, TEXT("Number Uobjs: %d"), GUObjectArray.GetObjectArrayNum());
}

void ABodySimulator::SimulateNaiveMode(const float DeltaTime)
{
	GravityStep(DeltaTime);
}

void ABodySimulator::ForceDestroy(UQuadTree* QuadTreeToDelete)
{
	if (!QuadTreeToDelete)
	{
		return;
	}
	if (!QuadTreeToDelete->IsValidLowLevel())
	{
		return;
	}

	for (UQuadTree* Child : QuadTreeToDelete->Children)
	{
		ForceDestroy(Child);
	}

	//Begin Destroy
	QuadTreeToDelete->ConditionalBeginDestroy();
	QuadTreeToDelete = nullptr;
}


void ABodySimulator::ConstructTree()
{
	//we need to do this because tehre is a limit of uobjects in UE
	if (QuadTree)
	{
		ForceDestroy(QuadTree);
		//GC
		GEngine->ForceGarbageCollection();
	}
	QuadTree = NewObject<UQuadTree>(this);

	QuadTree->Box = SceneBounds;

	for (UBodyEntity* Body : Bodies)
	{
		QuadTree->Insert(Body);
	}
}

void ABodySimulator::SimulateBarnesHut()
{
	ParallelFor(Bodies.Num(), [&](const int32 Index)
	{
		CalculateForcesBarnesHut(Bodies[Index], QuadTree);

		AdjustPosition(Bodies[Index]->Position);
		Bodies[Index]->Position += Bodies[Index]->Velocity * GetWorld()->GetDeltaSeconds();

		Transforms[Bodies[Index]->Index].SetTranslation(Bodies[Index]->Get3DPosition());
	});
	/*for (int Index = 0; Index < Bodies.Num(); ++Index)
	{
		CalculateForcesBarnesHut(Bodies[Index], QuadTree);

		AdjustPosition(Bodies[Index]->Position);
		Bodies[Index]->Position += Bodies[Index]->Velocity * GetWorld()->GetDeltaSeconds();

		Transforms[Bodies[Index]->Index].SetTranslation(Bodies[Index]->Get3DPosition());
	} */

	InstancedMesh->BatchUpdateInstancesTransforms(0, Transforms, true, true);
}

void ABodySimulator::CalculateForcesBarnesHut(UBodyEntity* BodyEntity, UQuadTree* Node)
{
	if (Node == nullptr)
	{
		return;
	}
	if (Node->IsLeaf() && Node->BodyEntity)
	{
		if (Node->BodyEntity->Index == BodyEntity->Index)
		{
			return;
		}

		BodyEntity->TryToApplyExternalForce(Node->BodyEntity->Position, Node->BodyEntity->Mass, GetWorld()->GetDeltaSeconds());
	}
	FVector2D Center, Extents;
	Node->Box.GetCenterAndExtents(Center, Extents);
	const float s = Extents.GetMax();
	const float d = FVector2D::Distance(Node->CenterMass, BodyEntity->Position);

	const float Quotient = s / d;
	//UE_LOG(LogTemp,Log,TEXT("s/d %f/%f=%f < %f θ "),s,d,Quotient,Theta);
	if (Quotient < Theta)
	{
		//UE_LOG(LogTemp, Log, TEXT("Skipped because TestValue< theta"));
		BodyEntity->TryToApplyExternalForce(Node->CenterMass, Node->Mass, GetWorld()->GetDeltaSeconds());
		return;
	}
	for (UQuadTree* Child : Node->Children)
	{
		CalculateForcesBarnesHut(BodyEntity, Child);
	}
}

void ABodySimulator::OnViewportResized(FViewport* Viewport, unsigned I)
{
	GetCameraValues();
}