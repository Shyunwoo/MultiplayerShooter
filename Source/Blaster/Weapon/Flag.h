// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()
	
public:
	void ResetFlag();

protected:
	virtual void BeginPlay() override;
	virtual void OnEquipped() override;

private:
	FTransform InitialTransform;

public:
	FORCEINLINE FTransform GetInitialTransform() const {return InitialTransform;}
};
