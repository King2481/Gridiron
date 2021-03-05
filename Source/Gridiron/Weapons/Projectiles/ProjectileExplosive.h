// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "Gridiron/Weapons/Projectiles/ProjectileBase.h"
#include "ProjectileExplosive.generated.h"

//class UCameraShake;

USTRUCT(BlueprintType)
struct FExplosionConfig
{
	GENERATED_BODY()

	///////////////////////////////////////////////////////////////////
	// Explosion Config

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ExplosionInnerRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ExplosionOuterRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ExplosionInnerDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ExplosionOuterDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UDamageType> ExplosionDamageTypeClass;

	///////////////////////////////////////////////////////////////////
	// Camera Shake Config

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UCameraShake> ExplosionCameraShakeClass;*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ExplosionCameraShakeInnerRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ExplosionCameraShakeOuterRadius;

	//////////////////////////////////////////////////////////////////
	// FX

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UParticleSystem* ExplosionFX;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	USoundBase* ExplosionSound;

	FExplosionConfig()
	{
		ExplosionInnerRadius = 150.f;
		ExplosionOuterRadius = 450.f;
		ExplosionInnerDamage = 100.f;
		ExplosionOuterDamage = 10.f;
		ExplosionDamageTypeClass = nullptr;

		//ExplosionCameraShakeClass = nullptr;
		ExplosionCameraShakeInnerRadius = 200.f;
		ExplosionCameraShakeOuterRadius = 1000.f;

		ExplosionFX = nullptr;
		ExplosionSound = nullptr;
	}
};

/**
 * 
 */
UCLASS()
class GRIDIRON_API AProjectileExplosive : public AProjectileBase
{
	GENERATED_BODY()

public:

	AProjectileExplosive();

	// Called on actor once everything has been initialized.
	virtual void BeginPlay() override;

protected:

	// Explodes the projectile.
	UFUNCTION(BlueprintCallable, Category = "Projectile Explosive")
	void Explode();

	// Explodes at a specfic location
	void ExplodeAt(const FVector& Location, const FExplosionConfig& InExplosionConfig);

	// Explosion Multicast, plays things like FX and sound.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastExplode(const FVector& Location);
	void MulticastExplode_Implementation(const FVector& Location);

	FTimerHandle ExplodeTimerHandle;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projecile Explosive")
	FExplosionConfig ExplosionConfig;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projecile Explosive")
	TArray<TSubclassOf<AActor>> DirectHitActorsThatCauseImmediateExplosion;

	// Do we explode on impact?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projecile Explosive")
	bool bImpactGrenade;

	// If timed explosion, how long does it take to blow up?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projecile Explosive")
	float ExplosionTime;

	virtual void HandleImpact(const FHitResult& Impact) override;
	
};
