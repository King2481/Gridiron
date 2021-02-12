// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "Gridiron/Weapons/ItemWeapon.h"
#include "ItemFirearm.generated.h"

class AProjectileBase;

UENUM(BlueprintType)
enum class EFirearmType : uint8
{
	Automatic UMETA(DisplayName = "Automatic"),
	Cycle     UMETA(DisplayName = "Cycle"),
	SemiAuto  UMETA(DisplayName = "Semi-Auto"),
	Burst     UMETA(DisplayName = "Burst")
};

UENUM(BlueprintType)
enum class EFirearmCycleState : uint8
{
	Ready         UMETA(DisplayName = "Ready"),
	NeedsCycling  UMETA(DisplayName = "NeedsCycling"),
	Cycling       UMETA(DisplayName = "Cycling")
};

USTRUCT(BlueprintType)
struct FFirearmConfig
{
	GENERATED_BODY()

	// How far do bullet traces go?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Range;

	// How many bullets is fired per round?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 BulletsPerRound;

	// What is the Min/Max spread of this weapon? X represents min and Y represents max
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector2D SpreadData;

	// What is the damage model for this weapon?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Damage;

	// In rounds per minute, what is the rate of fire for this weapon?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float RateOfFire;

	// If a cycle based weapon, how long does it take to "cycle" a round?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CycleTime;

	// How many bullets are bursted? (if burst based weapon)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 MaxBurst;

	FFirearmConfig()
	{
		// General
		Range = 10000.f;
		BulletsPerRound = 1;
		SpreadData = FVector2D();
		Damage = 20.f;
		RateOfFire = 750.f;
		MaxBurst = 3;
		CycleTime = 1.f;
	}
};

USTRUCT(BlueprintType)
struct FStoredFirearmHit
{
	GENERATED_BODY()

	// The actor that we had hit.
	UPROPERTY()
	TWeakObjectPtr<AActor> HitActor;

	// The physical material that we hit.
	UPROPERTY()
	TWeakObjectPtr<UPhysicalMaterial> PhysMaterial;

	// Origin of the shot
	UPROPERTY()
	FVector_NetQuantize Origin;

	// Where the shot landed
	UPROPERTY()
	FVector_NetQuantize ImpactPoint;

	// The impact normal of the hit
	UPROPERTY()
	FVector_NetQuantizeNormal ImpactNormal;

	// The direction of the shot
	UPROPERTY()
	FVector_NetQuantize ShotDirection;

	FStoredFirearmHit()
	{
		HitActor = nullptr;
		PhysMaterial = nullptr;
		Origin = FVector_NetQuantize();
		ImpactPoint = FVector_NetQuantize();
		ImpactNormal = FVector_NetQuantize();
		ShotDirection = FVector_NetQuantize();
	}

	FStoredFirearmHit(TWeakObjectPtr<AActor> StoredActor, TWeakObjectPtr<UPhysicalMaterial> StoredMaterial, const FVector& StoredOrigin, const FVector& StoredImpactPoint, const FVector& StoredImpactNormal, const FVector& StoredShotDirection)
	{
		HitActor = StoredActor;
		PhysMaterial = StoredMaterial;
		Origin = StoredOrigin;
		ImpactPoint = StoredImpactPoint;
		ImpactNormal = StoredImpactNormal;
		ShotDirection = StoredShotDirection;
	}
};


/**
 * 
 */
UCLASS()
class GRIDIRON_API AItemFirearm : public AItemWeapon
{
	GENERATED_BODY()
	
public:

	AItemFirearm();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Are we allowed to "fire" this weapon?
	virtual bool AllowFire() const override;

	// Called when we press the trigger
	UFUNCTION(BlueprintCallable, Category = "Firearm")
	void OnTriggerPressed();

	// Called when we release the trigger
	UFUNCTION(BlueprintCallable, Category = "Firearm")
	void OnTriggerReleased();

	// Will attempt to fire this weapon.
	UFUNCTION(BlueprintCallable, Category = "Firearm")
	void FireWeapon();

	// Called when we equip the item
	virtual void OnEquipped() override;

	// Called when we unequip the item
	virtual void OnUnequipped() override;

	// Is this weapon semi automatic?
	UFUNCTION(BlueprintPure, Category = "Firearm")
	bool IsSemiAutomatic() const;

	// Is this weapon fully automatic?
	UFUNCTION(BlueprintPure, Category = "Firearm")
	bool IsFullyAutomatic() const;

	// Is this weapon cycle based (ie: pump or bolt action)?
	UFUNCTION(BlueprintPure, Category = "Firearm")
	bool IsCycleBased() const;

