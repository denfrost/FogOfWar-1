// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDown/TopDownUnit.h"
#include "TopDown/TopDownPlayerController.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
ATopDownUnit::ATopDownUnit()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Selection = CreateDefaultSubobject<UDecalComponent>("Selection Decal");
	Selection->SetupAttachment(RootComponent);
	Selection->DecalSize = FVector(32.0f, 64.0f, 64.0f);
	Selection->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());
	Selection->SetRelativeLocation(FVector(0, 0, -90.0f));
	Selection->SetVisibility(false);
}

FGenericTeamId ATopDownUnit::GetGenericTeamId() const
{
	return (FGenericTeamId)TeamId;
}

void ATopDownUnit::SetSelect_Implementation(bool NewSelect)
{
	Selection->SetVisibility(NewSelect);
}

bool ATopDownUnit::IsFriendly_Implementation() const
{
	// Get TopDownPC
	auto TopDownPC = GetWorld()->GetFirstPlayerController<ATopDownPlayerController>();
	if (TopDownPC == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(__FUNCTION__) + TEXT(" - Invalid TopDownPC"));
		return false;
	}

	// TopDownPC�� TeamId�� ������ �Ʊ��Դϴ�.
	return TopDownPC->TeamId == TeamId;
}

bool ATopDownUnit::IsHostile_Implementation() const
{
	// Get TopDownPC
	auto TopDownPC = GetWorld()->GetFirstPlayerController<ATopDownPlayerController>();
	if (TopDownPC == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(__FUNCTION__) + TEXT(" - Invalid TopDownPC"));
		return true;
	}

	// TopDownPC�� TeamId�� �ٸ��� �����Դϴ�.
	return TopDownPC->TeamId != TeamId;
}

void ATopDownUnit::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	// ������ ���콺 ���� ��ư���� Ŭ���ϸ�
	if (ButtonPressed == EKeys::LeftMouseButton)
	{
		// Get TopDownPC
		auto TopDownPC = GetWorld()->GetFirstPlayerController<ATopDownPlayerController>();
		if (TopDownPC == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(__FUNCTION__) + TEXT(" - Invalid TopDownPC"));
		}

		// TopDownPC�� ���� ���� ���� ������ ���������մϴ�.
		TopDownPC->ClearSelectedActors();
		
		// TopDownPC�� ���� ������ Ȯ���մϴ�.
		if (ITopDownUnitInterface::Execute_IsHostile(this))
		{
			Client_SelectUnit(FLinearColor::Red);
		}
		if (ITopDownUnitInterface::Execute_IsFriendly(this))
		{
			Client_SelectUnit(FLinearColor::Green);
		}

		// TopDownPC�� ���� ���� ������ �ڽ����� �����մϴ�.
		TopDownPC->SelectedActors.Add(this);
	}
}

void ATopDownUnit::Client_SelectUnit_Implementation(const FLinearColor& Color)
{
	SelectionDMI->SetVectorParameterValue(TEXT("Color"), Color);
	ITopDownUnitInterface::Execute_SetSelect(this, true);
}

int ATopDownUnit::GetSight() const
{
	return Sight;
}
