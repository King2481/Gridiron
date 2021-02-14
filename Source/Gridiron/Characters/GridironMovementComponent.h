// Created by Bruce Crum

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GridironMovementComponent.generated.h"

class AGridironCharacter;

/**
 * 
 */
UCLASS()
class GRIDIRON_API UGridironMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FGridironSavedMove : public FSavedMove_Character
	{
	public:

		typedef FSavedMove_Character Super;

		///@brief Resets all saved variables.
		virtual void Clear() override;

		///@brief Store input commands in the compressed flags.
		virtual uint8 GetCompressedFlags() const override;

		///@brief This is used to check whether or not two moves can be combined into one.
		///Basically you just check to make sure that the saved variables are the same.
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;

		///@brief Sets up the move before sending it to the server. 
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;
		///@brief Sets variables on character movement component before making a predictive correction.
		virtual void PrepMoveFor(class ACharacter* Character) override;

		// Dash
		uint8 SavedRequestToStartDash : 1;

		// Aiming
		uint8 SavedRequestToStartAim : 1;

		// Sliding
		uint8 SavedRequestToStartSlide : 1;
	};

	class FGridironNetworkPredictionData_Client : public FNetworkPredictionData_Client_Character
	{
	public:
		FGridironNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		///@brief Allocates a new copy of our custom saved move
		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:

	UGridironMovementComponent();

	virtual float GetMaxSpeed() const override;

	// Cache of the owner of this movement component;
	UPROPERTY(BlueprintReadOnly, Category = "Movement Component")
	AGridironCharacter* OwningCharacter;

	// Is this character dashing?
	uint8 RequestToStartDash : 1;

	// Is this character aiming their weapon (movement component version)
	uint8 RequestToStartAim : 1;

	// Is this character sliding accross the floor?
	uint8 bIsSliding : 1;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	// Will start a dash from the character movement component side of things
	void StartDash();

	// Will End a dash from the character movement component side of things
	void EndDash();

	// When dashing from the ground, what is the speed multiplier?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character Movement: Dash")
	float GroundDashMultiplier;

	// When dashing from the air, what is the speed multiplier?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character Movement: Dash")
	float AirbornDashMultiplier;

	// Called when we are aiming and need to update some things in the character movement specifically
	void SetRequestToStartAim(const bool bNewAim);

	// When aiming, what is our ADS speed multiplier?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character Movement: Aiming")
	float AimSpeedMultiplier;

	// Called to begin slide related things for the character movement component.
	void OnBeginSlide();

	// Called to end slide related things for the character movement component.
	void OnEndSlide();

	// When we slide, what is the default speed we start with?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Sliding")
	float StartingSlideSpeed;

	// What is our current slide speed?
	UPROPERTY(BlueprintReadOnly, Category = "Character Movement: Sliding")
	float CurrentSlideSpeed;

	// How slowly do we slow down when sliding?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Sliding")
	float SlideDecrementRate;

	// What is the minimum speed to end a slide?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Sliding")
	float SpeedNeededToEndSlide;
};
