// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Blaster/Character/BlasterChar.h"
#include "Blaster/PlayerController/BlasterController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
    SetScore(GetScore()+ScoreAmount);
    Character=Character==nullptr?Cast<ABlasterChar>(GetPawn()):Character;
    if(Character)
    {
        Controller=Controller==nullptr?Cast<ABlasterController>(Character->Controller):Controller;
        if(Controller)
        {
            Controller->SetHUDScore(GetScore());
        }
    }
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
    Defeats+=DefeatsAmount;
    Character=Character==nullptr?Cast<ABlasterChar>(GetPawn()):Character;
    if(Character)
    {
        Controller=Controller==nullptr?Cast<ABlasterController>(Character->Controller):Controller;
        if(Controller)
        {
            Controller->SetHUDDefeats(Defeats);
        }
    }
}

void ABlasterPlayerState::OnRep_Score()
{
    Super::OnRep_Score();

    Character=Character==nullptr?Cast<ABlasterChar>(GetPawn()):Character;
    if(Character)
    {
        Controller=Controller==nullptr?Cast<ABlasterController>(Character->Controller):Controller;
        if(Controller)
        {
            Controller->SetHUDScore(GetScore());
        }
    }
}

void ABlasterPlayerState::OnRep_Defeats()
{
    Character=Character==nullptr?Cast<ABlasterChar>(GetPawn()):Character;
    if(Character)
    {
        Controller=Controller==nullptr?Cast<ABlasterController>(Character->Controller):Controller;
        if(Controller)
        {
            Controller->SetHUDDefeats(Defeats);
        }
    }
}

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&OutLifetimeProps) const 
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABlasterPlayerState, Defeats);
}
