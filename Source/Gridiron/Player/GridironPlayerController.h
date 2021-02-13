// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GridironPlayerController.generated.h"

class AGridironPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoundWonDelegate, AGridironPlayerState*, WinningPlayerState, uint8, WinningTeam);

/**
 * 
 */
UCLASS()
class GRIDIRON_API AGridironPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AGridironPlayerController();

	virtual void SetupInputComponent() override;

	virtual void ClientTeamMessage_Implementation(APlayerState* SenderPlayerStateBase, const FString& S, FName Type, float MsgLifeTime) override;

	// Called when we recieve a chat message
	virtual void OnChatMessageReceived(const FText& Message, AGridironPlayerState* SenderPlayerState = nullptr);

	// Server RPC to send chat messages.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSendChatMessage(const FText& Message);
	void ServerSendChatMessage_Implementation(const FText& Message);
	bool ServerSendChatMessage_Validate(const FText& Message);

	// Called when the chatbox has popped up and text input is allowed
	void OnChatInputStarted();

	// Called when the chatbox has closed.
	void OnChatInputEnded();

	/* Called when a round has been won from the gamemode.
	 * If single based, there will be a winning player state,
	 * otherwise, a winning team.
	 */
	void OnRoundWon(AGridironPlayerState* WinningPlayerState, uint8 WinningTeam);

	// Client version of OnRoundWon()
	UFUNCTION(Client, Reliable)
	void ClientOnRoundWon(AGridironPlayerState* WinningPlayerState, uint8 WinningTeam);
	void ClientOnRoundWon_Implementation(AGridironPlayerState* WinningPlayerState, uint8 WinningTeam);

	// Delegate for when a round is won.
	UPROPERTY(BlueprintAssignable)
	FOnRoundWonDelegate OnRoundWonDelegate;

	// Will play a sound only this player will hear
	UFUNCTION(Client, Reliable)
	void ClientPlaySound2D(USoundBase* SoundToPlay);
	void ClientPlaySound2D_Implementation(USoundBase* SoundToPlay);

protected:

	// Called when we want to start a chat
	void StartChat();

	// Is this player currently inputing text?
	bool bIsChatting;

	// Is this player looking at the in game menu?
	bool bIsInGameMenu;

	// Is this player looking at the scoreboard?
	bool bIsLookingAtScoreboard;

	// Updates the players input mode, we may want different input modes such as Game / UI / Game & UI
	void UpdateInputMode();
	
};
