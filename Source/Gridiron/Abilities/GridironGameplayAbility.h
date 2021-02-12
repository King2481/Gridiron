// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GridironGameplayAbility.generated.h"

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None			UMETA(DisplayName = "None"),
	Confirm			UMETA(DisplayName = "Confirm"),
	Cancel			UMETA(DisplayName = "Cancel"),
	Fire			UMETA(DisplayName = "Fire"), // Left Mouse
	AltFire			UMETA(DisplayName = "AltFire"), // Right Mouse
	ThrowGrenade    UMETA(DisplayName = "ThrowGrenade"), // Q
	Dash			UMETA(DisplayName = "Dash"), // Shift
	Slide			UMETA(DisplayName = "Slide"), // Left Alt
	Crouch			UMETA(DisplayName = "Crouch"), // Left Ctrl
	Jump            UMETA(DisplayName = "Jump") // Space Bar
};

/**
 * 
 */
UCLASS()
class GRIDIRON_API UGridironGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UGridironGameplayAbility();

	// What is the input ID associated with this ability?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	EAbilityInputID InputID;
	
};
