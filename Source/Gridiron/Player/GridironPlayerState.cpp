// Created by Bruce Crum


#include "Gridiron/Player/GridironPlayerState.h"
#include "Net/UnrealNetwork.h"

AGridironPlayerState::AGridironPlayerState()
{
	TeamId = ITeamInterface::InvalidId;
}

void AGridironPlayerState::OnRep_TeamId()
{

}

void AGridironPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(AGridironPlayerState, MatchStats);
	DOREPLIFETIME(AGridironPlayerState, TeamId);
}

uint8 AGridironPlayerState::GetTeamId() const
{
	return TeamId;
}

void AGridironPlayerState::SetTeamId(uint8 NewTeam)
{
	TeamId = NewTeam;
}

FMatchStats AGridironPlayerState::GetMatchStats() const
{
	return MatchStats;
}

void AGridironPlayerState::ScoreKill(int32 Amount)
{
	MatchStats.Kills += Amount;
	OnMatchStatsUpdated();
}

void AGridironPlayerState::ScoreAssist(int32 Amount)
{
	MatchStats.Assists += Amount;
	OnMatchStatsUpdated();
}

void AGridironPlayerState::ScoreDeath(int32 Amount)
{
	MatchStats.Deaths += Amount;
	OnMatchStatsUpdated();
}

void AGridironPlayerState::OnMatchStatsUpdated()
{
	OnMatchStatsUpdatedDelegate.Broadcast();
}

void AGridironPlayerState::OnRep_MatchStats()
{
	OnMatchStatsUpdated();
}
