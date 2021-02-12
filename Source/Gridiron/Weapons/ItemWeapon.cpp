// Created by Bruce Crum


#include "Gridiron/Weapons/ItemWeapon.h"
#include "Gridiron/Characters/GridironCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<int32> CvarInfiniteAmmo(TEXT("InfiniteAmmo"), 0, TEXT("Allows for infinite ammo"));
#endif

DEFINE_LOG_CATEGORY(LogWeapon);

AItemWeapon::AItemWeapon()
{
	AttachWeaponSocketName = FName("WeaponGripPoint");
	DamageTypeClass = nullptr;

	Ammo = 0;
	AmmoType = EAmmoType::AT_None;
	AmmoPerShot = 1;
	MinAmmoRequiredToShoot = 1;
	StartingAmmo = 0;
	bConsumesAmmo = true;
}

void AItemWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AItemWeapon, Ammo, COND_OwnerOnly);
}

void AItemWeapon::InitItem(AGridironCharacter* NewOwner)
{
	Super::InitItem(NewOwner);

	// Null check this for safety
	if (NewOwner)
	{
		AttachToComponent(NewOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, AttachWeaponSocketName);

		// We were just picked up, give the owner some ammo.
		if (StartingAmmo > 0)
		{
			// NewOwner->GiveAmmo(AmmoType, StartingAmmo);
		}
	}
}

void AItemWeapon::Equip()
{
	Super::Equip();

	UpdateAmmo();
}

void AItemWeapon::Unequip()
{
	Super::Unequip();

	if (PawnOwner)
	{
		// PawnOwner->StoreAmmo(AmmoType, Ammo);
	}
}

bool AItemWeapon::AllowFire() const
{
	bool bShouldConsumeAmmo = bConsumesAmmo;

#if !UE_BUILD_SHIPPING
	if (CvarInfiniteAmmo.GetValueOnGameThread() > 0)
	{
		bShouldConsumeAmmo = false;
	}
#endif

	if (bShouldConsumeAmmo && (Ammo <= 0 || Ammo < MinAmmoRequiredToShoot))
	{
		return false;
	}

	if (IsSwappingTo())
	{
		return false;
	}

	return true;
}

bool AItemWeapon::ShouldDealDamage(AActor* TestActor) const
{
	// if we're an actor on the server, or the actor's role is authoritative, we should register damage
	if (TestActor)
	{
		if (GetNetMode() != NM_Client ||
			TestActor->GetLocalRole() == ROLE_Authority ||
			TestActor->GetTearOff())
		{
			return true;
		}
	}

	return false;
}

void AItemWeapon::DealDamage(const FHitResult& Impact, float Damage, const FVector& ShootDir)
{
	if (!PawnOwner || !Impact.GetActor())
	{
		return;
	}

	FPointDamageEvent DamageEvent;
	DamageEvent.DamageTypeClass = DamageTypeClass;
	DamageEvent.HitInfo = Impact;
	DamageEvent.ShotDirection = ShootDir;
	DamageEvent.Damage = Damage;

	Impact.GetActor()->TakeDamage(Damage, DamageEvent, PawnOwner->GetController(), this);
}

void AItemWeapon::UpdateAmmo()
{
	if (PawnOwner)
	{
		// Ammo = PawnOwner->GetAmmoAmountForType(AmmoType);
	}
}

void AItemWeapon::DecrementAmmo()
{
	bool bShouldDecrement = bConsumesAmmo;

#if !UE_BUILD_SHIPPING
	if (CvarInfiniteAmmo.GetValueOnGameThread() > 0)
	{
		bShouldDecrement = false;
	}
#endif

	if (bShouldDecrement)
	{
		Ammo = FMath::Max<int32>(Ammo - AmmoPerShot, 0);
	}
}

void AItemWeapon::PlayWeaponSoundAtLocation(USoundBase* Sound, const FVector& Location)
{
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
	}
}

FVector AItemWeapon::GetAdjustedAim() const
{
	if (PawnOwner && PawnOwner->GetController())
	{
		return PawnOwner->GetController()->GetControlRotation().Vector();
	}

	return FVector::ZeroVector;
}

FVector AItemWeapon::GetCameraDamageStartLocation() const
{
	if (PawnOwner)
	{
		return PawnOwner->GetCameraLocation();
	}

	return FVector::ZeroVector;
}

float AItemWeapon::GetDamageMultiplier(TWeakObjectPtr<AActor> HitActor, TWeakObjectPtr<UPhysicalMaterial> HitMaterial) const
{
	float Multiplier = 1.f;

	if (HitMaterial.IsValid() && DamageMultiplierMap.Contains(HitMaterial.Get()))
	{
		Multiplier *= DamageMultiplierMap.FindRef(HitMaterial.Get());
	}

	return Multiplier;
}