// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GridironGameState.generated.h"

class ATeamInfo;
class AGridironPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerAmmountChangedDelegate);

/**
 * 
 */
UCLASS()
class GRIDIRON_API AGridironGameState : public AGameState
{
	GENERATED_BODY()

public:

	AGridironGameState();

	// Replication setup
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Adds a team to the game
	void AddTeam(ATeamInfo* NewTeam, const uint8 TeamId);

	// Returns the team info for the specified TeamId
	UFUNCTION(BlueprintPure, Category = "Game State")
	ATeamInfo* GetTeamFromId(const uint8 TeamId) const;

	// Adds a player to the specifed TeamId
	void AddPlayerForTeam(AGridironPlayerState* ForPlayer, uint8 TeamId);

	// Add PlayerState from the PlayerArray.
	virtual void AddPlayerState(APlayerState* PlayerState) override;

	// Remove PlayerState from the PlayerArray.
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	// Delegate that is sent out when player amount has changed
	UPROPERTY(BlueprintAssignable)
	FOnPlayerAmmountChangedDelegate OnPlayerAmmountChangedDelegate;

	// Sets the round timer for the game state so clients know how much round time is left
	void SetRoundTimer(const int32 Seconds);

	// Returns, in seconds, how much round time is remaining
	UFUNCTION(BlueprintPure, Category = "Game State")
	float GetRoundTimeRemaining() const;

protected:

	// What teams have been created and are currently in play?
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Game State")
	TArray<ATeamInfo*> Teams;

	// At what time will the round in (in world seconds)
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Game State")
	float RoundEndTime;

	// Has the round timer expired and no longer needs to tick?
	bool bRoundTimerExpired;

	// Ticks the round timer
	void TickTimer();

	FTimerHandle RoundTimeTimerHandle;

};
