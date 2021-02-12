// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GridironCharacter.generated.h"

UCLASS()
class GRIDIRON_API AGridironCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	// Sets default values for this character's properties
	AGridironCharacter();

	// Are we currently at or beyond our maximum health?
	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsAtOrBeyondMaxHealth() const;

	// Are we currently at or beyond our maximum armor?
	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsAtOrBeyondMaxArmor() const;

	// Restores the characters health by this amount (Server Only)
	UFUNCTION(BlueprintCallable, Category = "Character")
	void RestoreHealth(const float Amount);

	// Adds armor by this amount (Server Only)
	UFUNCTION(BlueprintCallable, Category = "Character")
	void AddArmor(const float Amount);

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Initializes the character
	void InitCharacter();

	// Applies damage to the character.
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// Calculates momentum from when we take damage.
	FVector CalculateMomentumFromDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	// Modifies damage coming into the player. Factors in things like armor, team, etc.
	virtual float ModifyDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	// Damages our armor
	virtual float DamageArmor(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	// Called when we fall out of the world map.
	virtual void FellOutOfWorld(const UDamageType& dmgType) override;

	// Checks to see if we should actually take damage.
	virtual bool ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const override;

	// Attempts to kill the pawn, returns true if successful.
	bool Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	// Checks to see if this pawn can die.
	bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	// Called when a character dies.
	void OnDeath();

	// Function for replication setup.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// The current health of the character.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character")
	float Health;

	// The max health of the character
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character")
	float MaxHealth;

	// The Starting health of the character
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character")
	float StartingHealth;

	// The current armor of the character.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character")
	float Armor;

	// The max armor of the character
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character")
	float MaxArmor;

	// The starting armor of the character
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character")
	float StartingArmor;

	// Is the character dead?
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_IsDying, BlueprintReadOnly, Category = "Character")
	bool bIsDying;

	UFUNCTION()
	void OnRep_IsDying();

	/* Reliably broadcasts a death event to clients, used to apply ragdoll forces */
	UFUNCTION(NetMulticast, Reliable)
	void BroadcastDeath(const FVector_NetQuantize& HitPosition, const FVector_NetQuantize& DamageForce, const FName& BoneName);
	void BroadcastDeath_Implementation(const FVector_NetQuantize& HitPosition, const FVector_NetQuantize& DamageForce, const FName& BoneName);

public:	

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	// Moves the character forward.
	void MoveForward(float Value);

	// Moves the character right.
	void MoveRight(float Value);
};
