// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "Blaster/Character/BlasterChar.h"
#include "Blaster/Components/BuffComponent.h"


AHealthPickup::AHealthPickup()
{
    bReplicates = true;

}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    ABlasterChar* BlasterCharacter = Cast<ABlasterChar>(OtherActor);
    if(BlasterCharacter)
    {
        UBuffComponent* Buff = BlasterCharacter->GetBuff();
        if(Buff)
        {
            Buff->Heal(HealAmount, HealingTime);
        }
    }
    Destroy();
}
