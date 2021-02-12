// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "WeaponStatics.generated.h"

class AProjectileBase;

DECLARE_LOG_CATEGORY_EXTERN(LogWeaponGameplayStatics, Log, Verbose);

/**
 * 
 */
UCLASS()
class GRIDIRON_API UWeaponStatics : public UGameplayStatics
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "Weapon Statics")
	static AProjectileBase* FireProjectile(const UObject* WorldContextObject, APawn* Instigator, TSubclassOf<AProjectileBase> ProjectileClass, const FVector& Location, const FVector& ShootDir);

};
