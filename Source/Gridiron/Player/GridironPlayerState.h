// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GridironPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchStatsUpdatedDelegate);

USTRUCT(BlueprintType)
struct FMatchStats
{
	GENERATED_BODY()

	// Kills Scored
	UPROPERTY(BlueprintReadOnly)
	int32 Kills;

	// Assists Scored
	UPROPERTY(BlueprintReadOnly)
	int32 Assists;

	// Deaths Scored
	UPROPERTY(BlueprintReadOnly)
	int32 Deaths;

	FMatchStats()
	{
		Kills = 0;
		Assists = 0;
		Deaths = 0;
	}
};

/**
 * 
 */
UCLASS()
class GRIDIRON_API AGridironPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	AGridironPlayerState();

	// Returns the match statss for the current match
	UFUNCTION(BlueprintPure, Category = "Player State")
	FMatchStats GetMatchStats() const;

	// Scores a death, must be called on auth to replicate.
	void ScoreKill(int32 Amount = 1);

	// Scores a death, must be called on auth to replicate.
	void ScoreAssist(int32 Amount = 1);

	// Scores a death, must be called on auth to replicate.
	void ScoreDeath(int32 Amount = 1);

protected:

	// Function for replication setup.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Match stats for this player.
	UPROPERTY(BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_MatchStats, Category = "Player State")
	FMatchStats MatchStats;

	UFUNCTION()
	void OnRep_MatchStats();

	void OnMatchStatsUpdated();

	UPROPERTY(BlueprintAssignable)
	FOnMatchStatsUpdatedDelegate OnMatchStatsUpdatedDelegate;
	
};
