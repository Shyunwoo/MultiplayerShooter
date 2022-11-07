// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PlayerEliminated(class ABlasterChar* ElimmedCharacter, class ABlasterController* VictimController, class ABlasterController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, class AController* ElimmedController);
};
