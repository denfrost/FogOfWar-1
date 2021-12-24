// Fill out your copyright notice in the Description page of Project Settings.


#include "FogOfWar/FogTexture.h"
#include "Engine/Texture2D.h"

FFogTexture::FFogTexture()
{
	OctantTransforms.Reserve(8);
	OctantTransforms.Emplace(FOctantTransform{  1,  0,  0,  1 });
	OctantTransforms.Emplace(FOctantTransform{  1,  0,  0, -1 });
	OctantTransforms.Emplace(FOctantTransform{ -1,  0,  0,  1 });
	OctantTransforms.Emplace(FOctantTransform{ -1,  0,  0, -1 });
	OctantTransforms.Emplace(FOctantTransform{  0,  1,  1,  0 });
	OctantTransforms.Emplace(FOctantTransform{  0,  1, -1,  0 });
	OctantTransforms.Emplace(FOctantTransform{  0, -1,  1,  0 });
	OctantTransforms.Emplace(FOctantTransform{  0, -1, -1,  0 });

	GenerateUpscaleMap();
}

FFogTexture::~FFogTexture()
{
	ReleaseFogTexture();
}

void FFogTexture::InitFogTexture(const uint32 Resolution)
{
	ReleaseFogTexture();

	uint32 TextureSize = FMath::RoundUpToPowerOfTwo(Resolution);

	// Create source buffer
	SourceWidth = TextureSize;
	SourceHeight = TextureSize;
	SourceBuffer = new uint8[SourceWidth * SourceHeight];
	SourceBufferSize = SourceWidth * SourceHeight * sizeof(uint8);
	FMemory::Memset(SourceBuffer, 0, SourceBufferSize);

	// Create upscale buffer
	UpscaleWidth = TextureSize * 4;
	UpscaleHeight = TextureSize * 4;
	UpscaleBuffer = new uint8[UpscaleWidth * UpscaleHeight];
	UpscaleBufferSize = UpscaleWidth * UpscaleHeight * sizeof(uint8);
	FMemory::Memset(UpscaleBuffer, 0, UpscaleBufferSize);
	UpscaleUpdateRegion = FUpdateTextureRegion2D(0, 0, 0, 0, UpscaleWidth, UpscaleHeight);

	// Create ExploredBuffer 
	ExploredBuffer = new uint8[UpscaleWidth * UpscaleHeight];
	FMemory::Memset(ExploredBuffer, 0, UpscaleBufferSize);

	// Create Fog Texture
	FogTexture = UTexture2D::CreateTransient(UpscaleWidth, UpscaleHeight, EPixelFormat::PF_G8);
	FogTexture->Filter = TextureFilter::TF_Nearest;
	FogTexture->CompressionSettings = TextureCompressionSettings::TC_Grayscale;
	FogTexture->AddressX = TextureAddress::TA_Clamp;
	FogTexture->AddressY = TextureAddress::TA_Clamp;
	FogTexture->SRGB = false;
	FogTexture->UpdateResource();
}

void FFogTexture::ReleaseFogTexture()
{
	if (FogTexture)
	{
		FogTexture = nullptr;
	}
	if (SourceBuffer)
	{
		delete[] SourceBuffer;
		SourceBuffer = nullptr;
	}
	if (UpscaleBuffer)
	{
		delete[] UpscaleBuffer;
		UpscaleBuffer = nullptr;
	}
	if (ExploredBuffer)
	{
		delete[] ExploredBuffer;
		ExploredBuffer = nullptr;
	}
}

void FFogTexture::UpdateExploredFog()
{
	for (uint32 i = 0; i < SourceBufferSize; ++i)
	{
		if (SourceBuffer[i] > 0)
		{
			SourceBuffer[i] = 0;
		}
	}

	for (uint32 i = 0; i < UpscaleBufferSize; ++i)
	{
		if (UpscaleBuffer[i] > 0)
		{
			ExploredBuffer[i] = ExploredFogColor;
		}
	}
}

