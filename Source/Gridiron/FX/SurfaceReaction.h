// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SurfaceReaction.generated.h"

USTRUCT(BlueprintType)
struct FSurfaceReactionInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* ReactionSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UParticleSystem* ReactionEffect;

	FSurfaceReactionInfo()
	{
		ReactionSound = nullptr;
		ReactionEffect = nullptr;
	}
};

/**
 * 
 */
UCLASS(Blueprintable)
class GRIDIRON_API USurfaceReaction : public UObject
{
	GENERATED_BODY()

public:

	USurfaceReaction();

	FSurfaceReactionInfo GetSurfaceReactionFromHit(TWeakObjectPtr<UPhysicalMaterial> PhysMaterial);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface Reactions")
	TMap<TEnumAsByte<EPhysicalSurface>, FSurfaceReactionInfo> SurfaceTypeReactions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface Reactions")
	FSurfaceReactionInfo DefaultReaction;
	
};
