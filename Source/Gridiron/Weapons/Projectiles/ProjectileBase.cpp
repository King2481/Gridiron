// Created by Bruce Crum


#include "Gridiron/Weapons/Projectiles/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectileBase::AProjectileBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 5000.f;
	ProjectileMovement->MaxSpeed = 5000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	InitialVelocity = FVector::ZeroVector;
	ProjectileLifepsan = 6.f;
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

