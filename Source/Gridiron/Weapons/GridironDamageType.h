// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "GridironDamageType.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GRIDIRON_API UGridironDamageType : public UDamageType
{
	GENERATED_BODY()

public:

	UGridironDamageType();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float Magnitude;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float RagdollLaunchMagnitude;

};

UCLASS(BlueprintType)
class GRIDIRON_API UDamageTypeBullet : public UGridironDamageType
{
	GENERATED_BODY()

public:

	UDamageTypeBullet();

};

UCLASS(BlueprintType)
class GRIDIRON_API UDamageTypePellet : public UGridironDamageType
{
	GENERATED_BODY()

public:

	UDamageTypePellet();

};

UCLASS(BlueprintType)
class GRIDIRON_API UDamageTypeImpact : public UGridironDamageType
{
	GENERATED_BODY()

public:

	UDamageTypeImpact();

};

UCLASS(BlueprintType)
class GRIDIRON_API UDamageTypeExplosive : public UGridironDamageType
{
	GENERATED_BODY()

public:

	UDamageTypeExplosive();
};

UCLASS(BlueprintType)
class GRIDIRON_API UDamageTypeEnergy : public UGridironDamageType
{
	GENERATED_BODY()

public:

	UDamageTypeEnergy();
};

UCLASS(BlueprintType)
class GRIDIRON_API UDamageTypePhysical : public UGridironDamageType
{
	GENERATED_BODY()

public:

	UDamageTypePhysical();
};