// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterChar.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"


void UBlasterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    BlasterChar=Cast<ABlasterChar>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if(BlasterChar==nullptr)
    {
        BlasterChar=Cast<ABlasterChar>(TryGetPawnOwner());
    }

    if(BlasterChar==nullptr) return;

    FVector Velocity=BlasterChar->GetVelocity();
    Velocity.Z=0.f;
    Speed=Velocity.Size();

    bIsInAir=BlasterChar->GetCharacterMovement()->IsFalling();
    bIsAccelerating=BlasterChar->GetCharacterMovement()->GetCurrentAcceleration().Size()>0.f?true:false;
    bWeaponEquipped=BlasterChar->IsWeaponEquipped();
    EquippedWeapon=BlasterChar->GetEquippedWeapon();
    bIsCrouched=BlasterChar->bIsCrouched;
    bAiming=BlasterChar->IsAiming();
    TurningInPlace=BlasterChar->GetTurningInPlace();
    bRotateRootBone=BlasterChar->ShouldRotateRootBone();
    bElimmed=BlasterChar->IsElimmed();

    FRotator AimRotation = BlasterChar->GetBaseAimRotation();
    FRotator MovementRotation=UKismetMathLibrary::MakeRotFromX(BlasterChar->GetVelocity());
    FRotator DeltaRot =UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
    DeltaRotation=FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
    YawOffset=DeltaRotation.Yaw;

    CharacterRotationLastFrame=CharacterRotation;
    CharacterRotation=BlasterChar->GetActorRotation();
    const FRotator Delta= UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
    const float Target=Delta.Yaw/DeltaTime;
    const float Interp=FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
    Lean=FMath::Clamp(Interp, -90.f, 90.f);

    AO_Yaw=BlasterChar->GetAO_Yaw();
    AO_Pitch=BlasterChar->GetAO_Pitch();

    if(bWeaponEquipped&&EquippedWeapon&&EquippedWeapon->GetWeaponMesh()&&BlasterChar->GetMesh())
    {
        LeftHandTransform=EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
        FVector OutPosition;
        FRotator OutRotation;
        BlasterChar->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
        LeftHandTransform.SetLocation(OutPosition);
        LeftHandTransform.SetRotation(FQuat(OutRotation));

        if(BlasterChar->IsLocallyControlled())
        {
            bLocallyControlled=true;
            FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);

            FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation()+ (RightHandTransform.GetLocation() - BlasterChar->GetHitTarget()));

            RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
        }
    }
}
