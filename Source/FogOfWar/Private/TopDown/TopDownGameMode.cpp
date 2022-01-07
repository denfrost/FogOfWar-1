// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDown/TopDownGameMode.h"
#include "TopDown/TopDownPlayerController.h"
#include "TopDown/TopDownGameInstance.h"

void ATopDownGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// ���� ����
	if (HasAuthority())
	{
		AllPlayerController.Add(NewPlayer);

		// Get TopDownPC
		auto TopDownPC = Cast<ATopDownPlayerController>(NewPlayer);
		if (TopDownPC == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(__FUNCTION__) + TEXT(": Invalid TopDownPC"));
			return;
		}

		// TopDownPC �ʱ�ȭ
		TopDownPC->Client_InitializeTopDownPC();
	}
}

void ATopDownGameMode::BeginPlay()
{
	Super::BeginPlay();
}
