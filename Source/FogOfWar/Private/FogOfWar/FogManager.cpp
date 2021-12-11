// Fill out your copyright notice in the Description page of Project Settings.


#include "FogOfWar/FogManager.h"
#include "FogOfWar/FogAgentComponent.h"
#include "TopDown/TopDownGrid.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values
AFogManager::AFogManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFogManager::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATopDownGrid::StaticClass(), OutActors);
	if (OutActors.Num() == 1)
	{
		TopDownGrid = Cast<ATopDownGrid>(OutActors[0]);

		FString Name;
		GetName(Name);
		UE_LOG(LogTemp, Log, TEXT("%s: Successfully get TopDownGrid."), *Name);
	}
	else
	{
		FString Name;
		GetName(Name);
		UE_LOG(LogTemp, Error, TEXT("%s: TopDownGrid must exist only one instance in world. Current instance is %d "), *Name, OutActors.Num());
	}
}

// Called every frame
void AFogManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateFogAgents();

	if (bDebugTile)
	{
		DrawDebugTile(FColor::Silver, DeltaTime * 2.0f);
	}
}

void AFogManager::AddFogAgent(UFogAgentComponent* const FogAgent)
{
	if (FogAgent)
	{
		FogAgents.Add(FogAgent);

		FString Name;
		GetName(Name);
		UE_LOG(LogTemp, Log, TEXT("%s: Added fog agent"), *Name);

	}
	else
	{
		FString Name;
		GetName(Name);
		UE_LOG(LogTemp, Warning, TEXT("%s: Invalid fog agent"), *Name);
	}
}

void AFogManager::RemoveFogAgent(UFogAgentComponent* const FogAgent)
{
	if (FogAgent)
	{
		FogAgents.Remove(FogAgent);

		FString Name;
		GetName(Name);
		UE_LOG(LogTemp, Log, TEXT("%s: Removed fog agent"), *Name);
	}
}

void AFogManager::UpdateFogAgents()
{
	if (TopDownGrid)
	{
		CachedCoords.Reset(CachedCoords.GetSlack());

		for (auto Agent : FogAgents)
		{
			if (Agent)
			{
				const FIntPoint& Coords = TopDownGrid->WorldToGrid(Agent->GetFogAgentLocation());
				GetBresenhamCircle(Coords, Agent->Sight);
				//GetCircleArea(Coords, Agent->Sight);
			}
		}
	}
}

void AFogManager::GetBresenhamCircle(const FIntPoint& Center, int Radius)
{
	int X = 0;
	int Y = Radius;
	int D = 1 - Radius;

	CachedCoords.AddUnique(Center + FIntPoint{  X,  Y });
	CachedCoords.AddUnique(Center + FIntPoint{  X, -Y });
	CachedCoords.AddUnique(Center + FIntPoint{  Y,  X });
	CachedCoords.AddUnique(Center + FIntPoint{ -Y,  X });

	for (X = 1; X < Y; ++X)
	{
		if (D < 0)
		{
			D += 2 * X + 1;
		}
		else
		{
			D += 2 * X + 1 - 2 * Y;
			--Y;

			for (int i = 0; i < X; ++i)
			{
				CachedCoords.AddUnique(Center + FIntPoint{  i,  Y });
				CachedCoords.AddUnique(Center + FIntPoint{ -i,  Y });
				CachedCoords.AddUnique(Center + FIntPoint{  i, -Y });
				CachedCoords.AddUnique(Center + FIntPoint{ -i, -Y });
				CachedCoords.AddUnique(Center + FIntPoint{  Y,  i });
				CachedCoords.AddUnique(Center + FIntPoint{ -Y,  i });
				CachedCoords.AddUnique(Center + FIntPoint{  Y, -i });
				CachedCoords.AddUnique(Center + FIntPoint{ -Y, -i });
			}
		}

		CachedCoords.AddUnique(Center + FIntPoint{  X,  Y });
		CachedCoords.AddUnique(Center + FIntPoint{ -X,  Y });
		CachedCoords.AddUnique(Center + FIntPoint{  X, -Y });
		CachedCoords.AddUnique(Center + FIntPoint{ -X, -Y });
		CachedCoords.AddUnique(Center + FIntPoint{  Y,  X });
		CachedCoords.AddUnique(Center + FIntPoint{ -Y,  X });
		CachedCoords.AddUnique(Center + FIntPoint{  Y, -X });
		CachedCoords.AddUnique(Center + FIntPoint{ -Y, -X });
	}

	for (int i = Center.X - X + 1; i < Center.X + X; ++i)
	{
		for (int j = Center.Y - Y + 1; j < Center.Y + Y; ++j)
		{
			CachedCoords.AddUnique(FIntPoint{ i, j });
		}
	}
}

