// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GridironMovementComponent.generated.h"

class AGridironCharacter;

/**
 * 
 */
UCLASS()
class GRIDIRON_API UGridironMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UGridironMovementComponent();

	// Cache of the owner of this movement component;
	UPROPERTY(BlueprintReadOnly, Category = "Movement Component")
	AGridironCharacter* OwningCharacter;
	
};
