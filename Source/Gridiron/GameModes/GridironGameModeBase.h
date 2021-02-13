// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GridironGameModeBase.generated.h"

class AGridironCharacter;
class AGridironPlayerState;

/**
* Additional match states
*/
namespace MatchState
{
	/* round has ended and a winner (or draw) has been declared */
	extern const FName RoundWon;

	/*The game is officially "over".*/
	extern const FName GameOver;
}

/**
 * 
 */
UCLASS()
class GRIDIRON_API AGridironGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:

	AGridironGameModeBase();

	// Called when the game is initialized
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	// Called when a character is killed.
	virtual void OnCharacterKilled(AGridironCharacter* Victim, float KillingDamage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	// Called when a character takes damage. Gamemodes may modify damage based on certain conditions.
	virtual float OnCharacterTakeDamage(AGridironCharacter* Reciever, float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	// Called before a client attempts to join this match
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	// True if the server requires a password to join 
	bool IsPasswordProtected() const;

	virtual void HandleMatchHasStarted() override;

	// Called when we have "won" the Gamemode and need to handle it.
	virtual void HandleRoundWon();

	// Called when we have actually reached the game mode
	void OnGameOverStart();

	// Called to handle the game over for this game mode.
	virtual void HandleGameOver();

	// Override handling of match state changing to accomodate our custom states 
	virtual void SetMatchState(FName NewState) override;

	// Called when the match state has been changed.
	virtual void CallMatchStateChangeNotify();

	// A generic score that is needed to "win" this gamemode, generally used in other gamemode classes as part of their win condition checks.
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Gamemode")
	int32 ScoreNeededToWin;

	// What is the universal damage multiplayer?
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Gamemode")
	float UniversalDamageMultiplayer;

	// What is the self damage multiplier?
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Gamemode")
	float SelfDamageMultiplier;

	// Do we allow friendly fire hits?
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Gamemode")
	bool bAllowFriendlyFire;

	// When damaged from friendly fire, what is the multiplier?
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Gamemode")
	float FriendlyFireDamageMultiplier;

	// Password required to connect to the server 
	FString ServerPassword;

	// How long does this game mode last, in seconds?
	UPROPERTY(Config, BlueprintReadOnly, Category = "Gamemode")
	int32 RoundTimeLimit;

	// This sole winner of this gamemode.
	AGridironPlayerState* WinningPlayerState;

	// The winning team of this gamemode
	uint8 WinningTeamId;

	FTimerHandle GameOverTimerHandle;

};
