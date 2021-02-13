// Created by Bruce Crum


#include "Gridiron/Player/GridironPlayerController.h"
#include "Gridiron/Player/GridironPlayerState.h"
#include "Gridiron/UI/GridironHUD.h"

AGridironPlayerController::AGridironPlayerController()
{
	bIsChatting = false;
	bIsInGameMenu = false;
	bIsLookingAtScoreboard = false;
}

void AGridironPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("StartChat", IE_Pressed, this, &AGridironPlayerController::StartChat);
}

void AGridironPlayerController::StartChat()
{
	const auto GridironHUD = Cast<AGridironHUD>(GetHUD());
	if (GridironHUD)
	{
		GridironHUD->StartChatInput();
	}
}

void AGridironPlayerController::ServerSendChatMessage_Implementation(const FText& Message)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		const auto PC = Cast<AGridironPlayerController>(*Iterator);
		if (!PC)
		{
			continue;
		}

		PC->ClientTeamMessage(PlayerState, Message.ToString(), false ? TEXT("Host") : TEXT("Client"));
	}
}

bool AGridironPlayerController::ServerSendChatMessage_Validate(const FText& Message)
{
	return true;
}

void AGridironPlayerController::ClientTeamMessage_Implementation(APlayerState* SenderPlayerStateBase, const FString& S, FName Type, float MsgLifeTime)
{
	Super::ClientTeamMessage_Implementation(SenderPlayerStateBase, S, Type, MsgLifeTime);

	const auto SenderPlayerState = Cast<AGridironPlayerState>(SenderPlayerStateBase);;

	const bool bGamemodeSay = Type == FName(TEXT("Gamemode"));
	const bool bHostSay = Type == FName(TEXT("Host"));

	static FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("Name"), FText::FromString(SenderPlayerState ? SenderPlayerState->GetPlayerName() : TEXT("")));
	Arguments.Add(TEXT("Title"), FText::FromString(bHostSay ? TEXT("(Host)") : TEXT("")));
	Arguments.Add(TEXT("Message"), FText::FromString(S));

	OnChatMessageReceived(FText::Format(NSLOCTEXT("HUD", "ChatMessageFormat", "{Name} {Title}: {Message}"), Arguments), SenderPlayerState);
}

void AGridironPlayerController::OnChatMessageReceived(const FText& Message, AGridironPlayerState* SenderPlayerState /*= nullptr*/)
{
	const auto GridironHUD = Cast<AGridironHUD>(GetHUD());
	if (GridironHUD)
	{
		GridironHUD->OnChatMessageReceived(Message, SenderPlayerState);
	}
}

void AGridironPlayerController::OnChatInputStarted()
{
	bIsChatting = true;
	UpdateInputMode();
}

void AGridironPlayerController::OnChatInputEnded()
{
	bIsChatting = false;
	UpdateInputMode();
}

void AGridironPlayerController::UpdateInputMode()
{
	if (bIsChatting || bIsInGameMenu)
	{
		SetInputMode(FInputModeUIOnly());
		bShowMouseCursor = true;
		return;
	}

	if (bIsLookingAtScoreboard)
	{
		SetInputMode(FInputModeGameAndUI());
		bShowMouseCursor = true;
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

void AGridironPlayerController::OnRoundWon(AGridironPlayerState* WinningPlayerState, uint8 WinningTeam)
{
	ClientOnRoundWon(WinningPlayerState, WinningTeam);
}

void AGridironPlayerController::ClientOnRoundWon_Implementation(AGridironPlayerState* WinningPlayerState, uint8 WinningTeam)
{
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	OnRoundWonDelegate.Broadcast(WinningPlayerState, WinningTeam);
}