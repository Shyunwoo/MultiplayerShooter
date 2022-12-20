// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/BlasterChar.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Components/CombatComponent.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/Image.h"

void ABlasterController::BeginPlay()
{
    Super::BeginPlay();

    BlasterHUD = Cast<ABlasterHUD>(GetHUD());
    ServerCheckMatchState();
    ABlasterChar* BlasterCharacter=Cast<ABlasterChar>(GetPawn());
    if(BlasterCharacter && BlasterCharacter->GetCombat())
    {
        SetHUDGrenades(BlasterCharacter->GetCombat()->GetGrenades());
    }
}

void ABlasterController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    SetHUDTime();
    CheckTimeSync(DeltaTime);
    PollInit();
    CheckPing(DeltaTime);
}

void ABlasterController::CheckPing(float DeltaTime)
{
    HighPingRunningTime += DeltaTime;
    if(HighPingRunningTime > CheckPingFrequency)
    {
        PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>(): PlayerState;
        if(PlayerState)
        {
            //Ping is compressed in Getping Function, It's actually ping / 4
            if(PlayerState->GetPingInMilliseconds()> HighPingThreshold)
            {
                HighPingWarning();
                PingAnimationRunningTime=0.f;
            }
        }
        HighPingRunningTime=0.f;
    }
    bool bHighPingAnimationPlaying = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HighPingAnimation && BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);

    if(bHighPingAnimationPlaying)
    {
        PingAnimationRunningTime += DeltaTime;
        if(PingAnimationRunningTime > HighPingDuration)
        {
            StopHighPingWarning();
        }
    }
}

void ABlasterController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABlasterController, MatchState);
}

void ABlasterController::CheckTimeSync(float DeltaTime)
{
    TimeSyncRunningTime += DeltaTime;
    if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
    {
        ServerRequestServerTime(GetWorld()->GetTimeSeconds());
        TimeSyncRunningTime = 0.f;
    }
}

void ABlasterController::ServerCheckMatchState_Implementation()
{
    ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));

    if(GameMode)
    {
        WarmupTime = GameMode->WarmupTime;
        MatchTime = GameMode->MatchTime;
        CooldownTime = GameMode->CooldownTime;
        LevelStartingTime = GameMode->LevelStartingTime;
        MatchState = GameMode->GetMatchState();
        ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
    }
}

void ABlasterController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
    WarmupTime = Warmup;
    MatchTime = Match;
    CooldownTime = Cooldown;
    LevelStartingTime = StartingTime;
    MatchState = StateOfMatch;
    OnMatchStateSet(MatchState);

    if(BlasterHUD && MatchState == MatchState::WaitingToStart)
    {
        BlasterHUD->AddAnnouncement();
    }
}

void ABlasterController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ABlasterChar* BlasterCharacter=Cast<ABlasterChar>(InPawn);
    if(BlasterCharacter)
    {
         SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
    }
    
}

void ABlasterController::SetHUDHealth(float Health, float MaxHealth)
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;

    bool bHUDValid=BlasterHUD&&BlasterHUD->CharacterOverlay&&BlasterHUD->CharacterOverlay->HealthBar&&BlasterHUD->CharacterOverlay->HealthText;
    if(bHUDValid)
    {
        const float HealthPercent=Health/MaxHealth;
        BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
        FString HealthText=FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
        BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
    }
    else
    {
        bInitializeHealth = true;
        HUDHealth = Health;
        HUDMaxHealth = MaxHealth;
    }
}

void ABlasterController::SetHUDShield(float Shield, float MaxShield)
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;

    bool bHUDValid=BlasterHUD&&BlasterHUD->CharacterOverlay&&BlasterHUD->CharacterOverlay->ShieldBar&&BlasterHUD->CharacterOverlay->ShieldText;
    if(bHUDValid)
    {
        const float ShieldPercent=Shield/MaxShield;
        BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
        FString ShieldText=FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
        BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
    }
    else
    {
        bInitializeShield = true;
        HUDShield = Shield;
        HUDMaxShield = MaxShield;
    }
}

void ABlasterController::SetHUDScore(float Score)
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;
    bool bHUDValid=BlasterHUD&&BlasterHUD->CharacterOverlay&&BlasterHUD->CharacterOverlay->ScoreAmount;
    if(bHUDValid)
    {
        FString ScoreText=FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
        BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
    }
    else
    {
        bInitializeScore = true;
        HUDScore = Score;
    }
}

