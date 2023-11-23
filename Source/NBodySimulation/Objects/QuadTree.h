﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BodyEntity.h"
#include "UObject/Object.h"


class UBodyEntity;
/**
 *
 */

class UQuadTree
{
public:
	UQuadTree(): BodyEntity(nullptr), Children{}, bIsSubdivided(false) {}


	virtual void SubDivide();


	virtual ~UQuadTree()
	{
		for (int i = 0; i < 4; i++)
		{
			if (Children[i] != nullptr)
			{
				delete Children[i];
				Children[i] = nullptr;
			}
		}
	};
	FORCEINLINE bool HasChildren()
	{
		for (const UQuadTree* Child : Children)
		{
			if (Child != nullptr)
			{
				return true;
			}
		}
		return false;
	}

	virtual void Initialize(const FBox2D& InBox);

	virtual bool Insert(UBodyEntity* Entity);

	virtual FVector2D GetCenterOfMass();
	virtual float GetMass();
	virtual bool IsLeaf();
	virtual bool IsEmpty();

public:
	FBox2D Box;
	UBodyEntity* BodyEntity;
	TArray<UQuadTree*> Children;

private:
	bool bIsSubdivided = false;
	bool bIsEmpty = true;
	float Mass = 0;
	FVector2D CenterMass;
};