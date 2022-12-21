// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterChar.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/PlayerController/BlasterController.h"
#include "Blaster/Components/LagCompensationComponent.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
    AWeapon::Fire(FVector());

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if(OwnerPawn == nullptr) return;

    AController* InstigatorController = OwnerPawn->GetController();
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if(MuzzleFlashSocket)
    {
        const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        const FVector Start = SocketTransform.GetLocation();

        //Maps hit character to number of times hit
        TMap<ABlasterChar*, uint32> HitMap;
        for(FVector_NetQuantize HitTarget : HitTargets)
        {
            FHitResult FireHit;
            WeaponTraceHit(Start, HitTarget, FireHit);

            ABlasterChar* BlasterCharacter = Cast<ABlasterChar>(FireHit.GetActor());
            if(BlasterCharacter)
            {
                if(HitMap.Contains(BlasterCharacter))
                {
                    HitMap[BlasterCharacter]++;
                }
                else
                {
                    HitMap.Emplace(BlasterCharacter, 1);
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
        }
        TArray<ABlasterChar*> HitCharacters;

        for(auto HitPair : HitMap)
        {
            if(HitPair.Key && InstigatorController)
            {
                if(HasAuthority() && !bUseServersideRewind)
                {
                    UGameplayStatics::ApplyDamage(
                        HitPair.Key,
                        Damage * HitPair.Value,
                        InstigatorController,
                        this,
                        UDamageType::StaticClass()
                        );
                }
                HitCharacters.Add(HitPair.Key);
            }
        }
        if(!HasAuthority() && bUseServersideRewind)
        {
            BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterChar>(OwnerPawn) : BlasterOwnerCharacter;
            BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterController>(InstigatorController) : BlasterOwnerController;

            if(BlasterOwnerController && BlasterOwnerCharacter && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerCharacter->IsLocallyControlled())
            {
                BlasterOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
                    HitCharacters,
                    Start,
                    HitTargets,
                    BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime
                );
            }
        }
    }
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if(MuzzleFlashSocket == nullptr) return;

    const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    const FVector TraceStart = SocketTransform.GetLocation();

    const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    const FVector SphereCenter = TraceStart+ToTargetNormalized * DistanceToSphere;

    for(uint32 i=0; i<NumberOfPellets; i++)
    {
        const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
        const FVector EndLoc = SphereCenter + RandVec;
        FVector ToEndLoc = EndLoc - TraceStart;
        ToEndLoc = TraceStart+ToEndLoc*TRACE_LENGTH/ToEndLoc.Size();

        HitTargets.Add(ToEndLoc);
    }
}
