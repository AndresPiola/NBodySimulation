// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BodyEntity.h"
#include "UObject/Object.h"
#include "QuadTree.generated.h"


struct FBodyEntity;
class UBodyEntity;
/**
 *
 */
UCLASS()
class NBODYSIMULATION_API UQuadTree : public UObject
{
	GENERATED_BODY()
	UQuadTree(): bIsSubdivided(false) {}


	virtual void SubDivide();

public:
	virtual void Initialize(const FBox2D& InBox);
	virtual bool Insert(UBodyEntity* Entity);
	virtual void Show();
	virtual FVector2D GetCenterOfMass();
	virtual bool IsLeaf();
	virtual bool IsEmpty();
	virtual void Reset();
	virtual bool IsActive() { return bIsActive; }
	virtual void SetActive() { bIsActive = true; }
	void DeactivateFast();

public:
	UPROPERTY(BlueprintReadOnly)
	FBox2D Box;

	UPROPERTY()
	UBodyEntity* BodyEntity;

	UPROPERTY(BlueprintReadOnly)
	TArray<UQuadTree*> Children;

	UPROPERTY()
	float Mass = 0;

	UPROPERTY()
	FVector2D CenterMass;

private:
	UPROPERTY()
	bool bIsSubdivided = false;

	UPROPERTY()
	bool bIsEmpty = true;

	UPROPERTY()
	bool bIsActive = false;
};