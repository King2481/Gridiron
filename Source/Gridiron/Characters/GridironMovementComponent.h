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

	uint8 RequestToStartDash : 1;

	uint8 RequestToStartAim : 1;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	UFUNCTION(BlueprintCallable, Category = "Character Movement: Dash")
	void StartDash();

	UFUNCTION(BlueprintCallable, Category = "Character Movement: Dash")
	void EndDash();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character Movement: Dash")
	float GroundDashMultiplier;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character Movement: Dash")
	float AirbornDashMultiplier;

	void SetRequestToStartAim(const bool bNewAim);

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character Movement: Aiming")
	float AimSpeedMultiplier;
	
};
