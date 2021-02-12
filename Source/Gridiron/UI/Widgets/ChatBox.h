// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatBox.generated.h"

class UEditableTextBox;

USTRUCT(BlueprintType)
struct FChatBoxEntry
{
	GENERATED_BODY()

	// The localized message.
	UPROPERTY(BlueprintReadOnly)
	FText Message;

	// The time this chat message was created.
	UPROPERTY(BlueprintReadOnly)
	float TimeCreated;
};

/**
 * 
 */
UCLASS()
class GRIDIRON_API UChatBox : public UUserWidget
{
	GENERATED_BODY()

public:

	UChatBox(const FObjectInitializer& ObjectInitializer);

	/* Grabs mouse focus, triggers chat events */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void StartChatInput();

	/* Releases mouse focus */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void EndChatInput();

	/* Called when the user is opening the chat box to type */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chat")
	void OnChatInputStarted();

	/* Called when the user has finished inputting chat */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chat")
	void OnChatInputEnded();

	/* Triggered when other players have sent a chat message */
	UFUNCTION(BlueprintNativeEvent, Category = "Chat")
	void OnChatMessageRecieved(const FText& Message);

protected:

	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	bool bInChatMode;

	/* Input for chat message */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableTextBox* Input;

	/* Delegate for rich text box text */
	UFUNCTION(BlueprintPure, Category = "Chat")
	FText GetChatBody();

	/* Total messages to display */
	UPROPERTY(EditDefaultsOnly, Category = "Chat", meta = (AllowPrivateAccess = "true"))
	uint8 MaxChatLines;

	/* Send a chat message to other players */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendChatMessage(const FText& Message);

	/* When was the last time we sent a chat message? */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	float LastChatMessageSentTime;

private:

	/* List of currently active chat messages */
	UPROPERTY(BlueprintReadOnly, Category = "Chat", meta = (AllowPrivateAccess = "true"))
	TArray<FChatBoxEntry> ChatMessages;

	/* List of chat messages compiled into a single text with appropriate markup */
	FText ChatBody;

	/* Has the body been changed and needs to be regenerated? */
	bool bIsBodyDirty;
	
};
