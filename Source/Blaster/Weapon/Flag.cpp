// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Blaster/Character/BlasterChar.h"
#include "Components/SphereComponent.h"

void AFlag::BeginPlay()
{
    Super::BeginPlay();
    
    InitialTransform = GetActorTransform();
}

void AFlag::OnEquipped()
{
    ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetWeaponMesh()->SetSimulatePhysics(false);
	GetWeaponMesh()->SetEnableGravity(false);
	GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetWeaponMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

	EnableCustomDepth(false);
}

void AFlag::ResetFlag()
{
    ABlasterChar* FlagBearer = Cast<ABlasterChar>(GetOwner());
    if(FlagBearer)
	{
		FlagBearer->SetHoldingTheFlag(false);
        FlagBearer->SetOverlappingWeapon(nullptr);
        FlagBearer->UnCrouch();
	}

    if(!HasAuthority()) return;

    FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
    GetWeaponMesh()->DetachFromComponent(DetachRules);

    SetWeaponState(EWeaponState::EWS_Initial);
    GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    SetOwner(nullptr);
    BlasterOwnerCharacter = nullptr;
    BlasterOwnerController = nullptr;

    SetActorTransform(InitialTransform);
}