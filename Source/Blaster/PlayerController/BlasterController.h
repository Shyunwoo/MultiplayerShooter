// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float Health, float MaxHealth);
	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;

private:
	class ABlasterHUD* BlasterHUD;
};
