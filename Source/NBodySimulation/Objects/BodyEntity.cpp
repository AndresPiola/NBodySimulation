// Fill out your copyright notice in the Description page of Project Settings.


#include "BodyEntity.h"

#include "NBodySimulation/Actors/BodySimulator.h"

void UBodyEntity::TryToApplyExternalForce(FVector2D OtherPosition, float OtherMass, ABodySimulator* BodySimulator)
{
	FVector2D Acceleration(0.0f, 0.0f);
	float Distance = FVector2D::Distance(Position, OtherPosition);
	Distance = FMath::Max(Distance, BodySimulator->MinimumGravityDistance); // avoids division by zero
	Acceleration += OtherMass / Distance * BodySimulator->G / Distance * (OtherPosition - Position) / Distance;

	Velocity += Acceleration * GetWorld()->GetDeltaSeconds();
}