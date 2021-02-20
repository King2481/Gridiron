// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TeamInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTeamInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GRIDIRON_API ITeamInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/* Invalid ID for a faction */
	static const uint8 InvalidId;

	static bool IsAlly(const ITeamInterface* Checker, const ITeamInterface* Other);

	virtual uint8 GetTeamId() const { return ITeamInterface::InvalidId; }
};
