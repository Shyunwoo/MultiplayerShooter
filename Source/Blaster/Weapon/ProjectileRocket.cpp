// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(RootComponent);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    RocketMovementComponent=CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
    RocketMovementComponent->bRotationFollowsVelocity=true;
    RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
    Super::BeginPlay();    

    if(ProjectileLoop && LoopingSoundAttenuation)
    {
        ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
            ProjectileLoop,
            GetRootComponent(),
            FName(),
            GetActorLocation(),
            EAttachLocation::KeepWorldPosition,
            false,
            1.f,
            1.f,
            0.f,
            LoopingSoundAttenuation,
            (USoundConcurrency*)nullptr,
            false
        );
    }
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if(OtherActor == GetOwner())
    {
        return;
    }
    ExplodeDamage();

    if(ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
    {
        ProjectileLoopComponent->Stop();
    }
    Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

