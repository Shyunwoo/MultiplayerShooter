// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "Blaster/Character/BlasterChar.h"
#include "Blaster/Components/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    ABlasterChar* BlasterCharacter = Cast<ABlasterChar>(OtherActor);
    if(BlasterCharacter)
    {
        UCombatComponent* Combat = BlasterCharacter->GetCombat();
        if(Combat)
        {
            Combat->PickupAmmo(WeaponType, AmmoAmount);
        }
    }
    Destroy();
}
