// Created by Bruce Crum


#include "Gridiron/UI/GridironHUD.h"
#include "Gridiron/UI/Widgets/ChatBox.h"
#include "Blueprint/UserWidget.h"
#include "Gridiron/Player/GridironPlayerController.h"

AGridironHUD::AGridironHUD()
{
	ChatBoxWidget = nullptr;
	GameHUD = nullptr;

#if UE_SERVER
	ChatBoxWidgetClass = nullptr;
	DefaultGameHUDClass = nullptr;
#else
	// Chat
	static ConstructorHelpers::FClassFinder<UChatBox> ChatBoxWidgetFinder(TEXT("/Game/UI/Widgets/HUD/BP_ChatBox"));
	ChatBoxWidgetClass = ChatBoxWidgetFinder.Succeeded() ? ChatBoxWidgetFinder.Class : nullptr;

	// HUD
	static ConstructorHelpers::FClassFinder<UUserWidget> GameHUDWidgetClass(TEXT("/Game/UI/Widgets/HUD/BP_GameHUD"));
	DefaultGameHUDClass = GameHUDWidgetClass.Class;
#endif
}

void AGridironHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SetupHUD();
}

void AGridironHUD::SetupHUD()
{
#if !UE_SERVER
	CreateGameHUD();
	CreateChatBoxWidget();

	const auto PC = Cast<AGridironPlayerController>(GetOwningPlayerController());
	if (PC)
	{
		// The player controller needs to construct its specific widgets.
		PC->ConstructWidgets();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Error, could not construct PlayerController specific widgets."));
	}
#endif
}

void AGridironHUD::CreateGameHUD()
{
	if (!DefaultGameHUDClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Error, attempted to create GameHUD widget, but GameHUD class is null"));
		return;
	}

	GameHUD = CreateWidget<UUserWidget>(GetOwningPlayerController(), DefaultGameHUDClass);
	if (GameHUD)
	{
		GameHUD->AddToViewport();
	}
}

void AGridironHUD::CreateChatBoxWidget()
{
	if (!ChatBoxWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Error, attempted to create chatbox widget, but chatbox class is null"));
		return;
	}

	ChatBoxWidget = CreateWidget<UChatBox>(GetOwningPlayerController(), ChatBoxWidgetClass);
	if (ChatBoxWidget)
	{
		ChatBoxWidget->AddToViewport();
	}
}

void AGridironHUD::StartChatInput()
{
	if (ChatBoxWidget)
	{
		ChatBoxWidget->StartChatInput();
	}
}

void AGridironHUD::OnChatMessageReceived(const FText& Message, AGridironPlayerState* SenderPlayerState)
{
	if (ChatBoxWidget)
	{
		ChatBoxWidget->OnChatMessageRecieved(Message);
	}
}
