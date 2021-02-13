// Created by Bruce Crum

#include "GridironGameModeBase.h"
#include "GridironGameState.h"
#include "Gridiron/Player/GridironPlayerController.h"
#include "Gridiron/Player/GridironPlayerState.h"
#include "Gridiron/Characters/GridironCharacter.h"
#include "Gridiron/UI/GridironHUD.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName RoundWon = FName(TEXT("RoundWon"));
	const FName GameOver = FName(TEXT("GameOver"));
}

AGridironGameModeBase::AGridironGameModeBase()
{
	static ConstructorHelpers::FClassFinder<AGridironCharacter>BPPawnClass(TEXT("/Game/Game/Actors/Characters/BP_Character_Player"));
	DefaultPawnClass = BPPawnClass.Class;

	GameStateClass = AGridironGameState::StaticClass();
	PlayerControllerClass = AGridironPlayerController::StaticClass();
	PlayerStateClass = AGridironPlayerState::StaticClass();
	HUDClass = AGridironHUD::StaticClass();

	ScoreNeededToWin = -1;
	UniversalDamageMultiplayer = 1.f;
	SelfDamageMultiplier = 0.25f;
	WinningPlayerState = nullptr;
	WinningTeamId = 255; // TODO: When team interface is implemented, use the InvalidID
	RoundTimeLimit = 300; // 5 minutes by default
}

void AGridironGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	RoundTimeLimit = UGameplayStatics::GetIntOption(Options, TEXT("RoundTimeLimit"), RoundTimeLimit);

	// Password
	ServerPassword = UGameplayStatics::ParseOption(Options, TEXT("Password="));
	if (!ServerPassword.IsEmpty())
	{
		UE_LOG(LogGameMode, Display, TEXT("Server is now password protected!"));
	}
}

void AGridironGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	// Password checking
	if (IsPasswordProtected())
	{
		const auto ProvidedPassword = UGameplayStatics::ParseOption(Options, TEXT("Password"));
		if (ProvidedPassword.IsEmpty())
		{
			ErrorMessage = TEXT("No password provided.");
		}
		else if (ProvidedPassword.Compare(ServerPassword, ESearchCase::CaseSensitive) != 0)
		{
			ErrorMessage = TEXT("Password mismatch.");
		}

		if (!ErrorMessage.IsEmpty())
		{
			UE_LOG(LogGameMode, Warning, TEXT("Rejecting incoming connection, password failure: %s"), *ErrorMessage);
			return;
		}
	}

	// Everything is fine, go ahead with login
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void AGridironGameModeBase::OnCharacterKilled(AGridironCharacter* Victim, float KillingDamage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!Victim)
	{
		UE_LOG(LogGameMode, Warning, TEXT("A character was killed, but the victim was nullptr."));
		return;
	}

	const auto VictimPS = Cast<AGridironPlayerState>(Victim->GetPlayerState());
	const auto KillerPS = EventInstigator ? EventInstigator->GetPlayerState<AGridironPlayerState>() : nullptr;
	const auto GS = Cast<AGridironGameState>(GameState);
	const bool bSelfKill = VictimPS == KillerPS;

	if (VictimPS && KillerPS)
	{
		UE_LOG(LogGameMode, VeryVerbose, TEXT("%s Was killed by %s"), *VictimPS->GetPlayerName(), *KillerPS->GetPlayerName());
	}

	if (KillerPS && !bSelfKill)
	{
		KillerPS->ScoreKill();
	}

	if (VictimPS)
	{
		VictimPS->ScoreDeath();
	}
}

float AGridironGameModeBase::OnCharacterTakeDamage(AGridironCharacter* Reciever, float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float AlteredDamage = Damage * UniversalDamageMultiplayer;

	const auto DamagedController = Reciever ? Reciever->GetController() : nullptr;
	const auto DamagerController = EventInstigator;
	const bool bSelfDamage = DamagedController == DamagerController;
	const auto DamagingCharacter = Cast<AGridironCharacter>(EventInstigator->GetPawn());

	if (bSelfDamage)
	{
		AlteredDamage *= SelfDamageMultiplier;
	}

	return AlteredDamage;
}

void AGridironGameModeBase::SetMatchState(FName NewState)
{
	Super::SetMatchState(NewState);

	UE_LOG(LogGameMode, Display, TEXT("State: %s -> %s"), *MatchState.ToString(), *NewState.ToString());

	if (GetGameState<AGameState>())
	{
		GetGameState<AGameState>()->SetMatchState(NewState);
	}

	CallMatchStateChangeNotify();
}

void AGridironGameModeBase::CallMatchStateChangeNotify()
{
	if (MatchState == MatchState::RoundWon)
	{
		HandleRoundWon();
	}
	else if (MatchState == MatchState::GameOver)
	{
		HandleGameOver();
	}
	else
	{
		HandleMatchHasStarted();
	}
}

void AGridironGameModeBase::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

}

void AGridironGameModeBase::HandleRoundWon()
{
	// Inform all connected players that we have won the round.
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		const auto Controller = Cast<AGridironPlayerController>(Iterator->Get());
		if (!Controller)
		{
			continue;
		}

		Controller->OnRoundWon(WinningPlayerState, WinningTeamId);
	}

	GetWorldTimerManager().SetTimer(GameOverTimerHandle, this, &AGridironGameModeBase::OnGameOverStart, 10.f);
}

void AGridironGameModeBase::OnGameOverStart()
{
	SetMatchState(MatchState::GameOver);
}

void AGridironGameModeBase::HandleGameOver()
{
	// Just restart the game for now.
	RestartGame();
}

bool AGridironGameModeBase::IsPasswordProtected() const
{
	return !ServerPassword.IsEmpty();
}