void ABlasterController::SetHUDDefeats(int32 Defeats)
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;
    bool bHUDValid=BlasterHUD&&
    BlasterHUD->CharacterOverlay&&
    BlasterHUD->CharacterOverlay->DefeatsAmount;

    if(bHUDValid)
    {
        FString DefeatsText=FString::Printf(TEXT("%d"), Defeats);
        BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
    }
    else
    {
        bInitializeDefeats = true;
        HUDDefeats = Defeats;
    }
}

void ABlasterController::SetHUDWeaponAmmo(int32 Ammo)
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;
    bool bHUDValid=BlasterHUD&&
    BlasterHUD->CharacterOverlay&&
    BlasterHUD->CharacterOverlay->WeaponAmmoAmount;

    if(bHUDValid)
    {
        FString AmmoText=FString::Printf(TEXT("%d"), Ammo);
        BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
    }
    else
    {
        bInitializeWeaponAmmo = true;
        HUDWeaponAmmo = Ammo;
    }
}

void ABlasterController::SetHUDCarriedAmmo(int32 Ammo)
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;
    bool bHUDValid=BlasterHUD&&
    BlasterHUD->CharacterOverlay&&
    BlasterHUD->CharacterOverlay->CarriedAmmoAmount;

    if(bHUDValid)
    {
        FString AmmoText=FString::Printf(TEXT("%d"), Ammo);
        BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
    }
    else
    {
        bInitializeCarriedAmmo = true;
        HUDCarriedAmmo = Ammo;
    }
}

void ABlasterController::SetHUDMatchCountdown(float CountdownTime)
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;
    bool bHUDValid=BlasterHUD&&BlasterHUD->CharacterOverlay&&BlasterHUD->CharacterOverlay->MatchCountdownText;
    if(bHUDValid)
    {
        if(CountdownTime < 0.f)
        {
            BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());   
            return; 
        }

        int32 Minutes = FMath::FloorToInt(CountdownTime/60.f);
        int32 Seconds = CountdownTime - Minutes * 60;

        FString CountdownText = FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds);
        BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
    }
}

void ABlasterController::SetHUDAnnouncementCountdown(float CountdownTime)
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;
    bool bHUDValid=BlasterHUD&&BlasterHUD->Announcement&&BlasterHUD->Announcement->WarmupTime;
    if(bHUDValid)
    {
        if(CountdownTime < 0.f)
        {
            BlasterHUD->Announcement->WarmupTime->SetText(FText());   
            return; 
        }

        int32 Minutes = FMath::FloorToInt(CountdownTime/60.f);
        int32 Seconds = CountdownTime - Minutes * 60;

        FString CountdownText = FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds);
        BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
    }
}

void ABlasterController::SetHUDTime()
{
    float TimeLeft = 0.f;
    if(MatchState == MatchState::WaitingToStart)
    {
        TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
    }
    else if(MatchState == MatchState::InProgress)
    {
        TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
    }
    else if(MatchState == MatchState::Cooldown)
    {
        TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
    }

    uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

    if(HasAuthority())
    {
        BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;

        if(BlasterGameMode)
        {
            SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
        }
    }

    if(CountdownInt != SecondsLeft)
    {
        if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
        {
            SetHUDAnnouncementCountdown(TimeLeft);
        }
        else if(MatchState == MatchState::InProgress)
        {
            SetHUDMatchCountdown(TimeLeft);
        }
    }
    CountdownInt = SecondsLeft;
}

void ABlasterController::SetHUDGrenades(int32 Grenades)
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;
    bool bHUDValid=BlasterHUD&&
        BlasterHUD->CharacterOverlay&&
        BlasterHUD->CharacterOverlay->GrenadesText;

    if(bHUDValid)
    {
        FString GrenadesText=FString::Printf(TEXT("%d"), Grenades);
        BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
    }
    else
    {
        bInitializeGrenades = true;
        HUDGrenades= Grenades;
    }
}

