// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JumpPad.generated.h"

class UBoxComponent;

UCLASS()
class GRIDIRON_API AJumpPad : public AActor
{
	GENERATED_BODY()
	
public:

	// Sets default values for this actor's properties
	AJumpPad();

	virtual void Tick(float DeltaTime) override;

	/** returns whether the given Actor can be launched by this jumppad */
	bool CanLaunch(AActor* TestActor);

	/** Launches the actor */
	void Launch(AActor* Actor);

protected:

	/** The length of air time it takes to reach the target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = JumpPad, meta = (ClampMin = "0.1"))
	float JumpTime;

	/** The destination of this Jump Pad */
	UPROPERTY(EditAnywhere, Category = JumpPad, BlueprintReadWrite, meta = (MakeEditWidget = ""))
	FVector JumpTarget;

	virtual void PostInitProperties() override;

	/** used to detect low grav mods */
	UPROPERTY()
	float AuthoredGravityZ;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = JumpPad)
	USceneComponent* SceneRoot;

	/** Static mesh for the Jump Pad */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = JumpPad)
	UStaticMeshComponent* Mesh;

	/** The Player will Jump when overlapping this box */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = JumpPad)
	UBoxComponent* TriggerBox;

	/** Actors we want to Jump next tick */
	TArray<AActor*> PendingJumpActors;

	UFUNCTION()
	void TriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	FVector CalculateJumpVelocity(AActor* JumpActor);

};