void FFogTexture::UpdateFogBuffer(const FIntPoint& Center, int Radius, TFunction<bool(const FIntPoint&, const FIntPoint&)> IsBlocked)
{
	// Ray casting
	//DrawRayCastingFog(Center, Radius, IsBlocked);

	// Shadow casting
	SourceBuffer[Center.Y * SourceWidth + Center.X] = 0xFF;
	for (int i = 0; i < 8; ++i)
	{
		DrawShadowCastingFog(Center, Radius, 1, 1.0f, 0.0f, OctantTransforms[i], IsBlocked);
	}

	// Upscale buffer
	UpdateUpscaleBuffer();
}

void FFogTexture::UpdateFogTexture()
{
	FFogTextureContext* FogTextureContext = new FFogTextureContext();
	FogTextureContext->TextureResource = (FTexture2DResource*)FogTexture->Resource;
	FogTextureContext->MipIndex = FogTextureContext->TextureResource->GetCurrentFirstMip();
	FogTextureContext->UpdateRegion = &UpscaleUpdateRegion;
	FogTextureContext->SourcePitch = UpscaleUpdateRegion.Width;
	FogTextureContext->SourceData = new uint8[UpscaleBufferSize];
	FMemory::Memcpy(FogTextureContext->SourceData, UpscaleBuffer, UpscaleBufferSize);

	ENQUEUE_RENDER_COMMAND(UpdateTexture)([FogTextureContext](FRHICommandListImmediate& RHICmdList)
		{
			RHIUpdateTexture2D(FogTextureContext->TextureResource->GetTexture2DRHI(),
				FogTextureContext->MipIndex,
				*FogTextureContext->UpdateRegion,
				FogTextureContext->SourcePitch,
				FogTextureContext->SourceData);
		});
}

void FFogTexture::DrawRayCastingFog(const FIntPoint& Center, int Radius, TFunction<bool(const FIntPoint&, const FIntPoint&)> IsBlocked)
{
	if (Radius == 0)
	{
		return;
	}

	// Get bresenham circle
	int X = 0;
	int Y = Radius;
	int D = 1 - Radius; // Discriminant

	CastBresenhamLine(Center, Center + FIntPoint{  X,  Y }, IsBlocked);
	CastBresenhamLine(Center, Center + FIntPoint{  X, -Y }, IsBlocked);
	CastBresenhamLine(Center, Center + FIntPoint{  Y,  X }, IsBlocked);
	CastBresenhamLine(Center, Center + FIntPoint{ -Y,  X }, IsBlocked);

	for (X = 1; X < Y; ++X)
	{
		if (D <= 0)
		{
			D += 2 * X + 1;
		}
		else
		{
			D += 2 * X + 1 - 2 * Y;
			--Y;
		}
		CastBresenhamLine(Center, Center + FIntPoint{ X,  Y }, IsBlocked);
		CastBresenhamLine(Center, Center + FIntPoint{ -X,  Y }, IsBlocked);
		CastBresenhamLine(Center, Center + FIntPoint{ X, -Y }, IsBlocked);
		CastBresenhamLine(Center, Center + FIntPoint{ -X, -Y }, IsBlocked);
		CastBresenhamLine(Center, Center + FIntPoint{ Y,  X }, IsBlocked);
		CastBresenhamLine(Center, Center + FIntPoint{ -Y,  X }, IsBlocked);
		CastBresenhamLine(Center, Center + FIntPoint{ Y, -X }, IsBlocked);
		CastBresenhamLine(Center, Center + FIntPoint{ -Y, -X }, IsBlocked);
	}
}

