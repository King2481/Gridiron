// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Gridiron/Teams/TeamInterface.h"
#include "GridironPlayerController.generated.h"

class AGridironPlayerState;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoundWonDelegate, AGridironPlayerState*, WinningPlayerState, uint8, WinningTeam);

/**
 * 
 */
UCLASS()
class GRIDIRON_API AGridironPlayerController : public APlayerController, public ITeamInterface
{
	GENERATED_BODY()

public:

	AGridironPlayerController();

	// Handles construction of widgets that are handled by the player controller.
	void ConstructWidgets();

	virtual void SetupInputComponent() override;

	virtual void ClientTeamMessage_Implementation(APlayerState* SenderPlayerStateBase, const FString& S, FName Type, float MsgLifeTime) override;

	// Called when we recieve a chat message
	virtual void OnChatMessageReceived(const FText& Message, AGridironPlayerState* SenderPlayerState = nullptr);

	// Server RPC to send chat messages.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSendChatMessage(const FText& Message, const bool bTeamMessage);
	void ServerSendChatMessage_Implementation(const FText& Message, const bool bTeamMessage);
	bool ServerSendChatMessage_Validate(const FText& Message, const bool bTeamMessage);

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
	UFUNCTION(BlueprintCallable, Client, Reliable, Category = "Player Controller")
	void ClientPlaySound2D(USoundBase* SoundToPlay);
	void ClientPlaySound2D_Implementation(USoundBase* SoundToPlay);

	// Respawns the player, if applicable.
	UFUNCTION(BlueprintCallable, Category = "Player Controller")
	void RespawnPlayer();

	// Queues a respawn with a delay.
	UFUNCTION(BlueprintCallable, Category = "Player Controller")
	void QueueRespawnDelay(float Delay);

	// Setter for the pawn.
	virtual void SetPawn(APawn* InPawn) override;

	// Joins the specified team
	void JoinTeam(uint8 NewTeam);

	// Returns the Team ID for the controller
	virtual uint8 GetTeamId() const override;

	// Changes a camera fade
	UFUNCTION(BlueprintCallable, Client, Reliable, Category = "Player Controller")
	void ClientStartCameraFade(float FromAlpha, float ToAlpha, float Duration, const FLinearColor& Color, bool bShouldFadeAudio, bool bHoldWhenFinished);
	void ClientStartCameraFade_Implementation(float FromAlpha, float ToAlpha, float Duration, const FLinearColor& Color, bool bShouldFadeAudio, bool bHoldWhenFinished);

protected:

	// Called when we want to open the in game menu
	void ToggleInGameMenu();

	// Shows/Hide in game menu based on the argument passed.
	UFUNCTION(BlueprintCallable, Category = "Player Controller")
	void SetShowInGameMenu(const bool NewIsInGameMenu);

	// Is this player looking at the in game menu?
	bool bIsInGameMenu;

	// Pointer to the InGameMenu Widget
	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	UUserWidget* InGameMenuWidget;

	// The class to use for the creation of the InGameMenu Widget
	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> InGameMenuWidgetClass;

	// Called when we want to show the scoreboard
	void ShowScoreboard();

	// Called when we want to hide the scoreboard
	void HideScoreboard();

	// Pointer to the Scoreboard Widget
	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	UUserWidget* ScoreboardWidget;

	// The class to use for the creation of the InGameMenu Widget
	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> ScoreboardWidgetClass;

	// Called when we want to start a chat
	void StartChat();

	// Called when we want to start a team chat
	void StartTeamChat();

	// Is this player currently inputing text?
	bool bIsChatting;

	// Is this player looking at the scoreboard?
	bool bIsLookingAtScoreboard;

	// Updates the players input mode, we may want different input modes such as Game / UI / Game & UI
	void UpdateInputMode();
	
	UPROPERTY()
	FTimerHandle DelayRespawn_TimerHandle;

	// Called when we QueueRespawnDelay() is finished
	void OnQueueRespawnDelayFinished();

};
