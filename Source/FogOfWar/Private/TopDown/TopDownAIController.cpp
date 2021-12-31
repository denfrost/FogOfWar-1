// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDown/TopDownAIController.h"
#include "TopDown/TopDownUnit.h"

void ATopDownAIController::BeginPlay()
{
    Super::BeginPlay();

    Unit = GetPawn<ATopDownUnit>();
    if (Unit)
    {
        TeamId = Unit->GetGenericTeamId();
    }
}

ETeamAttitude::Type ATopDownAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
    // ���Ͱ� ������ Ȯ���մϴ�.
    const APawn* OtherPawn = Cast<APawn>(&Other);
    if (OtherPawn == nullptr)
    {
        return ETeamAttitude::Neutral;
    }

    // ���Ͱ� IGenericTeamAgentInterface�� �����ߴ��� Ȯ���մϴ�.
    const IGenericTeamAgentInterface* IBot = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController());
    if (IBot == nullptr)
    {
        return ETeamAttitude::Neutral;
    }

    // ������ �� ���̵� �����ɴϴ�.
    FGenericTeamId OtherTeamId = IBot->GetGenericTeamId();

    // OtherActor�� ���� �µ��� �����մϴ�.
    if (TeamId == OtherTeamId)
    {
        return ETeamAttitude::Friendly;
    }
    else if (OtherTeamId == 255)
    {
        return ETeamAttitude::Neutral;
    }
    else
    {
        return ETeamAttitude::Hostile;
    }
}

ETeamAttitude::Type ATopDownAIController::GetAttitudeTowards(const AActor* Other) const
{
    return GetTeamAttitudeTowards(*Other);
}

FGenericTeamId ATopDownAIController::GetGenericTeamId() const
{
    return TeamId;
}
