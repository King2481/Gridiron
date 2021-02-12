// Created by Bruce Crum


#include "Gridiron/Weapons/ItemFirearm.h"
#include "Gridiron/Characters/GridironCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Gridiron/Weapons/WeaponStatics.h"
#include "Gridiron/GameModes/GridironGameModeBase.h"
#include "Gridiron/Weapons/Projectiles/ProjectileBase.h"
#include "Gridiron/Engine/GameTraceChannels.h"

#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<int32> CvarShowWeaponTraces(TEXT("DebugWeaponTraces"), 0, TEXT("Visualise Firearm Traces"));
#endif

AItemFirearm::AItemFirearm()
{
	PrimaryActorTick.bCanEverTick = true;

	bHoldingTrigger = false;
	FirearmType = EFirearmType::Automatic;
	FirearmCycleState = EFirearmCycleState::Ready;
	CurrentBursts = 0;
	BulletsFiredThisTriggerPull = 0;
	ProjectileClass = nullptr;
	MuzzleSocketName = FName("Muzzle");

	LastFireTime = -1.f;
	NextFireTime = -1.f;
	CycleFinishTime = -1.f;

	StartingAmmo = 10;
}

void AItemFirearm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const bool bIsLocallyControlled = IsPawnOwnerLocallyControlled();
	const bool bIsBursting = HasStartedBursting();

	if (IsFullyAutomatic() && bHoldingTrigger)
	{
		if (bIsLocallyControlled && AllowFire())
		{
			FireWeapon();
		}
	}

	if (IsBurstBased())
	{
		if (bIsLocallyControlled && bIsBursting && AllowFire())
		{
			FireWeapon();
		}
	}

	if (IsCycleBased())
	{
		if (IsCycling() && GetWorld() && GetWorld()->TimeSeconds >= CycleFinishTime)
		{
			FirearmCycleState = EFirearmCycleState::Ready;
		}
	}
}

void AItemFirearm::OnEquipped()
{
	Super::OnEquipped();

	// We can tick now.
	SetActorTickEnabled(true);

	if (DoesWeaponNeedCycling())
	{
		FirearmCycleState = EFirearmCycleState::Cycling;
	}
}

void AItemFirearm::OnUnequipped()
{
	Super::OnUnequipped();

	// Disable tick, no need for to tick every frame if we are not using it.
	SetActorTickEnabled(false);

	if (IsCycling())
	{
		FirearmCycleState = EFirearmCycleState::NeedsCycling;
	}
}

void AItemFirearm::FireWeapon()
{
	FireBullets();
}

void AItemFirearm::FireBullets()
{
	if (!HasAuthority())
	{
		ServerFireBullets();
	}
	else
	{
		// Multicast for the weapon fire sounds
		MulticastFireBullets();
	}

	if (IsPawnOwnerLocallyControlled())
	{
		if (HasProjectile())
		{
			// Fires a projectile, don't do bullet traces.
			FireProjectile();
		}
		else
		{
			// Calculate it here so it's already cached.
			const FVector AimDir = GetAdjustedAim();
			const FVector StartTrace = GetCameraDamageStartLocation();

			TArray<FStoredFirearmHit> StoredHits;

			for (uint8 i = 0; i < FirearmConfig.BulletsPerRound; i++)
			{
				const int32 RandomSeed = FMath::Rand();
				FRandomStream WeaponRandomStream(RandomSeed);
				const float CurrentSpread = 1.f;
				const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * FMath::RandRange(FirearmConfig.SpreadData.X, FirearmConfig.SpreadData.Y));

				const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
				const FVector EndTrace = StartTrace + (ShootDir * FirearmConfig.Range);

				const FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
				StoredHits.Add(FStoredFirearmHit(Impact.Actor, Impact.PhysMaterial, StartTrace, Impact.ImpactPoint, Impact.ImpactNormal, ShootDir));
			}

			ProcessInstantHits(StoredHits);
		}
	}

	if (!GetWorld())
	{
		return;
	}

	++BulletsFiredThisTriggerPull;

	LastFireTime = GetWorld()->TimeSeconds;
	NextFireTime = GetWorld()->TimeSeconds + GetRateOfFire();

	DecrementAmmo();

	// Cycle based weapon check, if it is, a round needs to be "cycled" in before we can fire.
	if (IsCycleBased())
	{
		FirearmCycleState = EFirearmCycleState::Cycling;
		CycleFinishTime = GetWorld()->TimeSeconds + FirearmConfig.CycleTime;
	}

	if (IsBurstBased())
	{
		++CurrentBursts;
		if (CurrentBursts >= FirearmConfig.MaxBurst)
		{
			CurrentBursts = 0;
		}
	}
}

