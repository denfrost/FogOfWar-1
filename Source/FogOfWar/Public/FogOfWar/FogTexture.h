// Fill out your copyright notice in the Description page of Project Settings.

# pragma once

#include "CoreMinimal.h"
#include "Rendering/Texture2DResource.h"

struct FTexel2X2
{
	bool operator==(const FTexel2X2& Rhs) const
	{
		return T[0] == Rhs.T[0] && T[1] == Rhs.T[1] && T[2] == Rhs.T[2] && T[3] == Rhs.T[3];
	}
	uint8 T[4];
};
uint32 GetTypeHash ( const FTexel2X2& Texel); // custom hash function to use TMap

struct FTexel4X4
{
	uint8 T[4][4];
};

class FOGOFWAR_API FFogTexture
{
	struct FUpdateTextureContext
	{
		FTexture2DResource* TextureResource = nullptr;
		uint32 MipIndex = 0;
		FUpdateTextureRegion2D* UpdateRegion = nullptr;
		uint32 SourcePitch = 0;
		uint8* SourceData = nullptr;
	};

	struct FOctantTransform
	{
		int8 XX, XY, YX, YY;
	};

	static constexpr uint8 RevealedFogColor = 0xFF;
	static constexpr uint8 ExploredFogColor = 0x20;

public:
	FFogTexture();
	~FFogTexture();

	void InitFogTexture(const uint32 Resolution);

	void ReleaseFogTexture();

	void UpdateExploredFog();

	void UpdateFogBuffer(const FIntPoint& Center, int Radius, TFunction<bool(const FIntPoint&, const FIntPoint&)> IsBlocked);

	void  UpdateFogTexture ();

	/* * @return Tells if the tile is in Revealed state. */
	UFUNCTION(Category = "Fog Texture", BlueprintPure)
	bool  IsRevealed ( const FIntPoint & Coords) const ;

	/* * Texture2D created with upscale buffer */
	UPROPERTY(Category = "Fog Texture", BlueprintReadOnly, Transient)
	class UTexture2D* FogTexture = nullptr;

private:
	/* * https://technology.riotgames.com/news/story-fog-and-war */
	void GenerateUpscaleMap();
	void UpdateUpscaleBuffer();

	// Ray casting: A method of inspecting obstacles by drawing a straight line from the center to the tile corresponding to the circumference of the circle.
	/* * Function to find the tile corresponding to the circumference of a circle using the Bresenham circle algorithm
	* https://en.wikipedia.org/wiki/Midpoint_circle_algorithm */
	void DrawRayCastingFog(const FIntPoint& Center, int Radius, TFunction<bool(const FIntPoint&, const FIntPoint&)> IsBlocked);
	/* * Check for obstacles in a straight line with the Bresenham line algorithm
	* https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm */
	void  CastBresenhamLine ( const FIntPoint & Start, const FIntPoint & End, TFunction < bool ( const FIntPoint &, const FIntPoint &)> IsBlocked);

	// Shadow casting
	/* * http://roguebasin.com/index.php/Shadow_casting */
	void DrawShadowCastingFog(const FIntPoint& Center, int Radius, int Row, float Start, float End, const FOctantTransform& T, TFunction<bool(const FIntPoint&, const FIntPoint&)> IsBlocked);

	/* * Make sure the Target is in a circle centered on the Center. */
	bool IsInRadius(const FIntPoint& Center, const FIntPoint& Target, int Radius) const;

	/* * Get (X,Y), (X+1,Y), (X,Y+1), (X+1,Y+1) texels. */
	FTexel2X2 GetTexel2X2(int X, int Y);

	/* * Buffer to update the fog state in conjunction with the tilemap. */
	uint8* SourceBuffer = nullptr;
	uint32 SourceBufferSize = 0;
	int SourceWidth = 0;
	int SourceHeight = 0;

	/* * This is a 4x magnification of the SourceBuffer. */
	uint8* UpscaleBuffer = nullptr;
	uint32 UpscaleBufferSize = 0;
	int UpscaleWidth = 0;
	int UpscaleHeight = 0;
	FUpdateTextureRegion2D UpscaleUpdateRegion;

	/* * Used to make the explored tile gray. */
	uint8* ExploredBuffer = nullptr;

	/* * Transform structure to apply the shadow casting algorithm to all quadrants. */
	UPROPERTY ()
	TArray <FOctantTransform> OctantTransforms;

	/* * This is a template that stores how to map 2X2 texels to 4X4 texels. */
	UPROPERTY ()
	TMap<FTexel2X2, FTexel4X4> UpscaleMap;
};
