// Created by Bruce Crum


#include "Gridiron/GameModes/GridironGameState.h"
#include "Gridiron/GameModes/GridironGameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "Gridiron/Teams/TeamInfo.h"

AGridironGameState::AGridironGameState()
{
	RoundEndTime = -1.0f;
	bRoundTimerExpired = false;
}

void AGridironGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGridironGameState, Teams);
	DOREPLIFETIME(AGridironGameState, RoundEndTime);
}

void AGridironGameState::AddTeam(ATeamInfo* NewTeam, const uint8 TeamId)
{
	Teams.Insert(NewTeam, TeamId);
}

ATeamInfo* AGridironGameState::GetTeamFromId(const uint8 TeamId) const
{
	if (Teams.Num() <= 0 || TeamId == ITeamInterface::InvalidId)
	{
		// No Teams
		return nullptr;
	}

	return Teams[TeamId];
}

void AGridironGameState::AddPlayerForTeam(AGridironPlayerState* ForPlayer, uint8 TeamId)
{
	const auto Team = GetTeamFromId(TeamId);
	if (!Team)
	{
		return;
	}

	Team->AddPlayer(ForPlayer);
}

void AGridironGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	OnPlayerAmmountChangedDelegate.Broadcast();
}

void AGridironGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	OnPlayerAmmountChangedDelegate.Broadcast();
}

void AGridironGameState::SetRoundTimer(const int32 Seconds)
{
	if (!HasAuthority())
	{
		return;
	}

	RoundEndTime = GetServerWorldTimeSeconds() + Seconds;
	bRoundTimerExpired = false;

	// reset timer
	if (GetWorld()->GetTimerManager().IsTimerActive(RoundTimeTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(RoundTimeTimerHandle);
	}

	GetWorld()->GetTimerManager().SetTimer(RoundTimeTimerHandle, this, &AGridironGameState::TickTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void AGridironGameState::TickTimer()
{
	const float RoundTimeRemaining = GetRoundTimeRemaining();

	// not expired
	if (RoundTimeRemaining > 0 || bRoundTimerExpired)
	{
		return;
	}

	// let MP gamemode know if the timer has run out
	const auto GM = GetWorld()->GetAuthGameMode<AGridironGameModeBase>();
	if (!GM || !GM->OnRoundTimerExpired())
	{
		return;
	}

	bRoundTimerExpired = true;
}

float AGridironGameState::GetRoundTimeRemaining() const
{
	return FMath::Max(RoundEndTime - GetServerWorldTimeSeconds(), 0.0f);
}