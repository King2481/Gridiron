// Created by Bruce Crum


#include "Gridiron/Weapons/WeaponStatics.h"
#include "Gridiron/Weapons/Projectiles/ProjectileBase.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogWeaponGameplayStatics);

AProjectileBase* UWeaponStatics::FireProjectile(const UObject* WorldContextObject, APawn* Instigator, TSubclassOf<AProjectileBase> ProjectileClass, const FVector& Location, const FVector& ShootDir)
{
	const auto World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		UE_LOG(LogWeaponGameplayStatics, Error, TEXT("Error: UWeaponGameplayStatics::FireProjectile() failed (Invalid World)"));
	}

	if (!ProjectileClass)
	{
		UE_LOG(LogWeaponGameplayStatics, Error, TEXT("Error: UWeaponGameplayStatics::FireProjectile() failed (Projectile Class Invalid)"));
		return nullptr;
	}

	if (!World)
	{
		UE_LOG(LogWeaponGameplayStatics, Error, TEXT("Error: UWeaponGameplayStatics::FireProjectile(), failed to fire projectile %s (World is nullptr)"), *ProjectileClass->GetName());
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Instigator;
	SpawnParams.Owner = Instigator;

	const FTransform SpawnTransform = FTransform(ShootDir.Rotation(), Location);

	AProjectileBase* Projectile = World->SpawnActor<AProjectileBase>(ProjectileClass, SpawnTransform, SpawnParams);
	if (Projectile)
	{
		Projectile->InitVelocity(ShootDir);
		return Projectile;
	}

	UE_LOG(LogWeaponGameplayStatics, Error, TEXT("Error: UWeaponGameplayStatics::FireProjectile(), failed to fire projectile %s (Failed to spawn)"), *ProjectileClass->GetName());
	return nullptr;
}
