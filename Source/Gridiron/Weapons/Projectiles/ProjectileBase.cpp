// Created by Bruce Crum


#include "Gridiron/Weapons/Projectiles/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Gridiron/FX/SurfaceReaction.h"

// Sets default values
AProjectileBase::AProjectileBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	CollisionComp = CreateOptionalDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->SetGenerateOverlapEvents(false);

	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 5000.f;
	ProjectileMovement->MaxSpeed = 5000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	InitialVelocity = FVector::ZeroVector;
	ProjectileLifepsan = 6.f;
	DamageTypeClass = nullptr;
	DirectDamage = 10.f;
	bDestroyOnHit = true;
	MaxAmountOfBounces = 0;
	SurfaceReaction = nullptr;
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	// Projectiles should be removed from the world if it hasn't hit anything within ProjectileLifespan's seconds
	SetLifeSpan(ProjectileLifepsan);
}

void AProjectileBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CollisionComp->IgnoreActorWhenMoving(this, true);

	if (GetInstigator())
	{
		CollisionComp->IgnoreActorWhenMoving(GetInstigator(), true);
	}

	if (CollisionComp)
	{
		CollisionComp->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	}
}

void AProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AProjectileBase, InitialVelocity, COND_InitialOnly);
}

void AProjectileBase::InitVelocity(const FVector& Direction)
{
	if (ProjectileMovement)
	{
		InitialVelocity = Direction * ProjectileMovement->InitialSpeed;
	}
}

void AProjectileBase::OnRep_InitialVelocity()
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = InitialVelocity;
	}
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	HandleImpact(Hit);
}

void AProjectileBase::HandleImpact(const FHitResult& Impact)
{
	if (HasAuthority())
	{
		MulticastHit(Impact);

		if (Impact.GetActor())
		{
			FPointDamageEvent DamageEvent;
			DamageEvent.DamageTypeClass = DamageTypeClass;
			DamageEvent.HitInfo = Impact;
			DamageEvent.ShotDirection = ProjectileMovement->Velocity.GetSafeNormal();
			DamageEvent.Damage = DirectDamage;

			Impact.GetActor()->TakeDamage(DirectDamage, DamageEvent, GetInstigatorController(), this);
		}

		if (bDestroyOnHit)
		{
			Destroy();
		}
		else if (ProjectileMovement && ProjectileMovement->bShouldBounce && MaxAmountOfBounces > 0)
		{
			++CurrentAmountOfBounces;
			if (CurrentAmountOfBounces >= MaxAmountOfBounces)
			{
				Destroy();
			}
		}
	}
}

void AProjectileBase::MulticastHit_Implementation(const FHitResult& Hit)
{
#if !UE_SERVER
	if (!SurfaceReaction)
	{
		return;
	}

	const auto SurfaceReactionInst = SurfaceReaction.GetDefaultObject();
	if (!SurfaceReactionInst)
	{
		return;
	}

	FSurfaceReactionInfo Info = SurfaceReactionInst->GetSurfaceReactionFromHit(Hit.PhysMaterial);

	if (Info.ReactionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Info.ReactionSound, Hit.ImpactPoint);
	}

	if (Info.ReactionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Info.ReactionEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}
#endif
}

