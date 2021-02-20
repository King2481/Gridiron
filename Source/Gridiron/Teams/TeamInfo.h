// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Gridiron/Teams/TeamInterface.h"
#include "TeamInfo.generated.h"

class UTeamDefinition;
class AGridironPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayersArrayUpdatedDelegate);


/**
 * 
 */
UCLASS()
class GRIDIRON_API ATeamInfo : public AInfo, public ITeamInterface
{
	GENERATED_BODY()

public:

	ATeamInfo();

	// Initializes this teaam with a definition and TeamId
	void InitializeTeam(UTeamDefinition* NewTeamDefition, uint8 NewTeamId);

	// Returns the TeamId for this team
	virtual uint8 GetTeamId() const override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Scores a specified amount of points (default is 1)
	UFUNCTION(BlueprintCallable, Category = "Team")
	void ScorePoints(int32 AmountToScore = 1);

	// Returns the score for this team
	int32 GetScore() const;

	// Adds a player to this team.
	void AddPlayer(AGridironPlayerState* NewPlayer);

	// Returns the team color
	FColor GetTeamColor() const;

protected:

	// What team does this character belong to?
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_TeamId, BlueprintReadOnly, Category = "Team")
	uint8 TeamId;

	// What is the team definition? (ie: Red Team, Blue Team, etc.)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
	UTeamDefinition* TeamDefinition;

	UFUNCTION()
	void OnRep_TeamId();

	// What is this teams score?
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
	int32 TeamScore;

	// What players are on this team?
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Players, BlueprintReadOnly, Category = "Team")
	TArray<AGridironPlayerState*> Players;

	UFUNCTION()
	void OnRep_Players();

	UPROPERTY(BlueprintAssignable)
	FOnPlayersArrayUpdatedDelegate OnPlayersArrayUpdatedDelegate;
	
};
