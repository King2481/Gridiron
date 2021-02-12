// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EItemSlot : uint8
{
	IS_Shotgun 	       UMETA(DisplayName = "Shotgun"),
	IS_Bullet	       UMETA(DisplayName = "Bullet"),
	IS_Energy          UMETA(DisplayName = "Energy"),
	IS_Explosive       UMETA(DisplayName = "Explosive"),
	IS_Melee           UMETA(DisplayName = "Melee"),
	IS_Misc            UMETA(DisplayName = "Misc"),
	IS_Count
};
