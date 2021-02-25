// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TeamDefinition.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class GRIDIRON_API UTeamDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UTeamDefinition();

	// What is the name of this team?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Team")
	FText TeamName;

	// What is the team color for this team?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Team")
	FColor TeamColor;
};
