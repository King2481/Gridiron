// Created by Bruce Crum


#include "Gridiron/GameModes/GridironGameState.h"
#include "Net/UnrealNetwork.h"
#include "Gridiron/Teams/TeamInfo.h"

AGridironGameState::AGridironGameState()
{

}

void AGridironGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGridironGameState, Teams);
}

void AGridironGameState::AddTeam(ATeamInfo* NewTeam, const uint8 TeamId)
{
	Teams.Insert(NewTeam, TeamId);
}

ATeamInfo* AGridironGameState::GetTeamFromId(const uint8 TeamId) const
{
	if (Teams.Num() <= 0)
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