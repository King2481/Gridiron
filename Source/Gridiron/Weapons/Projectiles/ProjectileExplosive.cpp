// Created by Bruce Crum


#include "Gridiron/Weapons/Projectiles/ProjectileExplosive.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h" 
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"

#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<int32> CvarDebugExplosion(TEXT("DebugExplosions"), 0, TEXT("Visualise Explosion Info"));
#endif

AProjectileExplosive::AProjectileExplosive()
{
	bImpactGrenade = false;
	ExplosionTime = -1.f;
	bDestroyOnHit = false; // The explosion will handle destroying the actor

	ProjectileMovement->bShouldBounce = true;
}

void AProjectileExplosive::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && ExplosionTime > FLT_EPSILON)
	{
		GetWorldTimerManager().SetTimer(ExplodeTimerHandle, this, &AProjectileExplosive::Explode, 5.f);
	}
}

void AProjectileExplosive::Explode()
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	ExplodeAt(GetActorLocation(), ExplosionConfig);
}

void AProjectileExplosive::ExplodeAt(const FVector& Location, const FExplosionConfig& InExplosionConfig)
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	UGameplayStatics::ApplyRadialDamageWithFalloff(this, InExplosionConfig.ExplosionInnerDamage, InExplosionConfig.ExplosionOuterDamage, GetActorLocation() + FVector(0.f, 0.f, 1.f), InExplosionConfig.ExplosionInnerRadius, InExplosionConfig.ExplosionOuterRadius, 1.f, InExplosionConfig.ExplosionDamageTypeClass, IgnoredActors, this, GetInstigatorController());

	/*if (ExplosionConfig.ExplosionCameraShakeClass)
	{
		UGameplayStatics::PlayWorldCameraShake(this, InExplosionConfig.ExplosionCameraShakeClass, GetActorLocation(), InExplosionConfig.ExplosionCameraShakeInnerRadius, InExplosionConfig.ExplosionCameraShakeOuterRadius);
	}*/

	MulticastExplode(Location);

	Destroy();
}

void AProjectileExplosive::MulticastExplode_Implementation(const FVector& Location)
{
#if !UE_SERVER

	if (GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		// Don't play on dedicated.
		return;
	}

	if (ExplosionConfig.ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionConfig.ExplosionSound, Location);
	}

	if (ExplosionConfig.ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionConfig.ExplosionFX, Location);
	}

#if !UE_BUILD_SHIPPING
	if (CvarDebugExplosion.GetValueOnGameThread() > 0)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, Location + FVector(0.f, 0.f, 1.f), ExplosionConfig.ExplosionInnerRadius, 12, FColor::Red, 5.f, 1.f);
		UKismetSystemLibrary::DrawDebugSphere(this, Location + FVector(0.f, 0.f, 1.f), ExplosionConfig.ExplosionOuterRadius, 12, FColor::Yellow, 5.f, 1.f);
	}
#endif

#endif	
}

void AProjectileExplosive::HandleImpact(const FHitResult& Impact)
{
	Super::HandleImpact(Impact);

	if (HasAuthority())
	{
		bool bContextualImpact = false;

		if (DirectHitActorsThatCauseImmediateExplosion.Num() > 0 && Impact.GetActor())
		{
			for (auto& Class : DirectHitActorsThatCauseImmediateExplosion)
			{
				if (Class && Impact.GetActor()->GetClass()->IsChildOf(Class))
				{
					bContextualImpact = true;
					break;
				}
			}
		}

		if (bImpactGrenade || bContextualImpact && ProjectileMovement->Velocity.Size() > 10.f)
		{
			Explode();
			return;
		}
	}
}