void FFogTexture::CastBresenhamLine(const FIntPoint& Start, const FIntPoint& End, TFunction<bool(const FIntPoint&, const FIntPoint&)> IsBlocked)
{
	int X = Start.X;
	int Y = Start.Y;
	int DeltaX = FMath::Abs(End.X - Start.X);
	int DeltaY = FMath::Abs(End.Y - Start.Y);
	int XIncreasement = (End.X < Start.X) ? -1 : 1;
	int YIncreasement = (End.Y < Start.Y) ? -1 : 1;

	if (DeltaY < DeltaX)
	{
		int D = 2 * (DeltaY - DeltaX);

		for (; (Start.X < End.X ? X < End.X : X > End.X); X += XIncreasement)
		{
			if (0 >= D)
			{
				D += 2 * DeltaY;
			}
			else
			{
				D += 2 * DeltaY - 2 * DeltaX;
				Y += YIncreasement;
			}
			if (IsBlocked({ Start.X, Start.Y }, { X, Y }))
			{
				SourceBuffer[Y * SourceWidth + X] = 0xFF;
				break;
			}
			SourceBuffer[Y * SourceWidth + X] = 0xFF;
		}
	}
	else
	{
		int D = 2 * (DeltaX - DeltaY);

		for (; (Start.Y < End.Y ? Y < End.Y : Y > End.Y); Y += YIncreasement)
		{
			if (0 >= D)
			{
				D += 2 * DeltaX;
			}
			else
			{
				D += 2 * (DeltaX - DeltaY);
				X += XIncreasement;
			}
			if (IsBlocked({ Start.X, Start.Y }, { X, Y }))
			{
				SourceBuffer[Y * SourceWidth + X] = 0xFF;
				break;
			}
			SourceBuffer[Y * SourceWidth + X] = 0xFF;
		}
	}
}

void FFogTexture::DrawShadowCastingFog(const FIntPoint& Center, int Radius, int Row, float Start, float End, const FOctantTransform& T, TFunction<bool(const FIntPoint&, const FIntPoint&)> IsBlocked)
{
	if (Start < End)
	{
		return;
	}

	float NewStart = 0.0f;
	bool bBlocked = false;

	for (int Distance = Row; Distance <= Radius && bBlocked == false; ++Distance)
	{
		int DeltaY = -Distance;
		for (int DeltaX = -Distance; DeltaX <= 0; ++DeltaX)
		{
			int CurrentX = Center.X + DeltaX * T.XX + DeltaY * T.XY;
			int CurrentY = Center.Y + DeltaX * T.YX + DeltaY * T.YY;
			float LeftSlope = (DeltaX - 0.5f) / (DeltaY + 0.5f);
			float RightSlope = (DeltaX + 0.5f) / (DeltaY - 0.5f);

			if (((CurrentX >= 0 && CurrentY >= 0 && CurrentX < SourceWidth && CurrentY < SourceHeight) == false) 
				|| Start < RightSlope)
			{
				continue;
			}
			else if (End > LeftSlope)
			{
				break;
			}

			// Check if it's within the lightable area
			if (IsInRadius(Center, { CurrentX, CurrentY }, Radius))
			{
				SourceBuffer[CurrentY * SourceWidth + CurrentX] = 0xFF;
			}

			// Previous cell was a blocking one
			if (bBlocked)
			{
				if (IsBlocked(Center, { CurrentX, CurrentY }))
				{
					NewStart = RightSlope;
					continue;
				}
				else
				{
					bBlocked = false;
					Start = NewStart;
				}
			}
			else
			{
				if (IsBlocked(Center, { CurrentX, CurrentY }) && Distance < Radius)
				{
					bBlocked = true;
					DrawShadowCastingFog(Center, Radius, Distance + 1, Start, LeftSlope, T, IsBlocked);
					NewStart = RightSlope;
				}
			}
		}
	}
}

bool FFogTexture::IsInRadius(const FIntPoint& Center, const FIntPoint& Target, int Radius) const
{
	return (Target.X - Center.X) * (Target.X - Center.X) + (Target.Y - Center.Y) * (Target.Y - Center.Y) < Radius * Radius;
}

