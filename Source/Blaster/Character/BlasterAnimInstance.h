// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Animation/AnimInstance.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	UPROPERTY(BlueprintReadOnly, Category=Character, meta=(AllowPrivateAccess="True"))
	class ABlasterChar* BlasterChar;

	UPROPERTY(BlueprintReadOnly, Category=Movement, meta=(AllowPrivateAccess="True"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category=Character, meta=(AllowPrivateAccess="True"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category=Character, meta=(AllowPrivateAccess="True"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category=Character, meta=(AllowPrivateAccess="True"))
	bool bWeaponEquipped;

	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category=Character, meta=(AllowPrivateAccess="True"))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category=Character, meta=(AllowPrivateAccess="True"))
	bool bAiming;

	UPROPERTY(BlueprintReadOnly, Category=Character, meta=(AllowPrivateAccess="True"))
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category=Character, meta=(AllowPrivateAccess="True"))
	float Lean;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category=Character, meta=(AllowPrivateAccess="True"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category=Character, meta=(AllowPrivateAccess="True"))
	float AO_Pitch;

	UPROPERTY(BlueprintReadOnly, Category=Character, meta=(AllowPrivateAccess="True"))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category=Character, meta=(AllowPrivateAccess="True"))
	ETurningInPlace TurningInPlace;
};
