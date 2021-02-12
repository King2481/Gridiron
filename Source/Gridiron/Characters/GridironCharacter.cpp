// Created by Bruce Crum


#include "Gridiron/Characters/GridironCharacter.h"
#include "Gridiron/Characters/GridironMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Gridiron/GameModes/GridironGameModeBase.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "Gridiron/Abilities/GridironGameplayAbility.h"
#include "Gridiron/Items/ItemBase.h"
#include "Gridiron/Items/ItemEquipable.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
AGridironCharacter::AGridironCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGridironMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	bReplicates = true;
	GetMesh()->bOwnerNoSee = true;
	JumpMaxCount = 2;

	Health = 0.f;
	StartingHealth = 100.f;
	MaxHealth = 150.f;
	Armor = 0.f;
	MaxArmor = 100.f;
	StartingArmor = 0.f;
	bIsDying = false;
	CurrentEquipable = nullptr;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->bUsePawnControlRotation = true;

	ArmMesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmMesh1P"));
	ArmMesh1P->SetupAttachment(CameraComponent);
	ArmMesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	ArmMesh1P->CastShadow = false;
	ArmMesh1P->bOnlyOwnerSee = true;

	WeaponMesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh1P"));
	WeaponMesh1P->SetupAttachment(ArmMesh1P);
	WeaponMesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	WeaponMesh1P->CastShadow = false;
	WeaponMesh1P->bOnlyOwnerSee = true;
}

// Called when the game starts or when spawned
void AGridironCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	InitCharacter();
}

void AGridironCharacter::InitCharacter()
{
	// Add the init here since we want clients to initialize the AbilitySystemComponent.
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (!HasAuthority())
	{
		return;
	}

	Health = StartingHealth;
	Armor = StartingArmor;

	if (DefaultWeapons.Num() > 0)
	{
		for (auto& Weapon : DefaultWeapons)
		{
			if (Weapon)
			{
				AddItemToInventory(Weapon);
			}
		}
	}

	EquipFirstAvailableInventoryItem();

	for (auto& Ability : StartingAbilities)
	{
		GiveCharacterAbility(Ability);
	}
}

void AGridironCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GridironMovement = Cast<UGridironMovementComponent>(GetCharacterMovement());
	if (GridironMovement)
	{
		GridironMovement->OwningCharacter = this;
	}
}

void AGridironCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone.
	DOREPLIFETIME(AGridironCharacter, Health);
	DOREPLIFETIME(AGridironCharacter, Armor);
	DOREPLIFETIME(AGridironCharacter, bIsDying);
	DOREPLIFETIME(AGridironCharacter, CurrentEquipable)
	DOREPLIFETIME(AGridironCharacter, Inventory);
}

// Called every frame
void AGridironCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float AGridironCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		FHitResult HitInfo;
		FVector MomentumDir;
		DamageEvent.GetBestHitInfo(this, EventInstigator, HitInfo, MomentumDir);

		const auto GM = GetWorld()->GetAuthGameMode<AGridironGameModeBase>();
		if (GM)
		{
			// Gamemode may want to modify
			ActualDamage = GM->OnCharacterTakeDamage(this, ActualDamage, DamageEvent, EventInstigator, DamageCauser);
		}

		// Apply modifiers. 
		ActualDamage = ModifyDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);

		Health -= ActualDamage;

		if (Armor > 0)
		{
			Armor = FMath::Max<float>(Armor - DamageArmor(ActualDamage, DamageEvent, EventInstigator, DamageCauser), 0);
		}

		if (Health <= 0)
		{
			Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);

			// If we actually are dying, since Die() may return false. Launch our character now.
			if (bIsDying)
			{
				BroadcastDeath(HitInfo.ImpactPoint, MomentumDir * 12500.f, HitInfo.BoneName);
			}
		}

		const FVector Momentum = CalculateMomentumFromDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
		if (!bIsDying)
		{
			LaunchCharacter(Momentum, true, false);
		}

		return ActualDamage;
	}

	return 0.f;
}

FVector AGridironCharacter::CalculateMomentumFromDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	return FVector(0);
}

float AGridironCharacter::ModifyDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float MitigatedDamage = DamageAmount;

	if (Armor >= FLT_EPSILON)
	{
		MitigatedDamage /= 2.f;
	}

	return MitigatedDamage;
}

float AGridironCharacter::DamageArmor(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	return DamageAmount * 3.f;
}

void AGridironCharacter::FellOutOfWorld(const UDamageType& dmgType)
{
	// TODO: Custom Damage Type Class for Falling out of world.
	Die(Health, FDamageEvent(dmgType.GetClass()), GetController(), nullptr);
}

bool AGridironCharacter::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	return Super::ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

