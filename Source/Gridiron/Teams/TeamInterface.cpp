// Created by Bruce Crum


#include "Gridiron/Teams/TeamInterface.h"

// Add default functionality here for any ITeamInterface functions that are not pure virtual.

const uint8 ITeamInterface::InvalidId = 255;

bool ITeamInterface::IsAlly(const ITeamInterface* Checker, const ITeamInterface* Other)
{
	if (!Checker || !Other)
	{
		return false;
	}

	if (Checker->GetTeamId() == InvalidId && Other->GetTeamId() == InvalidId)
	{
		// Neither the checker or the other are assigned teams, don't say they're allies.
		return false;
	}

	return Checker->GetTeamId() == Other->GetTeamId();
}