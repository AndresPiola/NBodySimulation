// Fill out your copyright notice in the Description page of Project Settings.


#include "QuadTree.h"


#include "BodyEntity.h"
#include "NBodySimulation/Utils/BodySimulatorFunctionLibrary.h"

UQuadTree::UQuadTree(): bIsSubdivided(false), TopLeft(nullptr), TopRight(nullptr), BottomLeft(nullptr), BottomRight(nullptr) {}


void UQuadTree::SubDivide()
{
	UE::Math::TVector2<double> Center = Box.GetCenter();
	const UE::Math::TVector2<double> Size = Box.GetExtent();
	this->TopLeft = NewObject<UQuadTree>(this);
	this->TopLeft->Initialize(FBox2D({Center.X, Center.Y - Size.Y}, {Center.X + Size.X, Center.Y}));
	this->TopRight = NewObject<UQuadTree>(this);
	this->TopRight->Initialize(FBox2D({Center.X, Center.Y}, {Center.X + Size.X, Center.Y + Size.Y}));
	this->BottomLeft = NewObject<UQuadTree>(this);
	this->BottomLeft->Initialize(FBox2D({Center.X - Size.X, Center.Y - Size.Y}, {Center.X, Center.Y}));
	this->BottomRight = NewObject<UQuadTree>(this);
	this->BottomRight->Initialize(FBox2D({Center.X - Size.X, Center.Y}, {Center.X, Center.Y + Size.Y}));
	Children.Add(this->TopLeft);
	Children.Add(this->TopRight);
	Children.Add(this->BottomLeft);
	Children.Add(this->BottomRight);
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

		TopLeft->Insert(BodyEntity);
		TopRight->Insert(BodyEntity);
		BottomLeft->Insert(BodyEntity);
		BottomRight->Insert(BodyEntity);
	}
	const bool bInserted = TopLeft->Insert(Entity) ||
		TopRight->Insert(Entity) ||
		BottomLeft->Insert(Entity) ||
		BottomRight->Insert(Entity);

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
	return TopLeft == nullptr && TopRight == nullptr && BottomLeft == nullptr && BottomRight == nullptr;
}

bool UQuadTree::IsEmpty()
{
	return bIsEmpty;
	return BodyEntity == nullptr;
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
		TopLeft->Show();
		TopRight->Show();
		BottomLeft->Show();
		BottomRight->Show();
	}
}