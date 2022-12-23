// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"

bool UReturnToMainMenu::Initialize()
{
    if(!Super::Initialize())
    {
        return false;
    }
    return true;
}

void UReturnToMainMenu::MenuSetup()
{
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    UWorld* World = GetWorld();
    if(World)
    {
        PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
        if(PlayerController)
        {
            FInputModeGameAndUI InputModeDate;
            InputModeDate.SetWidgetToFocus(TakeWidget());
            PlayerController->SetInputMode(InputModeDate);
            PlayerController->SetShowMouseCursor(true);
        }
    }
    if(ReturnButton)
    {
        ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
    }

    UGameInstance* GameInstance = GetGameInstance();
    if(GameInstance)
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
        if(MultiplayerSessionsSubsystem)
        {
            MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
        }
    }
}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
    if(!bWasSuccessful)
    {
        ReturnButton->SetIsEnabled(true);
        return;
    }

    UWorld* World = GetWorld();
    if(World)
    {
        AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
        if(GameMode)
        {
            GameMode->ReturnToMainMenuHost();
        }
        else
        {
            PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
            if(PlayerController)
            {
                PlayerController->ClientReturnToMainMenuWithTextReason(FText());
            }
        }
    }
}

void UReturnToMainMenu::MenuTearDown()
{
    RemoveFromParent();

    UWorld* World = GetWorld();
    if(World)
    {
        PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
        if(PlayerController)
        {
            FInputModeGameOnly InputModeDate;
            PlayerController->SetInputMode(InputModeDate);
            PlayerController->SetShowMouseCursor(false);
        }
    }
}

void UReturnToMainMenu::ReturnButtonClicked()
{
    ReturnButton->SetIsEnabled(false);

    if(MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->DestroySession();
    }
}