void FFogTexture::UpdateUpscaleBuffer()
{
	FTexel2X2 Texel2X2;
	FTexel4X4* Texel4X4;

	for (int i = 0; i < SourceWidth; ++i)
	{
		for (int j = 0; j < SourceHeight; ++j)
		{
			Texel2X2 = GetTexel2X2(i, j);
			Texel4X4 = UpscaleMap.Find(Texel2X2);
			if (Texel4X4 == nullptr)
			{
				continue;
			}

			for (int Row = 0; Row < 4; ++Row)
			{
				int Index = (Row + j * 4) * UpscaleWidth + i * 4;
				FMemory::Memcpy(&UpscaleBuffer[Index], &Texel4X4->T[Row], sizeof(uint8) * 4);
			}
		}
	}

	for (uint32 i = 0; i < UpscaleBufferSize; ++i)
	{
		if (ExploredBuffer[i] > 0 && UpscaleBuffer[i] == 0)
		{
			UpscaleBuffer[i] = ExploredFogColor;
		}
	}
}

FTexel2X2 FFogTexture::GetTexel2X2(int X, int Y)
{
	int NextX = FMath::Min(X + 1, SourceWidth  - 1);
	int NextY = FMath::Min(Y + 1, SourceHeight - 1);

	FTexel2X2 Texel;
	Texel.T[0] = SourceBuffer[Y		* SourceWidth + X];
	Texel.T[1] = SourceBuffer[Y		* SourceWidth + NextX];
	Texel.T[2] = SourceBuffer[NextY * SourceWidth + X];
	Texel.T[3] = SourceBuffer[NextY * SourceWidth + NextX];

	return Texel;
}