bool AGridironCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!CanDie(KillingDamage, DamageEvent, EventInstigator, DamageCauser))
	{
		return false;
	}

	const auto GM = GetWorld()->GetAuthGameMode<AGridironGameModeBase>();
	if (GM)
	{
		// Inform the Game mode.
		GM->OnCharacterKilled(this, KillingDamage, DamageEvent, EventInstigator, DamageCauser);
	}

	OnDeath();

	return true;
}

bool AGridironCharacter::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	if (bIsDying										// already dying
		|| IsPendingKill()								// already destroyed
		|| GetLocalRole() != ROLE_Authority						// not authority
		|| GetWorld()->GetAuthGameMode<AGridironGameModeBase>() == NULL
		|| GetWorld()->GetAuthGameMode<AGridironGameModeBase>()->GetMatchState() == MatchState::LeavingMap) // level transition occurring
	{
		return false;
	}

	return true;
}

void AGridironCharacter::OnDeath()
{
	bIsDying = true;

	DetachFromControllerPendingDestroy();

	SetReplicateMovement(false);
	TearOff();
	SetLifeSpan(30.f);
	DestroyInventoryItems();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGridironCharacter::BroadcastDeath_Implementation(const FVector_NetQuantize& HitPosition, const FVector_NetQuantize& DamageForce, const FName& BoneName)
{
	GetMesh()->SetCollisionProfileName("Ragdoll");
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->AddImpulseAtLocation(DamageForce, HitPosition, BoneName);
}

void AGridironCharacter::OnRep_IsDying()
{
	OnDeath();
}

void AGridironCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Note sure if we need this, but GASDocumentation does this to avoid a race condition, adding just in case.
	BindAbilitySystemToInputComponent();
}

void AGridironCharacter::OnRep_Inventory()
{

}

void AGridironCharacter::OnRep_CurrentEquipable()
{

}

bool AGridironCharacter::IsAtOrBeyondMaxHealth() const
{
	return Health >= MaxHealth;
}

bool AGridironCharacter::IsAtOrBeyondMaxArmor() const
{
	return Armor >= MaxArmor;
}

void AGridironCharacter::RestoreHealth(const float Amount)
{
	if (HasAuthority())
	{
		Health = FMath::Min<float>(Health + Amount, MaxHealth);
	}
}

void AGridironCharacter::AddArmor(const float Amount)
{
	if (HasAuthority())
	{
		Armor = FMath::Min<float>(Armor + Amount, MaxArmor);
	}
}

void AGridironCharacter::DestroyInventoryItems()
{
	if (!HasAuthority())
	{
		return;
	}

	for (int i = Inventory.Num() - 1; i >= 0; i--)
	{
		const auto Item = Inventory[i];
		if (Item)
		{
			Inventory.RemoveSingle(Item);
			Item->Destroy();
		}
	}

	Inventory.Empty();
}

void AGridironCharacter::AddItemToInventory(TSubclassOf<AItemBase> ItemToAdd)
{
	if (!ItemToAdd)
	{
		return;
	}

	const auto Item = GetWorld()->SpawnActor<AItemBase>(ItemToAdd);
	if (Item)
	{
		Item->InitItem(this);
		Inventory.Add(Item);
	}
}

bool AGridironCharacter::HasItemInInventory(TSubclassOf<AItemBase> ItemToFind) const
{
	for (auto& Item : Inventory)
	{
		if (!Item)
		{
			continue;
		}

		if (Item->GetClass() == ItemToFind)
		{
			return true;
		}
	}

	return false;
}

void AGridironCharacter::EquipFirstAvailableInventoryItem()
{
	for (auto& Item : Inventory)
	{
		if (!Item)
		{
			continue;
		}

		const auto Equipable = Cast<AItemEquipable>(Item);
		if (Equipable && Equipable->CanEquip())
		{
			EquipItem(Equipable);
			return;
		}
	}
}

void AGridironCharacter::EquipItem(AItemEquipable* Item)
{
	if (!HasAuthority())
	{
		ServerEquipItem(Item);
	}

	SetCurrentEquipable(Item);
}

void AGridironCharacter::ServerEquipItem_Implementation(AItemEquipable* Item)
{
	EquipItem(Item);
}

bool AGridironCharacter::ServerEquipItem_Validate(AItemEquipable* Item)
{
	return true;
}

void AGridironCharacter::SetCurrentEquipable(AItemEquipable* Item, bool bFromReplication /*= false*/)
{
	if (CurrentEquipable)
	{
		CurrentEquipable->Unequip();
	}

	CurrentEquipable = Item;

	if (CurrentEquipable)
	{
		CurrentEquipable->Equip();
	}
}

USkeletalMeshComponent* AGridironCharacter::GetWeaponMesh1P() const
{
	return WeaponMesh1P;
}

