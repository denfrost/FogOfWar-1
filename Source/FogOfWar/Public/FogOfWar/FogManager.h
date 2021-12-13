// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TopDown/TopDownGrid.h"
#include "FogManager.generated.h"

class UFogAgentComponent;

UCLASS()
class FOGOFWAR_API AFogManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFogManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void AddFogAgent(UFogAgentComponent* const FogAgent);
	void RemoveFogAgent(UFogAgentComponent* const FogAgent);
	void UpdateFogAgents();

	/** https://en.wikipedia.org/wiki/Midpoint_circle_algorithm */
	void GetBresenhamCircle(const FIntPoint& Center, int Radius, int AgentHeight);

	void GetBresenhamLine(const FIntPoint& Start, const FIntPoint& End);

	void GetCircleArea(const FIntPoint& Center, int Radius);

protected:
	UPROPERTY(Category = "Config", EditAnywhere, BlueprintReadWrite)
	bool bDebugTile = false;

	UPROPERTY(Category = "Fog Manager", BlueprintReadOnly)
	class ATopDownGrid* TopDownGrid = nullptr;

	UPROPERTY(Category = "Fog Manager", BlueprintReadOnly)
	TArray<UFogAgentComponent*> FogAgents;

	/** Tiles that need to investigate */
	UPROPERTY()
	TArray<FIntPoint> CachedTiles;
	UPROPERTY()
	TArray<FIntPoint> CustomCircles;

	/** Tiles that finish investigation */
	UPROPERTY()
	TArray<FIntPoint> Visibles;

	/** Tiles that finish investigation */
	UPROPERTY()
	TArray<FIntPoint> Obstacles;

private:
	void DrawDebugTile(float Duration);

	void AddTileCoords(const FIntPoint& TileCoords, int AgentHeight);
};