void ABlasterController::PollInit()
{
    if(CharacterOverlay == nullptr)
    {
        if(BlasterHUD && BlasterHUD->CharacterOverlay)
        {
            CharacterOverlay = BlasterHUD->CharacterOverlay;
            if(CharacterOverlay)
            {
                if(bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
                if(bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
                if(bInitializeScore) SetHUDScore(HUDScore);
                if(bInitializeDefeats) SetHUDDefeats(HUDDefeats);
                if(bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
                if(bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

                ABlasterChar* BlasterCharacter = Cast<ABlasterChar>(GetPawn());
                if(BlasterCharacter && BlasterCharacter->GetCombat())
                {
                    if(bInitializeGrenades) SetHUDGrenades(BlasterCharacter->GetCombat()->GetGrenades());
                }
            }
        }
    }
}

void ABlasterController::ServerRequestServerTime_Implementation(float TimeOfClinetRequest)
{
    float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
    ClientReportServerTime(TimeOfClinetRequest, ServerTimeOfReceipt);
}

void ABlasterController::ClientReportServerTime_Implementation(float TimeOffClientRequest, float TimeServerReceivedClientRequest)
{
    float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOffClientRequest;
    float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTripTime * 0.5f);
    ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlasterController::GetServerTime()
{
    if(HasAuthority()) return GetWorld()->GetTimeSeconds();
    else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterController::ReceivedPlayer()
{
    Super::ReceivedPlayer();

    if(IsLocalController())
    {
        ServerRequestServerTime(GetWorld()->GetTimeSeconds());
    }
}

void ABlasterController::OnMatchStateSet(FName State)
{
    MatchState = State;

    if(MatchState == MatchState::InProgress)
    {
        HandleMatchStarted();
    }
    else if(MatchState == MatchState::Cooldown)
    {
        HandleCooldown();
    }
}

void ABlasterController::OnRep_MetchState()
{
    if(MatchState == MatchState::InProgress)
    {
        HandleMatchStarted();
    }
    else if(MatchState == MatchState::Cooldown)
    {
        HandleCooldown();
    }
}

void ABlasterController::HandleMatchStarted()
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;
    if(BlasterHUD)
    {
        if(BlasterHUD->CharacterOverlay == nullptr)
        {
            BlasterHUD->AddCharacterOverlay();
        }
        if(BlasterHUD->Announcement)
        {
            BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void ABlasterController::HandleCooldown()
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;
    if(BlasterHUD)
    {
        BlasterHUD->CharacterOverlay->RemoveFromParent();
        bool bHUDValid = BlasterHUD->Announcement && BlasterHUD->Announcement->AnnouncementText && BlasterHUD->Announcement->InfoText;

        if(bHUDValid)
        {
            BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
            FString AnnouncementText("New Match Starts In : ");
            BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

            ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
            ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();

            if(BlasterGameState && BlasterPlayerState)
            {
                TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
                FString InfoTextString;

                if(TopPlayers.Num() == 0)
                {
                    InfoTextString = FString("There is no winner.");
                }
                else if(TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
                {
                    InfoTextString = FString("You are the winner!");
                }
                else if(TopPlayers.Num() == 1)
                {
                    InfoTextString = FString::Printf(TEXT("Winner : \n%s"), *TopPlayers[0]->GetPlayerName());
                }
                else if(TopPlayers.Num() > 1)
                {
                    InfoTextString = FString("Player tied for the win : \n");
                    for(auto TiedPlayer: TopPlayers)
                    {
                        InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
                    }
                }

                BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
            }
        }
    }
    ABlasterChar* BlasterCharacter = Cast<ABlasterChar>(GetPawn());
    if(BlasterCharacter && BlasterCharacter->GetCombat())
    {
        BlasterCharacter->bDisableGameplay=true;
        BlasterCharacter->GetCombat()->FireButtonPressed(false);
    }
}

void ABlasterController::HighPingWarning()
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;

    bool bHUDValid=BlasterHUD&&BlasterHUD->CharacterOverlay&&BlasterHUD->CharacterOverlay->HighPingImage&&BlasterHUD->CharacterOverlay->HighPingAnimation;
    if(bHUDValid)
    {
        BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
        BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation, 0.f, 5);
    }
}

void ABlasterController::StopHighPingWarning()
{
    BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;

    bool bHUDValid=BlasterHUD&&BlasterHUD->CharacterOverlay&&BlasterHUD->CharacterOverlay->HighPingImage&&BlasterHUD->CharacterOverlay->HighPingAnimation;
    if(bHUDValid)
    {
        BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
        if(BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
        {
            BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);
        }
    }
}