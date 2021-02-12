// Created by Bruce Crum


#include "Gridiron/Items/ItemEquipable.h"
#include "Net/UnrealNetwork.h"
#include "Gridiron/Characters/GridironCharacter.h"

AItemEquipable::AItemEquipable()
{
	ItemSlot = EItemSlot::IS_Misc;
	EquipableState = EEquipableState::Unequipped;
	SimulatedEquipableState = EEquipableState::Unequipped;

	SwapToTime = 1.5f;
	FireAbility = nullptr;
	AltFireAbility = nullptr;
}

void AItemEquipable::InitItem(AGridironCharacter* NewOwner)
{
	Super::InitItem(NewOwner);

	SetActorHiddenInGame(true);
}

void AItemEquipable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItemEquipable, EquipableState);
}

void AItemEquipable::Equip()
{
	if (PawnOwner)
	{
		PawnOwner->GiveCharacterAbility(FireAbility);
		PawnOwner->GiveCharacterAbility(AltFireAbility);
		// PawnOwner->SetupFirstPersonWeaponMesh(Mesh1PAsset);
		// PawnOwner->PlayAnimationMontages(EquipAnimationPair.FirstPersonAnim, EquipAnimationPair.ThirdPersonAnim);
	}

	SetActorHiddenInGame(false);

	GetWorldTimerManager().SetTimer(SwapToTimerHandle, this, &AItemEquipable::OnSwapToFinished, SwapToTime);
}

void AItemEquipable::OnSwapToFinished()
{
	SetEquipableState(EEquipableState::Equipped);
}

void AItemEquipable::Unequip()
{
	if (PawnOwner)
	{
		PawnOwner->RemoveCharacterAbility(FireAbility);
		PawnOwner->RemoveCharacterAbility(AltFireAbility);
	}

	SetEquipableState(EEquipableState::Unequipped);
}

void AItemEquipable::OnEquipped()
{

}

void AItemEquipable::OnUnequipped()
{
	SetActorHiddenInGame(true);

	GetWorldTimerManager().ClearTimer(SwapToTimerHandle);
}

void AItemEquipable::SetEquipableState(EEquipableState NewState)
{
	EquipableState = NewState;
	SimulatedEquipableState = NewState;

	OnEquipableStateChanged();
}

EEquipableState AItemEquipable::GetEquipableState() const
{
	if (!HasAuthority())
	{
		return SimulatedEquipableState;
	}

	return EquipableState;
}

void AItemEquipable::OnRep_EquipableState()
{
	if (EquipableState == SimulatedEquipableState)
	{
		return;
	}

	OnEquipableStateChanged();
}

void AItemEquipable::OnEquipableStateChanged()
{
	switch (EquipableState)
	{
		case EEquipableState::Equipping:
		{
			Equip();
			break;
		}

		case EEquipableState::Equipped:
		{
			OnEquipped();
			break;
		}

		case EEquipableState::Unequipping:
		{
			Unequip();
			break;
		}

		case EEquipableState::Unequipped:
		{
			OnUnequipped();
			break;
		}

		default:
		{
			// Do nothing
			break;
		}
	}
}

bool AItemEquipable::CanEquip() const
{
	return true;
}

bool AItemEquipable::IsSwappingTo() const
{
	return GetEquipableState() == EEquipableState::Equipping;
}
