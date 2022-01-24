// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDown/TopDownPlayerController.h"
#include "TopDown/TopDownCamera.h"
#include "TopDown/TopDownHUD.h"
#include "TopDown/TopDownUnit.h"
#include "TopDown/TopDownGameInstance.h"
#include "Kismet/GameplayStatics.h"

void ATopDownPlayerController::BeginPlay()
{
	Super::BeginPlay();

	TopDownCamera = GetPawn<ATopDownCamera>();

	FInputModeGameAndUI InputMode = {};
	InputMode.SetHideCursorDuringCapture(false);
	//InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(InputMode);
}

void ATopDownPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (TopDownCamera)
	{
		EdgeLocation = CheckEdgeMovement();
	}
}

void ATopDownPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("LeftMouseButton", IE_Pressed, this, &ATopDownPlayerController::OnLeftMouseButtonPressed);
	InputComponent->BindAction("LeftMouseButton", IE_Released, this, &ATopDownPlayerController::OnLeftMouseButtonReleased);
}

void ATopDownPlayerController::OnLeftMouseButtonPressed()
{
	auto TopDownHUD = GetHUD<ATopDownHUD>();
	if (TopDownHUD == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(__FUNCTION__) + TEXT(": Invalid TopDownHUD"));
		return;
	}
	TopDownHUD->BeginDrawRect();


	if (TopDownCamera == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(__FUNCTION__) + TEXT(": Invalid TopDownCamera"));
		return;
	}
	TopDownCamera->DisableInput(this);
}

void ATopDownPlayerController::OnLeftMouseButtonReleased()
{
	auto TopDownHUD = GetHUD<ATopDownHUD>();
	if (TopDownHUD == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(__FUNCTION__) + TEXT(": Invalid TopDownHUD"));
		return;
	}
	TopDownHUD->EndDrawRect();


	if (TopDownCamera == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(__FUNCTION__) + TEXT(": Invalid TopDownCamera"));
		return;
	}
	TopDownCamera->EnableInput(this);
}

void ATopDownPlayerController::Client_InitializeTopDownPC_Implementation()
{
	// Get TopDownGameInstance
	auto TopDownGameInst = GetGameInstance<UTopDownGameInstance>();
	if (TopDownGameInst->IsValidLowLevel() == false)
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, FString(__FUNCTION__) + TEXT(": Invalid TopDownGameInst"));
		return;
	}

	// TeamId 업데이트
	TeamId = TopDownGameInst->PlayerInfo.TeamId;

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATopDownUnit::StaticClass(), OutActors);

	// Get OwningUnit
	for (auto Actor : OutActors)
	{
		auto Unit = Cast<ATopDownUnit>(Actor);
		if (Unit->IsValidLowLevel() == false)
		{
			continue;
		}

		if (IsOwningUnit(Unit))
		{
			OwningUnits.Add(Unit);
		}
		else
		{
			OtherUnits.Add(Unit);
		}
	}
}

void ATopDownPlayerController::ClearSelectedActors()
{
	for (auto Actor : SelectedActors)
	{
		if (Actor->GetClass()->ImplementsInterface(UTopDownUnitInterface::StaticClass()))
		{
			ITopDownUnitInterface::Execute_SetSelect(Actor, false);
		}
	}
	SelectedActors.Empty(SelectedActors.GetSlack());
}

EEdgeLocation ATopDownPlayerController::GetMouseEdgeLocation() const
{
    return EdgeLocation;
}

bool ATopDownPlayerController::IsOwningUnit(AActor* const Actor) const
{
	auto Unit = Cast<ATopDownUnit>(Actor);
	if (Unit)
	{
		return Unit->GetGenericTeamId() == TeamId;
	}
	return false;
}

EEdgeLocation ATopDownPlayerController::CheckEdgeMovement()
{
	if (GetMousePosition(MouseLocation.X, MouseLocation.Y))
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		// 마우스 좌표 정규화
		MouseLocation /= ViewportSize;

		// Left Top
		if (MouseLocation.X < 0.02f && MouseLocation.Y < 0.02f)
		{
			TopDownCamera->OnMoveRight(-1.0f);
			TopDownCamera->OnMoveForward(1.0f);
			return EEdgeLocation::LeftTop;
		}
		// Right Top
		else if (MouseLocation.X > 0.98f && MouseLocation.Y < 0.02f)
		{
			TopDownCamera->OnMoveRight(1.0f);
			TopDownCamera->OnMoveForward(1.0f);
			return EEdgeLocation::RightTop;
		}
		// Left Bottom
		else if (MouseLocation.X < 0.02f && MouseLocation.Y > 0.98f)
		{
			TopDownCamera->OnMoveRight(-1.0f);
			TopDownCamera->OnMoveForward(-1.0f);
			return EEdgeLocation::LeftBottom;
		}
		// Right Bottom
		else if (MouseLocation.X > 0.98f && MouseLocation.Y > 0.98f)
		{
			TopDownCamera->OnMoveRight(1.0f);
			TopDownCamera->OnMoveForward(-1.0f);
			return EEdgeLocation::RightBottom;
		}
		// Right
		else if (MouseLocation.X > 0.98f)
		{
			TopDownCamera->OnMoveRight(1.0f);
			return EEdgeLocation::Right;
		}
		// Left
		else if (MouseLocation.X < 0.02f)
		{
			TopDownCamera->OnMoveRight(-1.0f);
			return EEdgeLocation::Left;
		}
		// Bottom
		else if (MouseLocation.Y > 0.98f)
		{
			TopDownCamera->OnMoveForward(-1.0f);
			return EEdgeLocation::Bottom;
		}
		// Top
		else if (MouseLocation.Y < 0.02f)
		{
			TopDownCamera->OnMoveForward(1.0f);
			return EEdgeLocation::Top;
		}
	}
	return EEdgeLocation::None;
} 
