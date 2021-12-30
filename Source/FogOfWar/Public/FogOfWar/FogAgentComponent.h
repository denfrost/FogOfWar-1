// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FogAgentComponent.generated.h"

UCLASS(ClassGroup = (FogOfWar), meta = (BlueprintSpawnableComponent))
class FOGOFWAR_API UFogAgentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFogAgentComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(Category = "Fog Agent", BlueprintCallable)
	void EnableFogAgent();
	UFUNCTION(Category = "Fog Agent", BlueprintCallable)
	void DisableFogAgent();

	UFUNCTION(Category = "Fog Agent", BlueprintPure)
	FVector GetFogAgentLocation() const;

	UPROPERTY(Category = "Fog Agent", EditAnywhere, BlueprintReadWrite)
	bool bEnableAgent = false;

	int Sight = 800;

private:
	UPROPERTY()
	class AFogManager* FogManager = nullptr;
};
