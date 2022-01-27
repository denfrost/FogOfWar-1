// Fill out your copyright notice in the Description page of Project Settings.

# pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TopDownGrid.generated.h"

UENUM(BlueprintType)
enum class ETileHeight : uint8
{
	INVALID = 0,
	Lowest,
	Low,
	Medium,
	High,
	Highest
};

USTRUCT(Atomic, BlueprintType)
struct FTile
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FVector WorldLocation;

	UPROPERTY(BlueprintReadWrite)
	ETileHeight Height;
};

UCLASS()
class FOGOFWAR_API ATopDownGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	/** Sets default values for this actor's properties */
	ATopDownGrid();

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;

public:
	/* * @return Convert world coordinates to grid coordinates. */
	UFUNCTION(Category = "Top Down Grid", BlueprintPure)
	FIntPoint WorldToGrid(const FVector& WorldLocation) const;
	
	UFUNCTION(Category = "Top Down Grid", BlueprintPure)
	FVector GetTileExtent () const ;

	UFUNCTION(Category = "Top Down Grid", BlueprintPure)
	int GetGridResolution() const;

	UFUNCTION(Category = "Top Down Grid", BlueprintPure)
	int GetGridVolumeExtentXY() const;

	UFUNCTION(Category = "Top Down Grid", BlueprintPure)
	int GetGridVolumeExtentZ() const;

	/* * @return Convert world units to grid units. */
	int  ToGridUnit ( const  int N) const ;

	UPROPERTY(Category = "Top Down Grid", BlueprintReadOnly)
	TMap<FIntPoint, FTile> TileData;

	/* * Callback function to compare the height of the Center tile and the Target tile */
	TFunction < bool ( const FIntPoint &, const FIntPoint &)> IsBlocked;

private:
	void UpdateGridTransform();

	/** @return Returns blocking hit */
	bool CoordsLineTraceToMinusZAxis(const FIntPoint& Coords, ETraceTypeQuery TraceChannel, FHitResult& OutHit);

	/* * Create a tile map */
	void  GenerateTileData ();

	void DrawDebugGrid();

	UPROPERTY(Category = "Config", EditAnywhere)
	bool bDebugGrid = false;

	UPROPERTY(Category = "Config", EditAnywhere)
	float DebugGridTime = 10.0f;

	/* * Number of tiles to fit into grid width & number of texels for fog texture width
	* Tile extent = GridVolumeExtentXY / GridResoulution */
	UPROPERTY(Category = "Config", EditAnywhere, meta = (ClampMin = 64, ClampMax = 2048, UIMin = 64, UIMax = 2048)) //!!!  amount tiles 
	int GridResolution = 512;

	UPROPERTY(Category = "Config", EditAnywhere)
	int GridVolumeExtentXY = 4096;

	UPROPERTY(Category = "Config", EditAnywhere)
	int GridVolumeExtentZ = 1024;

	UPROPERTY(Category = "Top Down Grid", VisibleDefaultsOnly)
	class UBillboardComponent* Billboard = nullptr;

	UPROPERTY(Category = "Top Down Grid", VisibleDefaultsOnly)
	class UBoxComponent* GridVolume = nullptr;

	/* * Transform of grid coordinates. Scale based on GridVolume */
	FTransform GridTransform;
	
	/* * Adjustment value to set the minimum value of grid coordinates to (0, 0) */
	int GridShift = GridResolution / 2;

	/* * Half the width of the tile. If 50, tile with side length of 100 */
	FVector TileExtent;
};
