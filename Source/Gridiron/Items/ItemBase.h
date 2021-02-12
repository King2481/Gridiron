// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

class AGridironCharacter;
class USkeletalMeshComponent;

UCLASS()
class GRIDIRON_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:

	// Sets default values for this actor's properties
	AItemBase();

	// Initilizes the item.
	virtual void InitItem(AGridironCharacter* NewOwner);

	// Sets the pawn owner.
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetPawnOwner(AGridironCharacter* NewOwner);

	// Returns the pawn owner of this item.
	UFUNCTION(BlueprintPure, Category = "Item")
	AGridironCharacter* GetPawnOwner() const;

	// Checks to see if the pawn owner is locally controlled.
	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsPawnOwnerLocallyControlled() const;

	// Variable replication setup.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// What is the localized name of this item?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText ItemName;

	// Gets the 3P Mesh
	UFUNCTION(BlueprintPure, Category = "Mesh")
	USkeletalMeshComponent* GetMesh3P() const;

protected:

	// Who "owns" this item?
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_PawnOwner, BlueprintReadOnly, Category = "Item")
	AGridironCharacter* PawnOwner;

	UFUNCTION()
	void OnRep_PawnOwner();

	// Item mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* Mesh3P;
};