FVector AGridironCharacter::GetCameraLocation() const
{
	if (CameraComponent)
	{
		return CameraComponent->GetComponentLocation();
	}

	return FVector::ZeroVector;
}

bool AGridironCharacter::AllowWeaponSwapping() const
{
	return true;
}

void AGridironCharacter::GiveCharacterAbility(TSubclassOf<UGridironGameplayAbility> Ability)
{
	if (!HasAuthority())
	{
		return;
	}

	if (Ability)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 0, static_cast<int32>(Ability.GetDefaultObject()->InputID), this));
	}
}

void AGridironCharacter::RemoveCharacterAbility(TSubclassOf<UGridironGameplayAbility> Ability)
{
	if (!HasAuthority())
	{
		return;
	}

	if (Ability)
	{
		// Check to see if there is a faster way to do this rather than iterating through the entire array.
		for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (Spec.Ability->GetClass() == Ability)
			{
				AbilitySystemComponent->ClearAbility(FGameplayAbilitySpecHandle(Spec.Handle));
				break;
			}
		}
	}
}

// Called to bind functionality to input
void AGridironCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGridironCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGridironCharacter::MoveRight);

	PlayerInputComponent->BindAxis("MouseX", this, &ThisClass::AddControllerYawInput);
	PlayerInputComponent->BindAxis("MouseY", this, &ThisClass::AddControllerPitchInput);

	PlayerInputComponent->BindAction("SelectWeaponSlotShotgun", IE_Pressed, this, &AGridironCharacter::OnSelectWeaponSlotShotgun);
	PlayerInputComponent->BindAction("SelectWeaponSlotBullet", IE_Pressed, this, &AGridironCharacter::OnSelectWeaponSlotBullet);
	PlayerInputComponent->BindAction("SelectWeaponSlotEnergy", IE_Pressed, this, &AGridironCharacter::OnSelectWeaponSlotEnergy);
	PlayerInputComponent->BindAction("SelectWeaponSlotExplosive", IE_Pressed, this, &AGridironCharacter::OnSelectWeaponSlotExplosive);

	PlayerInputComponent->BindAction("SelectInventoryPrevious", IE_Pressed, this, &AGridironCharacter::OnSelectInventoryPrevious);
	PlayerInputComponent->BindAction("SelectInventoryNext", IE_Pressed, this, &AGridironCharacter::OnSelectInventoryNext);

	BindAbilitySystemToInputComponent();
}

void AGridironCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector(), Value);
}

void AGridironCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

void AGridironCharacter::OnSelectWeaponSlotShotgun()
{
	const auto Item = GetNextItemInSlot(EItemSlot::IS_Shotgun, CurrentEquipable, true);
	if (Item && Item != CurrentEquipable)
	{
		EquipItem(Item);
	}
}

void AGridironCharacter::OnSelectWeaponSlotBullet()
{
	const auto Item = GetNextItemInSlot(EItemSlot::IS_Bullet, CurrentEquipable, true);
	if (Item && Item != CurrentEquipable)
	{
		EquipItem(Item);
	}
}

void AGridironCharacter::OnSelectWeaponSlotEnergy()
{
	const auto Item = GetNextItemInSlot(EItemSlot::IS_Energy, CurrentEquipable, true);
	if (Item && Item != CurrentEquipable)
	{
		EquipItem(Item);
	}
}

void AGridironCharacter::OnSelectWeaponSlotExplosive()
{
	const auto Item = GetNextItemInSlot(EItemSlot::IS_Explosive, CurrentEquipable, true);
	if (Item && Item != CurrentEquipable)
	{
		EquipItem(Item);
	}
}

void AGridironCharacter::OnSelectInventoryPrevious()
{
	if (!CurrentEquipable)
	{
		EquipFirstAvailableInventoryItem();
		return;
	}

	if (!AllowWeaponSwapping())
	{
		return;
	}

	AItemEquipable* ActiveItem = CurrentEquipable;
	AItemEquipable* Item = nullptr;

	// if we have a weapon, we want to try the current slot first, then the next one
	int CurrentSlot = (int)ActiveItem->ItemSlot;
	int OriginalSlot = CurrentSlot;
	Item = GetPreviousItemInSlot((EItemSlot)CurrentSlot, ActiveItem, false);

	// nothing else in the current slot, move on
	while (Item == nullptr)
	{
		--CurrentSlot;

		if (CurrentSlot < (int)EItemSlot::IS_Shotgun)
		{
			return;
		}

		// if we got back to our original slot, give up
		if (CurrentSlot == OriginalSlot)
		{
			return;
		}

		Item = GetPreviousItemInSlot((EItemSlot)CurrentSlot, nullptr, false);
	}

	// invalid item
	if (Item == nullptr)
	{
		return;
	}

	EquipItem(Item);
}

