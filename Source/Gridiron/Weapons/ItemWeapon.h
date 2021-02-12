// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "Gridiron/Items/ItemEquipable.h"
#include "Gridiron/Weapons/WeaponEnums.h"
#include "ItemWeapon.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWeapon, Log, Verbose);

/**
 * 
 */
UCLASS()
class GRIDIRON_API AItemWeapon : public AItemEquipable
{
	GENERATED_BODY()

public:

	AItemWeapon();

	// Initializes the item
	virtual void InitItem(AGridironCharacter* NewOwner) override;

	// What socket is this weapon going to attach to?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	FName AttachWeaponSocketName;

	// Are we allowed to "fire" this weapon?
	virtual bool AllowFire() const;

	// Variable replication setup.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Will attempt to decrement ammo, will not decremtn ammo is bConsumesAmmo is set to false.
	void DecrementAmmo();

	// Updates ammo, may give or take based on what the owning characters stored ammo is.
	void UpdateAmmo();

	// Will play a sound related to the weapon at this location.
	void PlayWeaponSoundAtLocation(USoundBase* Sound, const FVector& Location);

	// Gets the direction we are currently aiming
	FVector GetAdjustedAim() const;

	// Checks to see where we should start the trace.
	FVector GetCameraDamageStartLocation() const;

	// Returns a damage multiplier for when we are dealing damage.
	virtual float GetDamageMultiplier(TWeakObjectPtr<AActor> HitActor, TWeakObjectPtr<UPhysicalMaterial> HitMaterial) const;

	// Called when we equip the weapon
	virtual void Equip() override;

	// Called when we unequip the weapon
	virtual void Unequip() override;

protected:

	// Checks to see if we can actually deal damage.
	virtual bool ShouldDealDamage(AActor* TestActor) const;

	// Deals damage to the thing we hit.
	void DealDamage(const FHitResult& Impact, float DamageAmount, const FVector& ShootDir);

	// What is the damage type of this weapon?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageTypeClass;

	// What is the "ammo" for this weapon? 
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Weapon")
	int32 Ammo;

	// What is the ammo type for this weapon?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	EAmmoType AmmoType;

	// How much ammo is consumed per shot?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	uint8 AmmoPerShot;

	// How much ammo do we need to fire a shot?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	int32 MinAmmoRequiredToShoot;

	// Does this weapon consume ammo when fired?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	bool bConsumesAmmo;

	// How much ammo does this weapon start with?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	int32 StartingAmmo;

	// Maping for damage multiplier, use this if you want specific bonuses/penalties to apply when a weapon does damage against something.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	TMap<UPhysicalMaterial*, float> DamageMultiplierMap;
};
