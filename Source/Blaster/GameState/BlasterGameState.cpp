// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/PlayerController/BlasterController.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
    DOREPLIFETIME(ABlasterGameState, RedTeamScore);
    DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}

void ABlasterGameState::UpdateTopScore(class ABlasterPlayerState* ScoringPlayer)
{
    if(TopScoringPlayers.Num() == 0)
    {
        TopScoringPlayers.Add(ScoringPlayer);
        TopScore = ScoringPlayer->GetScore();
    }
    else if(ScoringPlayer->GetScore() == TopScore)
    {
        TopScoringPlayers.AddUnique(ScoringPlayer);
    }
    else if(ScoringPlayer->GetScore() > TopScore)
    {
        TopScoringPlayers.Empty();
        TopScoringPlayers.AddUnique(ScoringPlayer);
        TopScore = ScoringPlayer->GetScore();
    }
}

void ABlasterGameState::OnRep_RedTeamScore()
{
    ABlasterController* BPlayer = Cast<ABlasterController>(GetWorld()->GetFirstPlayerController());
    if(BPlayer)
    {
        BPlayer->SetHUDRedTeamScore(RedTeamScore);
    }
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
    ABlasterController* BPlayer = Cast<ABlasterController>(GetWorld()->GetFirstPlayerController());
    if(BPlayer)
    {
        BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
    }
}

void ABlasterGameState::RedTeamScores()
{
    ++RedTeamScore;

    ABlasterController* BPlayer = Cast<ABlasterController>(GetWorld()->GetFirstPlayerController());
    if(BPlayer)
    {
        BPlayer->SetHUDRedTeamScore(RedTeamScore);
    }
}

void ABlasterGameState::BlueTeamScores()
{
    ++BlueTeamScore;

    ABlasterController* BPlayer = Cast<ABlasterController>(GetWorld()->GetFirstPlayerController());
    if(BPlayer)
    {
        BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
    }
}