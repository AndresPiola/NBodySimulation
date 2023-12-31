﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NBodySimulation/Objects/BodyEntity.h"
#include "NBodySimulation/Objects/QuadTree.h"
#include "BodySimulator.generated.h"


class UBodyEntity;
class UCameraComponent;
class USpringArmComponent;

UENUM(BlueprintType)
enum class ESimulationType:uint8
{
	None,
	AllVsAll,
	BarnesHut
};

UCLASS()
class NBODYSIMULATION_API ABodySimulator : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABodySimulator();

	virtual void InitBodies();
	/*for the naive version we need to adjust inside of the move code*/
	virtual void MoveAllBodies(float DeltaTime, bool bSkipAdjust = true);
	virtual void AdjustPosition(FVector2D& InPosition) const;

	virtual void ConstructTree();
	/*this method uses naive aprox, tha means compare everyone vs everyone O(n¨2)*/
	virtual void SimulateCompareAllParallel(float DeltaTime);
	/*this method uses quadtree and should be O(nlog(n))*/
	virtual void SimulateBarnesHut(float DeltaTime);
	virtual void CalculateForcesBarnesHut(UBodyEntity* BodyEntity, UQuadTree* Node, float DeltaTime);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, NoClear)
	UInstancedStaticMeshComponent* InstancedMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USpringArmComponent* CameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation")
	ESimulationType SimulationType = ESimulationType::BarnesHut;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation")
	int BodyNum = 1500;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation")
	float PlacementRadius = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation")
	float BaseInitialSpeed = 500.0f;

	UPROPERTY(EditAnywhere, meta=(UIMin=0, UIMax=1.5f), Category = "NBody Simulation")
	float Theta = 0.7f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation")
	float BodyDisplayScale = 0.02f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation")
	float G = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation")
	float MinMass = 20.0f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation")
	float MaxMass = 100.0f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation")
	float MinimumGravityDistance = 100.0f;

	UPROPERTY(BlueprintReadOnly)
	FBox2D SceneBounds;

private:
	UPROPERTY()
	bool bCameraViewportReady;

	UPROPERTY()
	TArray<UBodyEntity*> Bodies;

	UPROPERTY()
	TArray<FTransform> Transforms;

	UQuadTree* QuadTree;
};