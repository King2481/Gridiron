// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class AGridironCharacter;
class UBoxComponent;
class UTextRenderComponent;
class USkeletalMeshComponent;

UCLASS()
class GRIDIRON_API APickup : public AActor
{
	GENERATED_BODY()
	
public:

	// Sets default values for this actor's properties
	APickup();

	// Network replication setup
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	// Item mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* Mesh3P;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* Collision shape component */
	UPROPERTY(VisibleDefaultsOnly, Category = "Pickup")
	UBoxComponent* CollisionComp;

	// When picked up, how long does it take for this pickup to be enabled again?
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float DisableTime;

	// What sound does this pickup play when picked up.
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	USoundBase* PickupSound;

	// Is this pickup currently enabed?
	UPROPERTY(BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_Enabled, Category = "Pickup")
	bool bEnabled;

	UFUNCTION()
	void OnRep_Enabled();

	// Checks to see if we can actually pickup this pickup
	virtual bool CanPickup(AGridironCharacter* ForUser) const;

	// Blueprint event for CanPickup()
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	bool BlueprintCanPickup(AGridironCharacter* ForUser) const;

	// Handles the picking up of the item
	virtual void HandlePickup(AGridironCharacter* ForUser);

	// Blueprint event for picking up the item.
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void BlueprintHandlePickup(AGridironCharacter* ForUser);

	// Overlap event for the box collision
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Handles visiblity of the pickup, if it is disabled, it is hidden, otherwise it is always shown.
	void HandleVisibility();

	// Timer Handle for when the Pickup gets disabled.
	FTimerHandle DisableTimerHandle;

	// Called when this pickup gets reenabled.
	void OnReenabled();

	// Does this pickup start disabled?
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	bool bStartsDisabled;

};
