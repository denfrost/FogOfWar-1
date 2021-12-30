// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
uint32 GetTypeHash(const FTexel2X2& Texel); // TMap�� ����ϱ� ���� Ŀ���� �ؽ� �Լ�

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

	static constexpr uint8 ExploredFogColor = 0x20;

public:
	FFogTexture();
	~FFogTexture();

	void InitFogTexture(const uint32 Resolution);

	void ReleaseFogTexture();

	void UpdateExploredFog();

	void UpdateFogBuffer(const FIntPoint& Center, int Radius, TFunction<bool(const FIntPoint&, const FIntPoint&)> IsBlocked);

	void UpdateFogTexture();

	bool IsRevealed(const FIntPoint& Coords);

	/** �������� ���۷� ������ �ؽ�ó�Դϴ�. */
	UPROPERTY(Category = "Fog Texture", BlueprintReadOnly, Transient)
	class UTexture2D* FogTexture = nullptr;

private:
	/** https://technology.riotgames.com/sites/default/files/fow_diagram.png */
	void GenerateUpscaleMap();

	void UpdateUpscaleBuffer();

	/** https://en.wikipedia.org/wiki/Midpoint_circle_algorithm */
	void DrawRayCastingFog(const FIntPoint& Center, int Radius, TFunction<bool(const FIntPoint&, const FIntPoint&)> IsBlocked);
	/** https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm */
	void CastBresenhamLine(const FIntPoint& Start, const FIntPoint& End, TFunction<bool(const FIntPoint&, const FIntPoint&)> IsBlocked);

	/*
	* http://www.roguebasin.com/index.php?title=Improved_Shadowcasting_in_Java
	* https://www.albertford.com/shadowcasting/
	* https://m.blog.naver.com/dunkydonk/220214116723
	*/
	void DrawShadowCastingFog(const FIntPoint& Center, int Radius, int Row, float Start, float End, const FOctantTransform& T, TFunction<bool(const FIntPoint&, const FIntPoint&)> IsBlocked);

	/** Target�� Center�� �߽����� �ϴ� �� �ȿ� �ִ��� Ȯ���մϴ�. */
	bool IsInRadius(const FIntPoint& Center, const FIntPoint& Target, int Radius) const;

	/** (X,Y), (X+1,Y), (X,Y+1), (X+1,Y+1) �ؼ��� �����ɴϴ�. */
	FTexel2X2 GetTexel2X2(int X, int Y);

	/** Ÿ�ϸʰ� �����Ͽ� �Ȱ� ���¸� ������Ʈ�ϴ� �����Դϴ�. */
	uint8* SourceBuffer = nullptr;
	uint32 SourceBufferSize = 0;
	int SourceWidth = 0;
	int SourceHeight = 0;

	/** SourceBuffer�� 4�� Ȯ���� �����Դϴ�. */
	uint8* UpscaleBuffer = nullptr;
	uint32 UpscaleBufferSize = 0;
	int UpscaleWidth = 0;
	int UpscaleHeight = 0;
	FUpdateTextureRegion2D UpscaleUpdateRegion;

	/** Ž���ߴ� Ÿ���� ȸ������ ����µ� ����մϴ�. */
	uint8* ExploredBuffer = nullptr;

	/** ��� 8�и鿡 ���� ĳ���� �˰����� �����ϱ� ���� ��ȯ ����ü�Դϴ�. */
	UPROPERTY()
	TArray<FOctantTransform> OctantTransforms;

	/** 2X2 �ؼ��� 4X4 �ؼ��� ��� �������� ������ ���� ���ø��Դϴ�. */
	UPROPERTY()
	TMap<FTexel2X2, FTexel4X4> UpscaleMap;
};
