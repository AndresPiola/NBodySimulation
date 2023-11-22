// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BodySimulatorFunctionLibrary.generated.h"

/**
 *
 */
UCLASS()
class NBODYSIMULATION_API UBodySimulatorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static void GetBoxBounds(const FBox2D& Box2D, FVector2D& Center, FVector2D& Extents);

	UFUNCTION(BlueprintPure)
	static FVector ConvertFVector2Dto3D(const FVector2D& XYCoordinates);
};