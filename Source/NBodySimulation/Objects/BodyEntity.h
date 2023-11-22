// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NBodySimulation/Utils/BodySimulatorFunctionLibrary.h"
#include "UObject/Object.h"
#include "BodyEntity.generated.h"

class ABodySimulator;
/**
 *
 */
UCLASS()
class NBODYSIMULATION_API UBodyEntity : public UObject
{
	GENERATED_BODY()

public:
	UBodyEntity(): Index(0) {}

	virtual void Initialize(const FVector2D InPosition, const FVector2D InVelocity, const float InMass, const int32 InIndex)
	{
		Position = InPosition;
		Velocity = InVelocity;
		Mass = InMass;
		Index = InIndex;
	}

	FVector2D GetCenterOfMass() const
	{
		return Position * Mass;
	}

	FVector Get3DPosition() const
	{
		return UBodySimulatorFunctionLibrary::ConvertFVector2Dto3D(Position);
	}

	void TryToApplyExternalForce(FVector2D OtherPosition, float OtherMass, ABodySimulator* BodySimulator);

public:
	UPROPERTY()
	FVector2D Position;
	UPROPERTY()
	FVector2D Velocity;
	UPROPERTY()
	float Mass = 0;
	UPROPERTY()
	int32 Index;
};