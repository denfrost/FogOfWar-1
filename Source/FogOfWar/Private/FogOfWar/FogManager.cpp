// Fill out your copyright notice in the Description page of Project Settings.


#include "FogOfWar/FogManager.h"
#include "FogOfWar/FogTexture.h"

#include "TopDown/TopDownPlayerController.h"
#include "TopDown/TopDownUnit.h"

#include "Kismet/GameplayStatics.h"

AFogManager :: AFogManager ()
{
	PrimaryActorTick.bCanEverTick = true;
}

void  AFogManager::BeginPlay ()
{
	Super::BeginPlay ();

	if (GetWorld() == nullptr)
	{
		return;
	}

	// Get TopDownGrid
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass ( GetWorld (), ATopDownGrid::StaticClass (), OutActors);
	if (OutActors.Num() == 1)
	{
		TopDownGrid = Cast<ATopDownGrid>(OutActors[0]);
	}
	else
	{
		FString Name;
		GetName(Name);
		UE_LOG(LogTemp, Error, TEXT("%s: TopDownGrid must exist only one instance in world. Current instance is %d "), *Name, OutActors.Num());
		return;
	}

	// Get grid resoultion
	GridResolution = TopDownGrid-> GetGridResolution ();

	// Initialize FogTexture class
	FogTexture = new FFogTexture();
	FogTexture->InitFogTexture(GridResolution);

	// Set fog update timer
	GetWorldTimerManager().SetTimer(FogUpdateTimer, this, &AFogManager::UpdateFog, 1.0f / static_cast<float>(FogUpdateInterval), true, 0.5f);
}

void AFogManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Release FogTexture class
	FogTexture->ReleaseFogTexture();
	delete FogTexture;

	Super::EndPlay(EndPlayReason);
}

void AFogManager::UpdateFog()
{
	if (TopDownGrid == nullptr)
	{
		return;
	}

	// update the fog texture
	UpdateFogTexture();

	// update unit visibility according to fog
	Client_UpdateUnitVisibility();
}

void AFogManager::UpdateFogTexture()
{
	// Update if there are tiles that have been explored at least once
	FogTexture->UpdateExploredFog();
	
	// Get TopDownPC
	auto TopDownPC = Cast<ATopDownPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (TopDownPC == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(__FUNCTION__) + TEXT(": Invalid TopDownPC"));
		return;
	}

	// Calculate visibility only for units owned by TopDownPC
	for (auto Unit : TopDownPC->OwningUnits)
	{
		if (Unit == nullptr)
		{
			continue;
		}
		// Transform the unit's position into grid coordinates
		const FIntPoint& UnitCoords = TopDownGrid->WorldToGrid(Unit->GetActorLocation());

		// Get unit sight
		const int UnitSight = TopDownGrid->ToGridUnit(Unit->GetSight());
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Sight: %d, Coords: %s"), UnitSight, *UnitCoords.ToString()));
		// Update the fog texture buffer on a per-unit basis
		FogTexture-> UpdateFogBuffer (UnitCoords, UnitSight, TopDownGrid-> IsBlocked );
	}
	// update texture with buffer
	FogTexture->UpdateFogTexture();
}

void AFogManager::Client_UpdateUnitVisibility_Implementation()
{
	// Get TopDownPC
	auto TopDownPC = Cast<ATopDownPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (TopDownPC == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(__FUNCTION__) + TEXT(": Invalid TopDownPC"));
		return;
	}
	
	// Among units not owned by TopDownPC
	for (auto Unit : TopDownPC->OtherUnits)
	{
		// Check the fog conditions where the unit is and determine the unit's visibility.
		auto Coords = TopDownGrid->WorldToGrid(Unit->GetActorLocation());
		if (FogTexture->IsRevealed(Coords))
		{
			Unit->SetUnitVisibility(true);
		}
		else
		{
			Unit->SetUnitVisibility(false);
		}
	}
}

UTexture2D* AFogManager::GetFogTexture() const
{
	return FogTexture->FogTexture;
}