void AItemFirearm::ServerFireBullets_Implementation()
{
	FireBullets();
}

bool AItemFirearm::ServerFireBullets_Validate()
{
	return true;
}

void AItemFirearm::MulticastFireBullets_Implementation()
{
#if !UE_SERVER
	// Dont allow for recursion, this is for remote clients only.
	if (HasAuthority() || IsPawnOwnerLocallyControlled())
	{
		return;
	}
#endif
}

void AItemFirearm::FireProjectile()
{
	const FVector Location = GetMuzzleLocation();
	const FVector AimDir = GetAdjustedAim();

	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float ConeHalfAngle = FMath::DegreesToRadians(1.f * FMath::RandRange(FirearmConfig.SpreadData.X, FirearmConfig.SpreadData.Y));
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);

	ServerFireProjectile(Location, ShootDir);
}

void AItemFirearm::ServerFireProjectile_Implementation(const FVector_NetQuantize& Location, const FVector_NetQuantizeNormal& ShootDir)
{
	if (PawnOwner)
	{
		UWeaponStatics::FireProjectile(this, PawnOwner, ProjectileClass, Location, ShootDir);
	}
}

bool AItemFirearm::ServerFireProjectile_Validate(const FVector_NetQuantize& Location, const FVector_NetQuantizeNormal& ShootDir)
{
	return true;
}

FHitResult AItemFirearm::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	// Perform trace to retrieve hit info
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(PawnOwner);
	Params.bTraceComplex = false;
	Params.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_Weapon, Params);

#if !UE_BUILD_SHIPPING
	if (CvarShowWeaponTraces.GetValueOnGameThread() > 0)
	{
		UKismetSystemLibrary::DrawDebugLine(GetWorld(), StartTrace, EndTrace, FLinearColor::Red, 1.f, 1.f);

		if (Hit.GetActor())
		{
			UKismetSystemLibrary::DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 25.f, 12, FLinearColor::Yellow, 1.f, 1.f);
		}
	}
#endif

	return Hit;
}


void AItemFirearm::ProcessInstantHits(const TArray<FStoredFirearmHit>& Hits)
{
	if (PawnOwner && PawnOwner->IsLocallyControlled() && GetNetMode() == NM_Client)
	{
		// notify the server of the hit
		ServerNotifyHit(Hits);
		return;
	}

	if (HasAuthority())
	{
		MulticastSpawnFXForHits(Hits);
	}

	// Not a client, go ahead and confirm the hit.
	ConfirmedFirearmHits(Hits);
}

void AItemFirearm::ServerNotifyHit_Implementation(const TArray<FStoredFirearmHit>& Hits)
{
	for (auto& Hit : Hits)
	{
		if (ValidateFirearmHit(Hit))
		{
			ConfirmedFirearmHit(Hit);
		}
	}

	MulticastSpawnFXForHits(Hits);
}

bool AItemFirearm::ServerNotifyHit_Validate(const TArray<FStoredFirearmHit>& Hits)
{
	return true;
}

bool AItemFirearm::ValidateFirearmHit(const FStoredFirearmHit& Hit) const
{
	if (!PawnOwner)
	{
		UE_LOG(LogWeapon, Warning, TEXT("%s Rejected client side hit of %s (No Pawn Owner)"), *GetNameSafe(this), *GetNameSafe(Hit.HitActor.Get()));
		return false;
	}

	const float DistanceFromHit = (Hit.Origin - Hit.ImpactPoint).Size();
	if (DistanceFromHit > FirearmConfig.Range)
	{
		UE_LOG(LogWeapon, Warning, TEXT("%s Rejected client side hit of %s (Out of range)"), *GetNameSafe(this), *GetNameSafe(Hit.HitActor.Get()));
		return false;
	}

	const float DistanceFromPlayer = (Hit.Origin - PawnOwner->GetPawnViewLocation()).Size();
	if (DistanceFromPlayer > 5000.f)
	{
		UE_LOG(LogWeapon, Warning, TEXT("%s Rejected client side hit of %s (Shot origin too far from player)"), *GetNameSafe(this), *GetNameSafe(Hit.HitActor.Get()));
		return false;
	}

	return true;
}

