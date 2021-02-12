// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "Gridiron/Items/ItemBase.h"
#include "Gridiron/Items/EquipableEnums.h"
#include "ItemEquipable.generated.h"

class AGridironCharacter;

UENUM(BlueprintType)
enum class EEquipableState : uint8
{
	Unequipped 	  UMETA(DisplayName = "Unequipped"),
	Unequipping   UMETA(DisplayName = "Unequipping"),
	Equipping	  UMETA(DisplayName = "Equipping"),
	Equipped      UMETA(DisplayName = "Equipped")
};

USTRUCT(BlueprintType)
struct FAnimationPair
{
	GENERATED_BODY();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* FirstPersonAnim;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* ThirdPersonAnim;

	FAnimationPair()
	{
		FirstPersonAnim = nullptr;
		ThirdPersonAnim = nullptr;
	}
};


/**
 * 
 */
UCLASS()
class GRIDIRON_API AItemEquipable : public AItemBase
{
	GENERATED_BODY()

public:

	AItemEquipable();

	// Called when we equip
	virtual void Equip();

	// Called when we unequip
	virtual void Unequip();

	// Can we equip this item?
	virtual bool CanEquip() const;

	// Initializes the item
	virtual void InitItem(AGridironCharacter* NewOwner) override;

	// The item slot
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Equipable")
	EItemSlot ItemSlot;

	// Set the equipable state.
	void SetEquipableState(EEquipableState NewState);

	// Returns the current equipable state of the weapon, will return simulated equippable state on remote clients.
	UFUNCTION(BlueprintPure, Category = "Equipable")
	EEquipableState GetEquipableState() const;

	// Checks to see if we are currently swapping to this weapon.
	UFUNCTION(BlueprintPure, Category = "Equipable")
	bool IsSwappingTo() const;

protected:

	// Variable replication setup.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// The current equipable state of the item.
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_EquipableState, BlueprintReadOnly, Category = "Equipable")
	EEquipableState EquipableState;

	// The simulated equipable state of the item (clients)
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	EEquipableState SimulatedEquipableState;

	UFUNCTION()
	void OnRep_EquipableState();

	// Called when the equipable state has changed.
	void OnEquipableStateChanged();

	// Called when equipped.
	virtual void OnEquipped();

	// Called when Unequipped
	virtual void OnUnequipped();

	// Called when we have finished swapping to this item.
	void OnSwapToFinished();

	// How much time does it take to swap to this weapon?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Equipable")
	float SwapToTime;

	UPROPERTY()
	FTimerHandle SwapToTimerHandle;

	// The equip to (ie: switching to) animation pair for this item.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	FAnimationPair EquipAnimationPair;
	
};
