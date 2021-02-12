// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GridironGameModeBase.generated.h"

class AGridironCharacter;

/**
 * 
 */
UCLASS()
class GRIDIRON_API AGridironGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:

	AGridironGameModeBase();

	// Called when a character is killed.
	virtual void OnCharacterKilled(AGridironCharacter* Victim, float KillingDamage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	// Called when a character takes damage. Gamemodes may modify damage based on certain conditions.
	virtual float OnCharacterTakeDamage(AGridironCharacter* Reciever, float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	
};
