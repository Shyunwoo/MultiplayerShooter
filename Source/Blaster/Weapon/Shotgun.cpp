// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterChar.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
    AWeapon::Fire(HitTarget);

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if(OwnerPawn == nullptr) return;

    AController* InstigatorController = OwnerPawn->GetController();

    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if(MuzzleFlashSocket)
    {
        FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        FVector Start = SocketTransform.GetLocation();

        TMap<ABlasterChar*, uint32> HitMap;
        for(uint32 i =0; i<NumberOfPellets; i++)
        {
            FHitResult FireHit;
            WeaponTraceHit(Start, HitTarget, FireHit);

            ABlasterChar* BlasterCharacter = Cast<ABlasterChar>(FireHit.GetActor());
            if(BlasterCharacter && InstigatorController && HasAuthority())
            {
                if(HitMap.Contains(BlasterCharacter))
                {
                    HitMap[BlasterCharacter]++;
                }
                else
                {
                    HitMap.Emplace(BlasterCharacter, 1);
                }
            }

            if(ImpactParticles)
            {
                UGameplayStatics::SpawnEmitterAtLocation(
                    GetWorld(),
                    ImpactParticles,
                    FireHit.ImpactPoint,
                    FireHit.ImpactNormal.Rotation()
                );
            }
            if(HitSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, .5f, FMath::FRandRange(-.5f, .5f));
            }
        }
        for(auto HitPair : HitMap)
        {
            if(HitPair.Key && InstigatorController && HasAuthority())
            {
                UGameplayStatics::ApplyDamage(
                    HitPair.Key,
                    Damage * HitPair.Value,
                    InstigatorController,
                    this,
                    UDamageType::StaticClass()
                    );
            }
        }
    }
}
