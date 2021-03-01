// Created by Bruce Crum

#include "GridironGameModeBase.h"
#include "GridironGameState.h"
#include "Gridiron/Player/GridironPlayerController.h"
#include "Gridiron/Player/GridironPlayerState.h"
#include "Gridiron/Characters/GridironCharacter.h"
#include "Gridiron/UI/GridironHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Gridiron/Teams/TeamInterface.h"
#include "Gridiron/Teams/TeamInfo.h"
#include "Gridiron/Teams/TeamDefinition.h"
#include "Gridiron/Game/GameStructTypes.h"

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
	WinningTeamId = ITeamInterface::InvalidId;
	RoundTimeLimit = 300; // 5 minutes by default
	bKillFeed = true;

	bRespawnPlayerOnDeath = true;
}

void AGridironGameModeBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InitializeTeams();
}

void AGridironGameModeBase::InitializeTeams()
{
	if (TeamsForMode.Num() <= 0)
	{
		// No teams to initialize
		return;
	}

	const auto GS = GetGameState<AGridironGameState>();
	if (!GS)
	{
		// No GameState available.
		return;
	}

	for (int i = 0; i <= TeamsForMode.Num() - 1; i++)
	{
		const auto Team = TeamsForMode[i].LoadSynchronous();
		if (Team)
		{
			const auto NewTeam = GetWorld()->SpawnActor<ATeamInfo>(ATeamInfo::StaticClass(), FTransform());
			if (NewTeam)
			{
				NewTeam->InitializeTeam(Team, i);
				GS->AddTeam(NewTeam, i);
			}
		}
	}
}

void AGridironGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	const auto PC = Cast<AGridironPlayerController>(NewPlayer);
	const auto PS = Cast<AGridironPlayerState>(NewPlayer->PlayerState);
	if (PC && PS)
	{
		if (TeamsForMode.Num() >= 2)
		{
			PC->JoinTeam(ChooseTeam(PS));
		}
		else
		{
			PC->JoinTeam(ITeamInterface::InvalidId);
		}
	}

	Super::PostLogin(NewPlayer);
}

uint8 AGridironGameModeBase::ChooseTeam(AGridironPlayerState* ForPlayerState) const
{
	TArray<int32> TeamBalance;
	TeamBalance.AddZeroed(TeamsForMode.Num());

	// get current team balance
	for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
	{
		AGridironPlayerState const* const TestPlayerState = Cast<AGridironPlayerState>(GameState->PlayerArray[i]);
		if (TestPlayerState && TestPlayerState != ForPlayerState && TeamBalance.IsValidIndex(TestPlayerState->GetTeamId()))
		{
			TeamBalance[TestPlayerState->GetTeamId()]++;
		}
	}

	// find least populated one
	int32 BestTeamScore = TeamBalance[0];
	for (int32 i = 1; i < TeamBalance.Num(); i++)
	{
		if (BestTeamScore > TeamBalance[i])
		{
			BestTeamScore = TeamBalance[i];
		}
	}

	// there could be more than one...
	TArray<int32> BestTeams;
	for (int32 i = 0; i < TeamBalance.Num(); i++)
	{
		if (TeamBalance[i] == BestTeamScore)
		{
			BestTeams.Add(i);
		}
	}

	// get random from best list
	const int32 RandomBestTeam = BestTeams[FMath::RandHelper(BestTeams.Num())];
	return RandomBestTeam;
}


void AGridironGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	RoundTimeLimit = UGameplayStatics::GetIntOption(Options, TEXT("RoundTimeLimit"), RoundTimeLimit) * 60; // Multiply the get it in seconds as the game counts seconds.
	bKillFeed = UGameplayStatics::GetIntOption(Options, TEXT("KillFeed"), bKillFeed) > 0;

	// Password
	ServerPassword = UGameplayStatics::ParseOption(Options, TEXT("Password"));
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

	BlueprintOnCharacterKilled(Victim, KillingDamage, DamageEvent, EventInstigator, DamageCauser);

	if (bRespawnPlayerOnDeath)
	{
		const auto VictimPC = Cast<AGridironPlayerController>(Victim->GetController());
		if (VictimPC)
		{
			if (MinRespawnDelay > FLT_EPSILON)
			{
				VictimPC->QueueRespawnDelay(MinRespawnDelay);
			}
			else
			{
				VictimPC->RespawnPlayer();
			}
		}
	}

	if (bKillFeed)
	{
		FKillfeedNotice Notice;
		Notice.Killer = KillerPS;
		Notice.Victim = VictimPS;
		Notice.KilledBy = DamageCauser;
		Notice.bSuicide = bSelfKill;
		Notice.DamageType = DamageEvent.DamageTypeClass;

		for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
		{
			const auto Controller = Cast<AGridironPlayerController>(Iterator->Get());
			if (Controller)
			{
				Controller->ClientRecieveKillFeedNotice(Notice);
			}
		}
	}
}

float AGridironGameModeBase::OnCharacterTakeDamage(AGridironCharacter* Reciever, float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float AlteredDamage = Damage * UniversalDamageMultiplayer;

	const auto DamagedController = Reciever ? Reciever->GetController() : nullptr;
	const auto DamagerController = EventInstigator;
	const bool bSelfDamage = DamagedController == DamagerController;
	const auto DamagingCharacter = Cast<AGridironCharacter>(EventInstigator->GetPawn());
	
	bool bHasQuad = false;
	if (DamagingCharacter && DamagingCharacter->IsGameplayCueActive(FName("Powerup.QuadDamage")))
	{
		bHasQuad = true;
	}

	if (bSelfDamage)
	{
		AlteredDamage *= SelfDamageMultiplier;
	}

	if (bHasQuad && !bSelfDamage)
	{
		AlteredDamage *= 4.f;
	}

	// Friendly fire specfics check.
	if (ITeamInterface::IsAlly(Reciever, DamagingCharacter) && bAllowFriendlyFire && !bSelfDamage)
	{
		AlteredDamage *= FriendlyFireDamageMultiplier;
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

void AGridironGameModeBase::PlayerStateWin(AGridironPlayerState* NewWinningPlayerState)
{
	// Winner, inform everyone.
	WinningPlayerState = NewWinningPlayerState;
	SetMatchState(MatchState::RoundWon);
}

void AGridironGameModeBase::TeamWin(const uint8 NewWinningTeam)
{
	// Winner, inform everyone.
	WinningTeamId = NewWinningTeam;
	SetMatchState(MatchState::RoundWon);
}