// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuadTree.generated.h"

/*
USTRUCT()
struct FBodyEntity
{
	GENERATED_BODY()

	UPROPERTY()
	FVector2D Position;
	UPROPERTY()
	FVector2D Velocity;
	UPROPERTY()
	float Mass = 0;
	UPROPERTY()
	int32 Index;
};*/

class UBodyEntity;
/**
 *
 */
UCLASS()
class NBODYSIMULATION_API UQuadTree : public UObject
{
	GENERATED_BODY()
	UQuadTree();


	virtual void SubDivide();

public:
	virtual void Initialize(const FBox2D& InBox);
	virtual bool Insert(UBodyEntity* Entity);
	virtual void Show();
	virtual FVector2D GetCenterOfMass();
	virtual bool IsLeaf();
	virtual bool IsEmpty();

public:
	UPROPERTY(BlueprintReadOnly)
	FBox2D Box;

	UPROPERTY()
	bool bIsSubdivided;

	UPROPERTY()
	bool bIsEmpty = true;

	UPROPERTY()
	UBodyEntity* BodyEntity;

	UPROPERTY(BlueprintReadOnly)
	TArray<UQuadTree*> Children;

	UPROPERTY()
	float Mass = 0;

	UPROPERTY()
	FVector2D CenterMass;

	UPROPERTY()
	UQuadTree* TopLeft;

	UPROPERTY()
	UQuadTree* TopRight;

	UPROPERTY()
	UQuadTree* BottomLeft;

	UPROPERTY()
	UQuadTree* BottomRight;
};