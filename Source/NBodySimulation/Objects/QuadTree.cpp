// Fill out your copyright notice in the Description page of Project Settings.


#include "QuadTree.h"


#include "BodyEntity.h"
#include "NBodySimulation/Utils/BodySimulatorFunctionLibrary.h"

UQuadTree::UQuadTree(): bIsSubdivided(false), BodyEntity(nullptr) {}

void UQuadTree::SubDivide()
{
	Children.Reserve(4);
	UE::Math::TVector2<double> Center = Box.GetCenter();
	const UE::Math::TVector2<double> Size = Box.GetExtent();
	Children.Insert(NewObject<UQuadTree>(this), 0);
	Children[0]->Initialize(FBox2D({Center.X, Center.Y - Size.Y}, {Center.X + Size.X, Center.Y}));
	Children.Insert(NewObject<UQuadTree>(this), 1);
	Children[1]->Initialize(FBox2D({Center.X, Center.Y}, {Center.X + Size.X, Center.Y + Size.Y}));
	Children.Insert(NewObject<UQuadTree>(this), 2);
	Children[2]->Initialize(FBox2D({Center.X - Size.X, Center.Y - Size.Y}, {Center.X, Center.Y}));
	Children.Insert(NewObject<UQuadTree>(this), 3);
	Children[3]->Initialize(FBox2D({Center.X - Size.X, Center.Y}, {Center.X, Center.Y + Size.Y}));
}

void UQuadTree::Initialize(const FBox2D& InBox)
{
	Box = InBox;
}

bool UQuadTree::Insert(UBodyEntity* Entity)
{
	//	UE_LOG(LogTemp, Log, TEXT("Trying to insert Body id %d  "), Entity->Index);
	if (!Box.IsInside(Entity->Position))
	{
		//		UE_LOG(LogTemp, Log, TEXT(" Body id %d not in cuadrant "), Entity->Index);
		return false;
	}
	if (IsEmpty() && Children.Num() == 0)
	{
		BodyEntity = Entity;

		Mass = BodyEntity->Mass;
		CenterMass = BodyEntity->Position;
		bIsEmpty = false;
		//	UE_LOG(LogTemp, Log, TEXT("Leaf Node %s mass: %f CenterMass:%s"), *GetName(), Mass, *CenterMass.ToString());
		return true;
	}

	if (!bIsSubdivided)
	{
		bIsSubdivided = true;
		SubDivide();
		//UE_LOG(LogTemp, Log, TEXT(" Node %s is subdivided"), *GetName());
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
	if (BodyEntity)
	{
		return BodyEntity->GetCenterOfMass();
	}
	return FVector2D::Zero();
}

bool UQuadTree::IsLeaf()
{
	if (Children.Num() == 0)
	{
		return true;
	}
	return Children[0] == nullptr && Children[1] == nullptr && Children[2] == nullptr && Children[3] == nullptr;
}

bool UQuadTree::IsEmpty()
{
	return bIsEmpty;
}

void UQuadTree::Show()
{
	const FColor Color = bIsSubdivided ? FColor::Green : FColor::Magenta;
	const FVector QuadCenter = UBodySimulatorFunctionLibrary::ConvertFVector2Dto3D(Box.GetCenter());
	DrawDebugBox(GetWorld(), QuadCenter,
	             UBodySimulatorFunctionLibrary::ConvertFVector2Dto3D(Box.GetExtent()),
	             Color);
	if (Mass < .1f)
	{
		return;
	}
	if (IsLeaf())
	{
		DrawDebugSphere(GetWorld(), UBodySimulatorFunctionLibrary::ConvertFVector2Dto3D(CenterMass), 22, 11, FColor::Blue);
		UE_LOG(LogTemp, Log, TEXT("LeafNode %s mass: %f CenterMass:%s"), *GetName(), Mass, *CenterMass.ToString());
	}
	else
	{
		const FVector CenterMass3D = UBodySimulatorFunctionLibrary::ConvertFVector2Dto3D(CenterMass);
		DrawDebugSphere(GetWorld(), CenterMass3D, 22, 11, FColor::Orange);
		UE_LOG(LogTemp, Log, TEXT("Node %s mass: %f CenterMass:%s"), *GetName(), Mass, *CenterMass.ToString());
		DrawDebugLine(GetWorld(), QuadCenter, CenterMass3D, FColor::Orange);
	}

	if (Children.Num() > 0 || IsLeaf())
	{
		const FString DebugText = FString::Printf(TEXT(" m: %f"), Mass);
		UE_LOG(LogTemp, Log, TEXT("%s"), *DebugText);

		DrawDebugString(GetWorld(), UBodySimulatorFunctionLibrary::ConvertFVector2Dto3D(Box.GetCenter()), DebugText);
	}

	if (bIsSubdivided)
	{
		for (UQuadTree* Child : Children)
		{
			if (Child)
			{
				Child->Show();
			}
		}
	}
}