void FFogTexture::GenerateUpscaleMap()
{
	UpscaleMap.Reserve(16);

	UpscaleMap.Emplace(FTexel2X2{ 0, 0, 0, 0 }, FTexel4X4{ { { 0, 0, 0, 0 },
															 { 0, 0, 0, 0 },
															 { 0, 0, 0, 0 },
															 { 0, 0, 0, 0 } } });

	UpscaleMap.Emplace(FTexel2X2{ 0xFF, 0, 0, 0 }, FTexel4X4{ { { 0xFF, 0x04, 0,    0	 },
																{ 0x04, 0,	  0,    0	 },
																{ 0,	0,	  0,    0	 },
																{ 0,	0,	  0,    0	 } } });

	UpscaleMap.Emplace(FTexel2X2{ 0, 0xFF, 0, 0 }, FTexel4X4{ { { 0,    0,    0x04, 0xFF },
																{ 0,    0,    0,	0x04 },
																{ 0,    0,    0,	0	 },
																{ 0,    0,    0,	0	 } } });

	UpscaleMap.Emplace(FTexel2X2{ 0, 0, 0xFF, 0 }, FTexel4X4{ { { 0,	0,	  0,    0    },
																{ 0,	0,	  0,    0    },
																{ 0x04, 0,	  0,    0    },
																{ 0xFF, 0x04, 0,    0    } } });

	UpscaleMap.Emplace(FTexel2X2{ 0, 0, 0, 0xFF }, FTexel4X4{ { { 0,    0,    0,    0    },
															    { 0,    0,    0,    0    },
															    { 0,    0,    0,    0x04 },
															    { 0,    0,    0x04, 0xFF } } });

	UpscaleMap.Emplace(FTexel2X2{ 0xFF, 0xFF, 0, 0 }, FTexel4X4{ { { 0xFF, 0xFF, 0xFF, 0xFF },
																   { 0xFF, 0xFF, 0xFF, 0xFF },
																   { 0,    0,	 0,	   0	},
																   { 0,    0,	 0,	   0	} } });

	UpscaleMap.Emplace(FTexel2X2{ 0xFF, 0, 0xFF, 0 }, FTexel4X4{ { { 0xFF, 0xFF, 0, 0 },
																   { 0xFF, 0xFF, 0, 0 },
																   { 0xFF, 0xFF, 0, 0 },
																   { 0xFF, 0xFF, 0, 0 } } });

	UpscaleMap.Emplace(FTexel2X2{ 0xFF, 0, 0, 0xFF }, FTexel4X4{ { { 0xFF, 0x04, 0,	   0    },
																   { 0x04, 0,	 0,	   0    },
																   { 0,	   0,	 0,	   0x04 },
																   { 0,	   0,	 0x04, 0xFF } } });

	UpscaleMap.Emplace(FTexel2X2{ 0, 0xFF, 0xFF, 0 }, FTexel4X4{ { { 0,	   0,	 0x04, 0xFF },
																   { 0,	   0,	 0,	   0x04 },
																   { 0x04, 0,	 0,	   0    },
																   { 0xFF, 0x04, 0,	   0    } } });

	UpscaleMap.Emplace(FTexel2X2{ 0, 0xFF, 0, 0xFF }, FTexel4X4{ { { 0,	   0,	 0xFF, 0xFF },
																   { 0,	   0,	 0xFF, 0xFF },
																   { 0,	   0,	 0xFF, 0xFF },
																   { 0,	   0,	 0xFF, 0xFF } } });

	UpscaleMap.Emplace(FTexel2X2{ 0, 0, 0xFF, 0xFF }, FTexel4X4{ { { 0,	   0,	 0,	   0    },
																   { 0,	   0,	 0,	   0    },
																   { 0xFF, 0xFF, 0xFF, 0xFF },
																   { 0xFF, 0xFF, 0xFF, 0xFF } } });

	UpscaleMap.Emplace(FTexel2X2{ 0xFF, 0xFF, 0xFF, 0 }, FTexel4X4{ { { 0xFF, 0xFF, 0xFF, 0xFF },
																	  { 0xFF, 0xFF, 0xFF, 0xFF },
																	  { 0xFF, 0xFF, 0xFF, 0x04 },
																	  { 0xFF, 0xFF, 0x04, 0	   } } });

	UpscaleMap.Emplace(FTexel2X2{ 0xFF, 0xFF, 0, 0xFF }, FTexel4X4{ { { 0xFF, 0xFF, 0xFF, 0xFF },
																	  { 0xFF, 0xFF, 0xFF, 0xFF },
																	  { 0x04, 0xFF, 0xFF, 0xFF },
																	  { 0,	  0x04, 0xFF, 0xFF } } });

	UpscaleMap.Emplace(FTexel2X2{ 0xFF, 0, 0xFF, 0xFF }, FTexel4X4{ { { 0xFF, 0xFF, 0x04, 0	   },
																	  { 0xFF, 0xFF, 0xFF, 0x04 },
																	  { 0xFF, 0xFF, 0xFF, 0xFF },
																	  { 0xFF, 0xFF, 0xFF, 0xFF } } });

	UpscaleMap.Emplace(FTexel2X2{ 0, 0xFF, 0xFF, 0xFF }, FTexel4X4{ { { 0,	  0x04,	0xFF, 0xFF },
																	  { 0x04, 0xFF, 0xFF, 0xFF },
																	  { 0xFF, 0xFF, 0xFF, 0xFF },
																	  { 0xFF, 0xFF, 0xFF, 0xFF } } });

	UpscaleMap.Emplace(FTexel2X2{ 0xFF, 0xFF, 0xFF, 0xFF }, FTexel4X4{ { { 0xFF, 0xFF, 0xFF, 0xFF },
																	     { 0xFF, 0xFF, 0xFF, 0xFF },
																	     { 0xFF, 0xFF, 0xFF, 0xFF },
																	     { 0xFF, 0xFF, 0xFF, 0xFF } } });
}

uint32 GetTypeHash(const FTexel2X2& Texel)
{
	return Texel.T[0] * 1000 + Texel.T[1] * 100 + Texel.T[2] * 10 + Texel.T[3];
}
