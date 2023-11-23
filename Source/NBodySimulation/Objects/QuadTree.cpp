// Fill out your copyright notice in the Description page of Project Settings.


#include "QuadTree.h"


#include "BodyEntity.h"


void UQuadTree::SubDivide()
{
	FVector2D Center;
	FVector2D Size;
	Box.GetCenterAndExtents(Center, Size);
	Children.Reserve(4);
	Children.Insert(new UQuadTree(FBox2D({Center.X, Center.Y - Size.Y}, {Center.X + Size.X, Center.Y})), 0);
	Children.Insert(new UQuadTree(FBox2D({Center.X, Center.Y}, {Center.X + Size.X, Center.Y + Size.Y})), 1);
	Children.Insert(new UQuadTree(FBox2D({Center.X - Size.X, Center.Y - Size.Y}, {Center.X, Center.Y})), 2);
	Children.Insert(new UQuadTree(FBox2D({Center.X - Size.X, Center.Y}, {Center.X, Center.Y + Size.Y})), 3);
}

bool UQuadTree::Insert(UBodyEntity* Entity)
{
	if (!Box.IsInside(Entity->Position))
	{
		return false;
	}
	if (IsEmpty() && !HasChildren())
	{
		BodyEntity = Entity;
		Mass = BodyEntity->Mass;
		CenterMass = BodyEntity->Position;
		bIsEmpty = false;
		return true;
	}

	if (!bIsSubdivided)
	{
		bIsSubdivided = true;
		SubDivide();
		for (UQuadTree* Child : Children)
		{
			if (Child->Insert(BodyEntity))
			{
				break;
			}
		}
	}
	const bool bInserted = Children[0]->Insert(Entity) ||
		Children[1]->Insert(Entity) ||
		Children[2]->Insert(Entity) ||
		Children[3]->Insert(Entity);
	float TempMass = 0;
	float CenterX = 0;
	float CenterY = 0;
	int Size = 0;

	for (const UQuadTree* Child : Children)
	{
		if (!Child->BodyEntity)
		{
			continue;
		}
		Size++;
		TempMass += Child->Mass;
		CenterX += Child->CenterMass.X;
		CenterY += Child->CenterMass.Y;
	}
	Mass = TempMass;
	CenterMass.X = CenterX / Size;
	CenterMass.Y = CenterY / Size;

	return bInserted;
}

FVector2D UQuadTree::GetCenterOfMass()
{
	if (IsLeaf() && BodyEntity)
	{
		return BodyEntity->GetCenterOfMass();
	}
	return CenterMass;
}

float UQuadTree::GetMass()
{
	if (IsLeaf() && BodyEntity)
	{
		return BodyEntity->Mass;
	}
	return Mass;
}

bool UQuadTree::IsLeaf()
{
	if (!HasChildren())
	{
		return true;
	}
	return Children[0] == nullptr && Children[1] == nullptr && Children[2] == nullptr && Children[3] == nullptr;
}

bool UQuadTree::IsEmpty()
{
	return bIsEmpty;
}

/*
void UQuadTree::Reset()
{
	bIsActive = false;
	Box = FBox2D();
	BodyEntity = nullptr;
	for (auto Child : Children)
	{
		if (Child)
		{
			Child->Reset();
		}
	}
	Children.Empty();
	Mass = 0;
	CenterMass = FVector2D();
	bIsSubdivided = false;
	bIsEmpty = true;
}

void UQuadTree::DeactivateFast()
{
	bIsActive = false;
	Box = FBox2D();
	BodyEntity = nullptr;

	Children.Empty();
	Mass = 0;
	CenterMass = FVector2D();
	bIsSubdivided = false;
	bIsEmpty = true;
}*/