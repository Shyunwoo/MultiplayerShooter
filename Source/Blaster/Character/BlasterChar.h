// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Components/TimelineComponent.h"
#include "Blaster/Interfaces/InteractCrosshairsInterface.h"
#include "GameFramework/Character.h"
#include "BlasterChar.generated.h"

UCLASS()
class BLASTER_API ABlasterChar : public ACharacter, public IInteractCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterChar();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();

	virtual void OnRep_ReplicatedMovement() override;

	void Elim();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	virtual void Jump() override;
	void SimProxiesTurn();

	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();

	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	void UpdateHUDHealth();

	//Poll for any relavant classes and initialize our HUD
	void PollInit();
private:
	UPROPERTY(VisibleAnywhere, Category=Camere)
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, Category=Camere)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category=Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	class UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	class UAnimMontage* ElimMontage;

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CamearaThreshold=200.f;

	bool bRotateRootBone;
	float TurnThreshold=0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	//Player health
	UPROPERTY(EditAnywhere, Category="Player Stats")
	float MaxHealth=100.f;

	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category="Player Stats")
	float Health=100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class ABlasterController* BlasterPlayerController;

	bool bElimmed=false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay=3.f;

	void ElimTimerFinished();

	//Dissolve effect
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	//Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category=Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	//Material instance set on the bluprint, used with the dynamin material instance
	UPROPERTY(EditAnywhere, Category=Elim)
	UMaterialInstance* DissolveMaterialInstance;

	//Elim Bot
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;
public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw()const{return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch()const{return AO_Pitch;}
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const{return TurningInPlace;}
	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const{return FollowCamera;}
	FORCEINLINE bool ShouldRotateRootBone() const{return bRotateRootBone;}
	FORCEINLINE bool IsElimmed() const{return bElimmed;}
	FORCEINLINE float GetHealth() const{return Health;}
	FORCEINLINE float GetMaxHealth() const{return MaxHealth;}
};