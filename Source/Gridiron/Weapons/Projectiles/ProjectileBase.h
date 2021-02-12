// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class UProjectileMovementComponent;

UCLASS()
class GRIDIRON_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:

	// Sets default values for this actor's properties
	AProjectileBase();

	// Called after all components have been initialized.
	virtual void PostInitializeComponents() override;

	// Intializes the velocity of the weapon.
	void InitVelocity(const FVector& Direction);

	// Replication setup
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovement;
	
	// What is the initial velocity of this weapon?
	UPROPERTY(BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_InitialVelocity, Category = "Movement")
	FVector InitialVelocity;

	UFUNCTION()
	void OnRep_InitialVelocity();

	// How long does this projectile last before it destroys itself?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float ProjectileLifepsan;

};
