// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	AT_None            UMETA(DisplayName = "None"),
	AT_Bullet 	       UMETA(DisplayName = "Bullet"),
	AT_BigBullet 	   UMETA(DisplayName = "Big Bullet"),
	AT_Shells 	       UMETA(DisplayName = "Shells"),
	AT_Explosive       UMETA(DisplayName = "Explosive"),
	AT_Energy          UMETA(DisplayName = "Energy")
};