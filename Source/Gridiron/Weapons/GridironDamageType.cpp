// Created by Bruce Crum


#include "Gridiron/Weapons/GridironDamageType.h"

UGridironDamageType::UGridironDamageType()
{
	Magnitude = 1.f;
	RagdollLaunchMagnitude = 12500.f;
}

UDamageTypeBullet::UDamageTypeBullet()
{
	Magnitude = 0.5f;
}

UDamageTypePellet::UDamageTypePellet()
{
	Magnitude = 100.f;
}

UDamageTypeImpact::UDamageTypeImpact()
{
	Magnitude = 0.1f;
}

UDamageTypeEnergy::UDamageTypeEnergy()
{
	Magnitude = 0.1f;
}

UDamageTypeExplosive::UDamageTypeExplosive()
{
	Magnitude = 1000.f;
	RagdollLaunchMagnitude = 30000.f;
}

UDamageTypePhysical::UDamageTypePhysical()
{
	Magnitude = 1000.f;
	RagdollLaunchMagnitude = 3000.f;
}