	// Is this weapon burst based?
	UFUNCTION(BlueprintPure, Category = "Firearm")
	bool IsBurstBased() const;

	// Are we currently bursting at the moment?
	UFUNCTION(BlueprintPure, Category = "Firearm")
	bool HasStartedBursting() const;

	// Are we currently cycling?
	UFUNCTION(BlueprintPure, Category = "Firearm")
	bool IsCycling() const;

	// Does this weapon need to cycle in order to fire again?
	UFUNCTION(BlueprintPure, Category = "Firearm")
	bool DoesWeaponNeedCycling() const;

	// What is the rate of fire for this weapon?
	UFUNCTION(BlueprintPure, Category = "Firearm")
	float GetRateOfFire() const;

	// Does this weapon fire a projectile?
	bool HasProjectile() const;

	// Returns the muzzle location for the weapon
	FVector GetMuzzleLocation(const bool bFirstPerson = true) const;

protected:

	void FireBullets();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireBullets();
	void ServerFireBullets_Implementation();
	bool ServerFireBullets_Validate();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFireBullets();
	void MulticastFireBullets_Implementation();

	// Fires a projectile based on the pawns current view location.
	void FireProjectile();

	// Server RPC for firing a projectile.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireProjectile(const FVector_NetQuantize& Location, const FVector_NetQuantizeNormal& ShootDir);
	void ServerFireProjectile_Implementation(const FVector_NetQuantize& Location, const FVector_NetQuantizeNormal& ShootDir);
	bool ServerFireProjectile_Validate(const FVector_NetQuantize& Location, const FVector_NetQuantizeNormal& ShootDir);

	// Actual trace for the firearm
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const;

	// Process whatever we just hit.
	void ProcessInstantHits(const TArray<FStoredFirearmHit>& Hits);

	// Notify the server that we hit something and needs validation.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerNotifyHit(const TArray<FStoredFirearmHit>& Hits);
	void ServerNotifyHit_Implementation(const TArray<FStoredFirearmHit>& Hits);
	bool ServerNotifyHit_Validate(const TArray<FStoredFirearmHit>& Hits);

	// Called when a hit was confirmed. (this uses the whole stored hits and assume all are valid)
	void ConfirmedFirearmHits(const TArray<FStoredFirearmHit>& Hits);

	// Called when a hit was confirmed.
	void ConfirmedFirearmHit(const FStoredFirearmHit& Hit);

	// Validates a possible firearm hit
	bool ValidateFirearmHit(const FStoredFirearmHit& Hit) const;

	// Calculates the bullet damage based on what we hit.
	float CalculateDamageFromBulletHit(TWeakObjectPtr<AActor> HitActor, TWeakObjectPtr<UPhysicalMaterial> HitMaterial) const;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnFXForHits(const TArray<FStoredFirearmHit>& Hits);
	void MulticastSpawnFXForHits_Implementation(const TArray<FStoredFirearmHit>& Hits);

	// Are we currently holding the trigger down?
	UPROPERTY(BlueprintReadOnly, Category = "Firearm")
	bool bHoldingTrigger;

	// Firearm config for this firearm.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firearm")
	FFirearmConfig FirearmConfig;

	// If we use a projectile class, state it here. Will replace Hitscan if valid.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Firearm")
	TSubclassOf<AProjectileBase> ProjectileClass;

	// What is the firearm type of this weapon?
	UPROPERTY(EditDefaultsOnly, Category = "Firearm")
	EFirearmType FirearmType;

	// What is the firearm type of this weapon?
	UPROPERTY(BlueprintReadOnly, Category = "Firearm")
	EFirearmCycleState FirearmCycleState;

	// When was the last time we fired this weapon?
	UPROPERTY(BlueprintReadOnly, Category = "Firearm")
	float LastFireTime;

	// When are we allowed to fire this weapon next?
	UPROPERTY(BlueprintReadOnly, Category = "Firearm")
	float NextFireTime;

	// When is the "cycle" going to be completed? (only applies to cycle based weapons)
	UPROPERTY(BlueprintReadOnly, Category = "Firearm")
	float CycleFinishTime;

	// How many bursts has this weapon perfom? (If burst based)
	UPROPERTY(BlueprintReadOnly, Category = "Firearm")
	uint8 CurrentBursts;

	// How many bullets have we fired during this trigger pull?
	UPROPERTY(BlueprintReadOnly, Category = "Firearm")
	int32 BulletsFiredThisTriggerPull;

	// What is the muzzle socket name for this weapon?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Firearm")
	FName MuzzleSocketName;
};
