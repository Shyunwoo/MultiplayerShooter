// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterChar.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Components/CombatComponent.h"
#include "Blaster/Components/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterController.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/BoxComponent.h"
#include "Blaster/Components/LagCompensationComponent.h"

// Sets default values
ABlasterChar::ABlasterChar()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	CameraBoom=CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength=600.f;
	CameraBoom->bUsePawnControlRotation=true;

	FollowCamera=CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation=false;

	bUseControllerRotationYaw=false;
	GetCharacterMovement()->bOrientRotationToMovement=true;

	OverheadWidget=CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat=CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff=CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	LagCompensation=CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	GetCharacterMovement()->NavAgentProps.bCanCrouch=true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate=FRotator(0.f, 0.f, 850.f);

	TurningInPlace=ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency=66.f;
	MinNetUpdateFrequency=33.f;

	DissolveTimeline=CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Hit boxes for server-side rewind
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("haed"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ABlasterChar::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterChar::ReceiveDamage);
	}
	if(AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void ABlasterChar::Destroyed()
{
	Super::Destroyed();

	if(ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;

	if(Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ABlasterChar::Elim()
{
	DropOrDestroyWeapons();
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterChar::ElimTimerFinished,
		ElimDelay
	);
}

void ABlasterChar::DropOrDestroyWeapons()
{
	if(Combat)
	{
		if(Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if(Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
	}
}

void ABlasterChar::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if(Weapon == nullptr) return;
	if(Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterChar::MulticastElim_Implementation()
{
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed=true;
	PlayElimMontage();

	//Start dissolve effect
	if(DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance=UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	//Disable character movement
	bDisableGameplay=true;
	GetCharacterMovement()->DisableMovement();
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}

	//Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Spawn elim bot
	if(ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X,GetActorLocation().Y,GetActorLocation().Z+200.f);
		ElimBotComponent= UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if(ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
	bool bHideSniperScope = IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if(bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
}

void ABlasterChar::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if(BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterChar::UpdateHUDHealth()
{
	BlasterPlayerController=BlasterPlayerController==nullptr? Cast<ABlasterController>(Controller):BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterChar::UpdateHUDAmmo()
{
	BlasterPlayerController=BlasterPlayerController==nullptr? Cast<ABlasterController>(Controller):BlasterPlayerController;
	if(BlasterPlayerController && Combat && Combat->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ABlasterChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void ABlasterChar::RotateInPlace(float DeltaTime)
{
	if(bDisableGameplay)
	{
		bUseControllerRotationYaw=false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;

		return;
	}

	if(GetLocalRole()>ENetRole::ROLE_SimulatedProxy&&IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication+=DeltaTime;
		if(TimeSinceLastMovementReplication>0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlasterChar::PollInit()
{
	if(BlasterPlayerState==nullptr)
	{
		BlasterPlayerState=GetPlayerState<ABlasterPlayerState>();
		if(BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
		}
	}
}

void ABlasterChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterChar::Jump);

	PlayerInputComponent->BindAxis("MoveForward",this, &ABlasterChar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this, &ABlasterChar::MoveRight);
	PlayerInputComponent->BindAxis("Turn",this, &ABlasterChar::Turn);
	PlayerInputComponent->BindAxis("LookUp",this, &ABlasterChar::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterChar::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterChar::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterChar::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterChar::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterChar::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterChar::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterChar::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterChar::GrenadeButtonPressed);
}

void ABlasterChar::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combat)
	{
		Combat->Character=this;
	}
	if(Buff)
	{
		Buff->Character=this;
		Buff->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if(LagCompensation)
	{
		LagCompensation->Character = this;
		if(Controller)
		{
			LagCompensation->Controller = Cast<ABlasterController>(Controller);
		}
	}
}

void ABlasterChar::PlayFireMontage(bool bAiming)
{
	if(Combat==nullptr||Combat->EquippedWeapon==nullptr) return;

	UAnimInstance* AnimInstance=GetMesh()->GetAnimInstance();
	if(AnimInstance&&FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName=bAiming?FName("RifleAim"):FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterChar::PlayReloadMontage()
{
	if(Combat==nullptr||Combat->EquippedWeapon==nullptr) return;

	UAnimInstance* AnimInstance=GetMesh()->GetAnimInstance();
	if(AnimInstance&&ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		
		switch(Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterChar::PlayElimMontage()
{
	UAnimInstance* AnimInstance=GetMesh()->GetAnimInstance();
	if(AnimInstance&&ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterChar::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication=0.f;
}

void ABlasterChar::PlayHitReactMontage()
{
	if(Combat==nullptr||Combat->EquippedWeapon==nullptr) return;

	UAnimInstance* AnimInstance=GetMesh()->GetAnimInstance();
	if(AnimInstance&&HitReactMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterChar::PlaySwapMontage()
{
	UAnimInstance* AnimInstance=GetMesh()->GetAnimInstance();
	if(AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void ABlasterChar::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser)
{
	if(bElimmed) return;

	float DamageToHealth = Damage;
	if(Shield > 0.f)
	{
		if(Shield >= Damage)
		{
			Shield=FMath::Clamp(Shield-Damage, 0.f, MaxShield);
			DamageToHealth=0.f;
		}
		else
		{
			Shield = 0.f;
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
		}
	}

	Health=FMath::Clamp(Health-DamageToHealth, 0.f, MaxHealth);

	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if(Health==0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if(BlasterGameMode)
		{
			BlasterPlayerController=BlasterPlayerController==nullptr?Cast<ABlasterController>(Controller):BlasterPlayerController;
			ABlasterController* AttackerController=Cast<ABlasterController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
	
}

void ABlasterChar::MoveForward(float Value)
{
	if(bDisableGameplay) return;
	if(Controller!=nullptr&&Value!=0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterChar::MoveRight(float Value)
{
	if(bDisableGameplay) return;
	if(Controller!=nullptr&&Value!=0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterChar::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterChar::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterChar::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterChar, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterChar, Health);
	DOREPLIFETIME(ABlasterChar, bDisableGameplay);
	DOREPLIFETIME(ABlasterChar, Shield);
}

void ABlasterChar::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterChar::EquipButtonPressed()
{
	if(bDisableGameplay) return;
	if(Combat)
	{
		if(Combat->CombatState == ECombatState::ECS_Unoccupied)
		{
			ServerEquipButtonPressed();
		}
		bool bSwap = Combat->ShouldSwapWeapons() &&
		 !HasAuthority() &&
		  Combat->CombatState == ECombatState::ECS_Unoccupied &&
		   OverlappingWeapon == nullptr;

		if(bSwap)
		{
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}
	}
}

void ABlasterChar::ServerEquipButtonPressed_Implementation()
{
	if(Combat)
	{
		if(OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if(Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}

void ABlasterChar::CrouchButtonPressed()
{
	if(bDisableGameplay) return;
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterChar::ReloadButtonPressed()
{
	if(bDisableGameplay) return;
	if(Combat)
	{
		Combat->Reload();
	}
}

void ABlasterChar::AimButtonPressed()
{
	if(bDisableGameplay) return;
	if(Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterChar::AimButtonReleased()
{
	if(bDisableGameplay) return;
	if(Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterChar::GrenadeButtonPressed()
{
	if(Combat)
	{
		Combat->ThrowGrenade();
	}
}

void ABlasterChar::FireButtonPressed()
{
	if(bDisableGameplay) return;
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterChar::FireButtonReleased()
{
	if(bDisableGameplay) return;
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterChar::Jump()
{
	if(bDisableGameplay) return;
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterChar::SimProxiesTurn()
{
	if(Combat==nullptr||Combat->EquippedWeapon==nullptr) return;

	bRotateRootBone=false;
	float Speed=CalculateSpeed();
	if(Speed>0.f)
	{
		TurningInPlace=ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame=ProxyRotation;
	ProxyRotation=GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	
	if(FMath::Abs(ProxyYaw)>TurnThreshold)
	{
		if(ProxyYaw>TurnThreshold)
		{
			TurningInPlace=ETurningInPlace::ETIP_Right;
		}
		else if(ProxyYaw<-TurnThreshold)
		{
			TurningInPlace=ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace=ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace=ETurningInPlace::ETIP_NotTurning;
}

float ABlasterChar::CalculateSpeed()
{
	FVector Velocity=GetVelocity();
    Velocity.Z=0.f;
    return Velocity.Size();
}

void ABlasterChar::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if(Health < LastHealth)
	{
		PlayHitReactMontage();
	}
	
}

void ABlasterChar::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if(Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABlasterChar::UpdateHUDShield()
{
	BlasterPlayerController=BlasterPlayerController==nullptr? Cast<ABlasterController>(Controller):BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterChar::AimOffset(float DeltaTime)
{
	if(Combat&&Combat->EquippedWeapon==nullptr) return;

	float Speed=CalculateSpeed();
	bool bIsInAir=GetCharacterMovement()->IsFalling();

	if(Speed==0.f&&!bIsInAir)
	{
		bRotateRootBone=true;

		FRotator CurrentAimRotation=FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation=UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw=DeltaAimRotation.Yaw;

		if(TurningInPlace==ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw=AO_Yaw;
		}

		bUseControllerRotationYaw=true;

		TurnInPlace(DeltaTime);
	}
	if(Speed>0.f||bIsInAir)
	{
		bRotateRootBone=false;

		StartingAimRotation=FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw=0.f;
		bUseControllerRotationYaw=true;

		TurningInPlace=ETurningInPlace::ETIP_NotTurning;
	}
	CalculateAO_Pitch();
}

void ABlasterChar::CalculateAO_Pitch()
{
	AO_Pitch=GetBaseAimRotation().Pitch;
	if(AO_Pitch>90.f&&!IsLocallyControlled())
	{
		//map pitch from(270, 360) to (-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch=FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterChar::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw>90.f)
	{
		TurningInPlace=ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw<-90.f)
	{
		TurningInPlace=ETurningInPlace::ETIP_Left;
	}

	if(TurningInPlace!=ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw=FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw=InterpAO_Yaw;
		if(FMath::Abs(AO_Yaw)<15.f)
		{
			TurningInPlace=ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation=FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterChar::HideCameraIfCharacterClose()
{
	if(!IsLocallyControlled()) return;

	if((FollowCamera->GetComponentLocation()-GetActorLocation()).Size()<CamearaThreshold)
	{
		GetMesh()->SetVisibility(false);
		if(Combat&&Combat->EquippedWeapon&&Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee=true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat&&Combat->EquippedWeapon&&Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee=false;
		}
	}
}

void ABlasterChar::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterChar::UpdateDissolveMaterial);
	if(DissolveCurve&&DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterChar::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterChar::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon=Weapon;

	if(IsLocallyControlled())
	{
		if(OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterChar::IsWeaponEquipped()
{
	return (Combat&&Combat->EquippedWeapon);
}

bool ABlasterChar::IsAiming()
{
	return (Combat&&Combat->bAiming);
}

AWeapon* ABlasterChar::GetEquippedWeapon()
{
	if(Combat==nullptr) return nullptr;

	return Combat->EquippedWeapon;
}

FVector ABlasterChar::GetHitTarget() const
{
	if(Combat==nullptr) return FVector();

	return Combat->HitTarget;
}

ECombatState ABlasterChar::GetCombatState() const
{
	if(Combat==nullptr) return ECombatState::ECS_MAX;

	return Combat->CombatState;
}

void ABlasterChar::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance=GetMesh()->GetAnimInstance();
	if(AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterChar::SpawnDefaultWeapon()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if(BlasterGameMode && World && !bElimmed && DefaultWeaponClass)
	{	
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if(Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

bool ABlasterChar::IsLocallyReloading()
{
	if(Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}