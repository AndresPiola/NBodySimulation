// Fill out your copyright notice in the Description page of Project Settings.


#include "BodySimulator.h"

#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NBodySimulation/Objects/BodyEntity.h"
#include "NBodySimulation/Objects/QuadTree.h"
#include "NBodySimulation/Utils/BodySimulatorFunctionLibrary.h"


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
	/*hardcoded with magic numbers, I could adjust this with viewport calculation but
	 * it is not required or important right now
	 */

	SceneBounds = FBox2D({0, 0}, {3296, 5000});
	SetActorTickEnabled(false);
}

void ABodySimulator::BeginPlay()
{
	Super::BeginPlay();
	/* If I would like to calculate viewport size so I could get the exact SceneBounds I should add a code here
	  the timer is because UE gets the wrong viewport Size at the beginning, because according to UE docs
	we must wait until HUD is ready
	*/
	/*FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &ThisClass::InitBodies);
	 GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 1.0f, false);*/
	InitBodies();
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


void ABodySimulator::SimulateCompareAllParallel(const float DeltaTime)
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


void ABodySimulator::MoveAllBodies(const float DeltaTime)
{
	bool bSkipBody = false;
	for (UBodyEntity* Body : Bodies)
	{
		AdjustPosition(Body->Position);
		Body->Position += Body->Velocity * DeltaTime;
		Transforms[Body->Index].SetTranslation(Body->Get3DPosition());
		if (Transforms[Body->Index].ContainsNaN())
		{
			bSkipBody = true;
		}
	}
	if (!bSkipBody)
	{
		InstancedMesh->BatchUpdateInstancesTransforms(0, Transforms, true, true);
	}
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

void ABodySimulator::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (SimulationType)
	{
	case ESimulationType::None:
		break;
	case ESimulationType::AllVsAll:
		SimulateCompareAllParallel(DeltaTime);
		MoveAllBodies(DeltaTime);
		break;
	case ESimulationType::BarnesHut:
		ConstructTree();
		SimulateBarnesHut(DeltaTime);
		MoveAllBodies(DeltaTime);
		break;
	}
}


void ABodySimulator::ConstructTree()
{
	delete QuadTree;
	QuadTree = new UQuadTree(SceneBounds);

	for (int Index = 0; Index < Bodies.Num(); ++Index)
	{
		QuadTree->Insert(Bodies[Index]);
	}
}

void ABodySimulator::SimulateBarnesHut(const float DeltaTime)
{
	ParallelFor(Bodies.Num(), [&](const int32 Index)
	{
		CalculateForcesBarnesHut(Bodies[Index], QuadTree, DeltaTime);
	});
}

void ABodySimulator::CalculateForcesBarnesHut(UBodyEntity* BodyEntity, UQuadTree* Node, float DeltaTime)
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
		BodyEntity->TryToApplyExternalForce(Node->GetCenterOfMass(), Node->GetMass(), DeltaTime);
	}
	if (Node->IsEmpty())
	{
		return;
	}
	FVector2D Center, Extents;
	Node->Box.GetCenterAndExtents(Center, Extents);
	//I could join this but i left this apart for better visual readability
	const float s = Extents.GetMax();
	const float d = FVector2D::Distance(Node->GetCenterOfMass(), BodyEntity->Position);

	if (const float Quotient = s / d; Quotient < Theta)
	{
		if (Node->GetMass() > 0)
		{
			BodyEntity->TryToApplyExternalForce(Node->GetCenterOfMass(), Node->GetMass(), DeltaTime);
		}

		return;
	}
	for (UQuadTree* Child : Node->Children)
	{
		CalculateForcesBarnesHut(BodyEntity, Child, DeltaTime);
	}
}