void AFogManager::DrawBresenhamLine(const FIntPoint& Start, const FIntPoint& End)
{
	int X = Start.X;
	int Y = Start.Y;

	int DeltaX = FMath::Abs(End.X - Start.X);
	int DeltaY = FMath::Abs(End.Y - Start.Y);

	int XIncreasement = (End.X < Start.X) ? -1 : 1;
	int YIncreasement = (End.Y < Start.Y) ? -1 : 1;

	if (DeltaY < DeltaX)
	{
		int Discriminant = 2 * (DeltaY - DeltaX);

		for (; (Start.X < End.X ? X < End.X : X > End.X); X += XIncreasement)
		{
			if (0 >= Discriminant)
			{
				Discriminant += 2 * DeltaY;
			}
			else
			{
				Discriminant += 2 * (DeltaY - DeltaX);
				Y += YIncreasement;
			}
			CachedCoords.AddUnique({ X, Y });
		}
	}
	else
	{
		int Discriminant = 2 * (DeltaX - DeltaY);

		for (; (Start.Y < End.Y ? Y < End.Y : Y > End.Y); Y += YIncreasement)
		{
			if (0 >= Discriminant)
			{
				Discriminant += 2 * DeltaX;
			}
			else
			{
				Discriminant += 2 * (DeltaX - DeltaY);
				X += XIncreasement;
			}
			CachedCoords.AddUnique({ X, Y });
		}
	}
}

void AFogManager::GetCircleArea(const FIntPoint& Center, int Radius)
{
	// 1. Center�� �������� Extent�� Radius�� �簢�� Ÿ���� ���Ѵ�.
	// 2. for X in �簢�� :
	// 2-1. ���� Ÿ�Ϻ��� Center���� �Ÿ� D�� ����Ѵ�.
	// 2-2. D <= Radius�� ���߰� ���� ��ġ L�� �����Ѵ�.
	// 2-3. ������ Ÿ�Ϻ��� Center���� �Ÿ� D�� ����Ѵ�.
	// 2-4. D <= Radius�� ���߰� ���� ��ġ R�� �����Ѵ�.
	// 2-5. (X, L)���� (X, R)������ Ÿ���� ���� �ش��ϴ� Ÿ���̴�.

	// ^ X-Axis
	// |
	// |
	// |- - - > Y-Axis
	const int Left = Center.Y - Radius + 1;		
	const int Right = Center.Y + Radius - 1;	
	const int Top = Center.X + Radius - 1;		
	const int Bottom = Center.X - Radius + 1;	

	const float RadiuSquared = Radius * Radius;		
	
	for (int X = Bottom; X <= Top; ++X)
	{
		int LeftY = Left;
		float Dist = FVector2D::DistSquared(Center, FIntPoint{ X, LeftY });
		while (Dist > RadiuSquared)
		{
			++LeftY;
			Dist = FVector2D::DistSquared(Center, FIntPoint{ X, LeftY });
		}
		
		int RightY = Right;
		Dist = FVector2D::DistSquared(Center, FIntPoint{ X, RightY });
		while (Dist > RadiuSquared)
		{
			--RightY;
			Dist = FVector2D::DistSquared(Center, FIntPoint{ X, RightY });
		}

		for (int i = LeftY; i <= RightY; ++i)
		{
			CachedCoords.AddUnique({ X, i });
		}
	}
}

void AFogManager::DrawDebugTile(const FColor& Color, float Duration)
{
	for (const auto& Coords : CachedCoords)
	{
		if (TopDownGrid)
		{
			FTile* Tile = TopDownGrid->TileData.Find(Coords);
			if (Tile)
			{
				FVector Location = Tile->WorldLocation;
				Location.Z += 1.0f;
				DrawDebugBox(GetWorld(), Location, TopDownGrid->GetTileExtent() * 0.95f, Color, false, Duration);
			}
		}
	}
}