void AItemFirearm::ConfirmedFirearmHits(const TArray<FStoredFirearmHit>& Hits)
{
	for (auto& Hit : Hits)
	{
		ConfirmedFirearmHit(Hit);
	}
}

void AItemFirearm::ConfirmedFirearmHit(const FStoredFirearmHit& Hit)
{
	// handle damage
	if (ShouldDealDamage(Hit.HitActor.Get()))
	{
		FHitResult Impact;
		Impact.Actor = Hit.HitActor;
		Impact.PhysMaterial = Hit.PhysMaterial;
		Impact.ImpactPoint = Hit.ImpactPoint;
		Impact.TraceStart = Hit.Origin;
		Impact.ImpactNormal = Hit.ImpactNormal;

		const float Damage = CalculateDamageFromBulletHit(Hit.HitActor, Hit.PhysMaterial);
		DealDamage(Impact, Damage, Hit.ShotDirection);
	}
}

void AItemFirearm::MulticastSpawnFXForHits_Implementation(const TArray<FStoredFirearmHit>& Hits)
{
#if !UE_SERVER

#endif
}

float AItemFirearm::CalculateDamageFromBulletHit(TWeakObjectPtr<AActor> HitActor, TWeakObjectPtr<UPhysicalMaterial> HitMaterial) const
{
	return FirearmConfig.Damage * GetDamageMultiplier(HitActor, HitMaterial);
}

bool AItemFirearm::HasStartedBursting() const
{
	return CurrentBursts > 0;
}

bool AItemFirearm::IsSemiAutomatic() const
{
	return FirearmType == EFirearmType::SemiAuto;
}

bool AItemFirearm::IsFullyAutomatic() const
{
	return FirearmType == EFirearmType::Automatic;
}

bool AItemFirearm::IsCycleBased() const
{
	return FirearmType == EFirearmType::Cycle;
}

bool AItemFirearm::IsBurstBased() const
{
	return FirearmType == EFirearmType::Burst;
}

bool AItemFirearm::IsCycling() const
{
	return FirearmCycleState == EFirearmCycleState::Cycling;
}

bool AItemFirearm::DoesWeaponNeedCycling() const
{
	return FirearmCycleState == EFirearmCycleState::NeedsCycling;
}

bool AItemFirearm::HasProjectile() const
{
	return IsValid(ProjectileClass);
}

bool AItemFirearm::AllowFire() const
{
	if (!GetWorld() || GetWorld()->TimeSeconds < NextFireTime)
	{
		// Not a valid world or not allowed to fire yet.
		return false;
	}

	if (IsCycling())
	{
		// Cycling, can't fire.
		return false;
	}

	return Super::AllowFire();
}

float AItemFirearm::GetRateOfFire() const
{
	return 60.f / FirearmConfig.RateOfFire;
}

FVector AItemFirearm::GetMuzzleLocation(const bool bFirstPerson /* = true */) const
{
	if (bFirstPerson)
	{
		if (PawnOwner)
		{
			const auto Mesh = PawnOwner->GetWeaponMesh1P();
			if (Mesh && Mesh->DoesSocketExist(MuzzleSocketName))
			{
				return Mesh->GetSocketLocation(MuzzleSocketName);
			}
		}
	}

	if (Mesh3P && Mesh3P->DoesSocketExist(MuzzleSocketName))
	{
		return Mesh3P->GetSocketLocation(MuzzleSocketName);
	}

	UE_LOG(LogWeapon, Warning, TEXT("%s: tried to get weapon muzzle location, but the third person mesh doesn't have a socket named 'Muzzle'."), *GetNameSafe(this));

	return FVector::ZeroVector;
}
