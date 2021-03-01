// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameStructTypes.generated.h"

class AGridironPlayerState;
class UGridironDamageType;

USTRUCT(BlueprintType)
struct FKillfeedNotice
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	AGridironPlayerState* Killer;

	UPROPERTY(BlueprintReadOnly)
	AGridironPlayerState* Victim;

	UPROPERTY(BlueprintReadOnly)
	AActor* KilledBy;

	UPROPERTY(BlueprintReadOnly)
	bool bSuicide;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UGridironDamageType> DamageType;

	FKillfeedNotice()
	{
		Killer = nullptr;
		Victim = nullptr;
		KilledBy = nullptr;
		bSuicide = false;
		DamageType = nullptr;
	}
};