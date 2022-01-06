// Fill out your copyright notice in the Description page of Project Settings.


#include "FogOfWar/FogManager.h"
#include "FogOfWar/FogTexture.h"

#include "TopDown/TopDownPlayerController.h"
#include "TopDown/TopDownUnit.h"
#include "TopDown/TopDownGameState.h"

#include "Kismet/GameplayStatics.h"
#include "Rendering/Texture2DResource.h"
#include "DrawDebugHelpers.h"

AFogManager::AFogManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFogManager::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld() == nullptr)
	{
		return;
	}

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATopDownGrid::StaticClass(), OutActors);
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
	GridResolution = TopDownGrid->GetGridResolution();

	FogTexture = new FFogTexture();
	FogTexture->InitFogTexture(GridResolution);

	GetWorldTimerManager().SetTimer(FogUpdateTimer, this, &AFogManager::UpdateFog, 1.0f / static_cast<float>(FogUpdateInterval), true, 0.5f);
}

void AFogManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FogTexture->ReleaseFogTexture();
	delete FogTexture;

	Super::EndPlay(EndPlayReason);
}

void AFogManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AFogManager::UpdateFog()
{
	if (TopDownGrid == nullptr)
	{
		return;
	}

	// �Ȱ� �ؽ�ó ������Ʈ
	UpdateFogTexture();

	// �Ȱ��� ���� ���� ���ü� ������Ʈ
	UpdateUnitVisibility();
}

void AFogManager::UpdateFogTexture()
{
	FogTexture->UpdateExploredFog();

	// PC�� ������ ���ֵ鸸 �þ� ���
	auto TopDownPC = Cast<ATopDownPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (TopDownPC == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Invalid TopDownPC"));
		return;
	}
	for (auto Unit : TopDownPC->OwningUnits)
	{
		if (Unit == nullptr)
		{
			continue;
		}

		const FIntPoint& UnitCoords = TopDownGrid->WorldToGrid(Unit->GetActorLocation());
		const int UnitSight = TopDownGrid->ToGridUnit(Unit->Sight);
		FogTexture->UpdateFogBuffer(UnitCoords, UnitSight, TopDownGrid->IsBlocked);
	}

	FogTexture->UpdateFogTexture();
}

void AFogManager::UpdateUnitVisibility()
{
	// Get TopDownGS
	auto TopDownGS = Cast<ATopDownGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (TopDownGS == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Invalid TopDownGS"));
		return;
	}

	// Get TopDownPC
	auto TopDownPC = Cast<ATopDownPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (TopDownPC == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Invalid TopDownPC"));
		return;
	}

	// ��� TopDownUnit ��ȸ
	for (auto Unit : TopDownGS->AllUnits)
	{
		if (Unit == nullptr)
		{
			continue;
		}

		// PC�� ���� �� �����̸� ���ü� �˻� ����
		if (Unit->GetGenericTeamId() == TopDownPC->TeamId)
		{
			continue;
		}

		// �ٸ� �� �����̸� �� ������ �׸��� ��ǥ�� Ȯ���Ͽ� ������ ���ü� ����
		const FIntPoint& UnitCoords = TopDownGrid->WorldToGrid(Unit->GetActorLocation());
		if (FogTexture->IsRevealed(UnitCoords))
		{
			//TopDownGS->Client_SetUnitHiddenInGame(Unit, false);
		}
		else
		{
			//TopDownGS->Client_SetUnitHiddenInGame(Unit, false);
		}
	}
}

UTexture2D* AFogManager::GetFogTexture() const
{
	return FogTexture->FogTexture;
}
