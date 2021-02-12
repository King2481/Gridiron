// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GridironCharacter.generated.h"

class UAbilitySystemComponent;
class UGridironGameplayAbility;
class UGridironMovementComponent;
class AItemBase;
class AItemEquipable;
class UCameraComponent;

UCLASS()
class GRIDIRON_API AGridironCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	// Sets default values for this character's properties
	AGridironCharacter(const FObjectInitializer& ObjectInitializer);

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

	// Adds an item to our inventory
	UFUNCTION(BlueprintCallable, Category = "Character")
	void AddItemToInventory(TSubclassOf<AItemBase> ItemToAdd);

	// Checks to see if an item class exists in our iventory
	UFUNCTION(BlueprintPure, Category = "Character")
	bool HasItemInInventory(TSubclassOf<AItemBase> ItemToFind) const;

	// Attempts to equip the first available inventory item.
	void EquipFirstAvailableInventoryItem();

	// Equips an item
	void EquipItem(AItemEquipable* Item);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipItem(AItemEquipable* Item);
	void ServerEquipItem_Implementation(AItemEquipable* Item);
	bool ServerEquipItem_Validate(AItemEquipable* Item);

	// Sets the current equipable
	void SetCurrentEquipable(AItemEquipable* Item, bool bFromReplication = false);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UAbilitySystemComponent* AbilitySystemComponent;

	// The Default abilities this character starts with.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGridironGameplayAbility>> StartingAbilities;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when all the components have been initialized
	virtual void PostInitializeComponents() override;

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

	virtual void OnRep_PlayerState() override;

	// Cache of the movement component
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	UGridironMovementComponent* GridironMovement;

	// Destroys all inventory items. Must be called on Authority
	void DestroyInventoryItems();

	// What item is currently equipped for this character?
	UPROPERTY(Replicated, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentEquipable, Category = "Character")
	AItemEquipable* CurrentEquipable;

	UFUNCTION()
	void OnRep_CurrentEquipable();

	// The characters inventory
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_Inventory, Category = "Inventory")
	TArray<AItemBase*> Inventory;

	UFUNCTION()
	void OnRep_Inventory();

	// The default weapon this character starts with.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character")
	TSubclassOf<AItemEquipable> DefaultWeapon;

	// The Camera for this character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComponent;

	/** First person character mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* ArmMesh1P;

	/** Weapon mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* WeaponMesh1P;

public:	

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	// Binds the InputComponent to the AbilitySystemComponent to allow abilities to read inputs.
	void BindAbilitySystemToInputComponent();

	// Moves the character forward.
	void MoveForward(float Value);

	// Moves the character right.
	void MoveRight(float Value);
};
