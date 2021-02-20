// Created by Bruce Crum


#include "Gridiron/Teams/TeamInfo.h"
#include "TeamDefinition.h"
#include "TeamInterface.h"
#include "Net/UnrealNetwork.h"
#include "Gridiron/Player/GridironPlayerState.h"

ATeamInfo::ATeamInfo()
{
	bReplicates = true;
	TeamId = ITeamInterface::InvalidId;
	TeamDefinition = nullptr;
	TeamScore = 0;
}

void ATeamInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATeamInfo, TeamId);
	DOREPLIFETIME(ATeamInfo, TeamDefinition);
	DOREPLIFETIME(ATeamInfo, TeamScore);
	DOREPLIFETIME(ATeamInfo, Players);
}

void ATeamInfo::InitializeTeam(UTeamDefinition* NewTeamDefition, uint8 NewTeamId)
{
	TeamDefinition = NewTeamDefition;
	TeamId = NewTeamId;
}

void ATeamInfo::OnRep_TeamId()
{

}

uint8 ATeamInfo::GetTeamId() const
{
	return TeamId;
}

void ATeamInfo::ScorePoints(int32 AmountToScore /* = 1 */)
{
	TeamScore += AmountToScore;
}

int32 ATeamInfo::GetScore() const
{
	return TeamScore;
}

void ATeamInfo::AddPlayer(AGridironPlayerState* NewPlayer)
{
	if (NewPlayer)
	{
		Players.AddUnique(NewPlayer);
		NewPlayer->SetTeamId(TeamId);

		OnPlayersArrayUpdatedDelegate.Broadcast();
	}
}

void ATeamInfo::OnRep_Players()
{
	OnPlayersArrayUpdatedDelegate.Broadcast();
}