void AGridironCharacter::OnSelectInventoryNext()
{
	if (!CurrentEquipable)
	{
		EquipFirstAvailableInventoryItem();
		return;
	}

	if (!AllowWeaponSwapping())
	{
		return;
	}

	AItemEquipable* ActiveItem = CurrentEquipable;
	AItemEquipable* Item = nullptr;

	// if we have a weapon, we want to try the current slot first, then the next one
	int CurrentSlot = (int)ActiveItem->ItemSlot;
	int OriginalSlot = CurrentSlot;
	Item = GetNextItemInSlot((EItemSlot)CurrentSlot, ActiveItem, false);

	// nothing else in the current slot, move on
	while (Item == nullptr)
	{
		++CurrentSlot;

		// roll over back to primary
		if (CurrentSlot >= (int)EItemSlot::IS_Count)
		{
			return;
		}

		// if we got back to our original slot, give up
		if (CurrentSlot == OriginalSlot)
		{
			return;
		}

		Item = GetNextItemInSlot((EItemSlot)CurrentSlot, nullptr, false);
	}

	// invalid item
	if (Item == nullptr)
	{
		return;
	}

	EquipItem(Item);
}

AItemEquipable* AGridironCharacter::GetNextItemInSlot(EItemSlot Slot, AItemEquipable* CurrentItem, bool bFallbackToFirst)
{
	const bool bCurrentInSameSlot = (CurrentItem && CurrentItem->ItemSlot == Slot); // Are we transitioning from a weapon in the same slot?
	bool bHasPassedCurrentItem = false;
	AItemEquipable* FirstItemInSlot = nullptr;

	// Iterate through our entire inventory
	for (auto Item : Inventory)
	{
		auto Equipable = Cast<AItemEquipable>(Item);

		// Skip null items or anything not matching our slot
		if (!Equipable || Equipable->IsPendingKillPending() || Equipable->ItemSlot != Slot || !Equipable->CanEquip())
		{
			continue;
		}

		// Not transitioning from an item in the same slot, the first item is fine
		if (!bCurrentInSameSlot)
		{
			return Equipable;
		}

		// Record the first item we encounter in this specific slot
		if (FirstItemInSlot == nullptr)
		{
			FirstItemInSlot = Equipable;
		}

		if (!bHasPassedCurrentItem)
		{
			// Flag if we've reached our CurrentItem, the next weapon in this slot is good to use
			bHasPassedCurrentItem = (CurrentItem == Equipable);
		}
		else
		{
			// We've passed our CurrentItem and hit a new weapon in the same slot
			return Equipable;
		}
	}

	// Unable to find a "next" item, assume that we reached the end of our inventory and are looping back to the first
	if (bFallbackToFirst && bHasPassedCurrentItem && FirstItemInSlot != CurrentItem)
	{
		return FirstItemInSlot;
	}

	return nullptr;
}

AItemEquipable* AGridironCharacter::GetPreviousItemInSlot(EItemSlot Slot, AItemEquipable* CurrentItem, bool bFallbackToLast)
{
	const bool bCurrentInSameSlot = (CurrentItem && CurrentItem->ItemSlot == Slot); // Are we transitioning from a weapon in the same slot?
	bool bHasPassedCurrentItem = false;
	AItemEquipable* LastItemInSlot = nullptr;

	// Iterate through our entire inventory backwards
	for (int i = Inventory.Num() - 1; i >= 0; --i)
	{
		auto Item = Cast<AItemEquipable>(Inventory[i]);

		// Skip null items or anything not matching our slot
		if (!Item || Item->ItemSlot != Slot || !Item->CanEquip())
		{
			continue;
		}

		// Not transitioning from an item in the same slot, the first item is fine
		if (!bCurrentInSameSlot)
		{
			return Item;
		}

		// Record the first item we encounter in this specific slot (which will be the last one in the slot)
		if (LastItemInSlot == nullptr)
		{
			LastItemInSlot = Item;
		}

		if (!bHasPassedCurrentItem)
		{
			// Flag if we've reached our CurrentItem, the next weapon in this slot is good to use
			bHasPassedCurrentItem = (CurrentItem == Item);
		}
		else
		{
			// We've passed our CurrentItem and hit a new weapon in the same slot
			return Item;
		}
	}

	// Unable to find a "previous" item, assume that we reached the end of our inventory and are looping back to the last
	if (bFallbackToLast && bHasPassedCurrentItem && LastItemInSlot != CurrentItem)
	{
		return LastItemInSlot;
	}

	return nullptr;
}

void AGridironCharacter::BindAbilitySystemToInputComponent()
{
	if (IsValid(InputComponent))
	{
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(FString("ConfirmTarget"),
			FString("CancelTarget"), FString("EAbilityInputID"), static_cast<int32>(EAbilityInputID::Confirm), static_cast<int32>(EAbilityInputID::Cancel)));
	}
}