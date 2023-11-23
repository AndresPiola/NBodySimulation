// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NBodySimulation/Utils/BodySimulatorFunctionLibrary.h"
#include "UObject/Object.h"
#include "BodyEntity.generated.h"


UCLASS()
class NBODYSIMULATION_API UBodyEntity : public UObject
{
	GENERATED_BODY()

public:
	UBodyEntity(): Index(0) {}

	virtual void Initialize(const FVector2D InPosition, const FVector2D InVelocity, const float InMass, const int32 InIndex,
	                        const float InMinimumGravityDistance = 100.0f,
	                        const float InG = 100.0f)
	{
		Position = InPosition;
		Velocity = InVelocity;
		Mass = InMass;
		Index = InIndex;
		MinimumGravityDistance = InMinimumGravityDistance;
		G = InG;
	}

	FVector2D GetCenterOfMass() const
	{
		return Position * Mass;
	}

	FVector Get3DPosition() const
	{
		return UBodySimulatorFunctionLibrary::ConvertFVector2Dto3D(Position);
	}

	void TryToApplyExternalForce(FVector2D OtherPosition, float OtherMass, float DeltaSeconds)
	{
		FVector2D Acceleration(0.0f, 0.0f);
		float Distance = FVector2D::Distance(Position, OtherPosition);
		Distance = FMath::Max(Distance, MinimumGravityDistance); // avoids division by zero
		Acceleration += OtherMass / Distance * G / Distance * (OtherPosition - Position) / Distance;

		Velocity += Acceleration * DeltaSeconds;
	}

public:
	UPROPERTY()
	float MinimumGravityDistance = 100.0f;

	UPROPERTY()
	float G = 1000.0f;

	UPROPERTY()
	FVector2D Position;

	UPROPERTY()
	FVector2D Velocity;

	UPROPERTY()
	float Mass = 0;

	UPROPERTY()
	int32 Index;
};