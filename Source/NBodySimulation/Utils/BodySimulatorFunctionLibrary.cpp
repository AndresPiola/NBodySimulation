// Fill out your copyright notice in the Description page of Project Settings.


#include "BodySimulatorFunctionLibrary.h"

void UBodySimulatorFunctionLibrary::GetBoxBounds(const FBox2D& Box2D, FVector2D& Center, FVector2D& Extents)
{
	Box2D.GetCenterAndExtents(Center, Extents);
}

FVector UBodySimulatorFunctionLibrary::ConvertFVector2Dto3D(const FVector2D& XYCoordinates)
{
	return FVector(XYCoordinates.X, XYCoordinates.Y, 0.0f);
}