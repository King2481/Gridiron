// Created by Bruce Crum


#include "Gridiron/Characters/GridironMovementComponent.h"
#include "Gridiron/Characters/GridironCharacter.h"

UGridironMovementComponent::UGridironMovementComponent()
{
	OwningCharacter = nullptr;
	
	RequestToStartDash = false;
	GroundDashMultiplier = 2.f;
	AirbornDashMultiplier = 1.1f;

	RequestToStartAim = false;
	AimSpeedMultiplier = 0.4f;

	bIsSliding = false;
	StartingSlideSpeed = 1500.f;
	CurrentSlideSpeed = 0.f;
	SlideDecrementRate = 25.f;
	SpeedNeededToEndSlide = 50.f;
}

float UGridironMovementComponent::GetMaxSpeed() const
{
	if (bIsSliding)
	{
		return CurrentSlideSpeed;
	}

	float MaxSpeed = Super::GetMaxSpeed();

	if (RequestToStartAim)
	{
		MaxSpeed *= AimSpeedMultiplier;
	}

	return MaxSpeed;
}

void UGridironMovementComponent::StartDash()
{
	RequestToStartDash = true;

	Velocity = FVector::ZeroVector;

	FVector LaunchVelocity = Acceleration;
	LaunchVelocity.Z = 0.f;
	LaunchVelocity *= IsFalling() ? AirbornDashMultiplier : GroundDashMultiplier;

	Launch(LaunchVelocity);
}

void UGridironMovementComponent::EndDash()
{
	RequestToStartDash = false;
}

void UGridironMovementComponent::SetRequestToStartAim(const bool bNewAim)
{
	RequestToStartAim = bNewAim;
}

void UGridironMovementComponent::OnBeginSlide()
{
	CurrentSlideSpeed = StartingSlideSpeed;
}

void UGridironMovementComponent::OnEndSlide()
{
	if (OwningCharacter)
	{
		OwningCharacter->OnEndSlide();
	}
}

void UGridironMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	//The Flags parameter contains the compressed input flags that are stored in the saved move.
	//UpdateFromCompressed flags simply copies the flags from the saved move into the movement component.
	//It basically just resets the movement component to the state when the move was made so it can simulate from there.
	RequestToStartDash = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	RequestToStartAim = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
	bIsSliding = (Flags & FSavedMove_Character::FLAG_Custom_2) != 0;
}

FNetworkPredictionData_Client * UGridironMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);

	if (!ClientPredictionData)
	{
		UGridironMovementComponent* MutableThis = const_cast<UGridironMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FGridironNetworkPredictionData_Client(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UGridironMovementComponent::FGridironSavedMove::Clear()
{
	Super::Clear();

	SavedRequestToStartDash = false;
	SavedRequestToStartAim = false;
	SavedRequestToStartSlide = false;
}

uint8 UGridironMovementComponent::FGridironSavedMove::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (SavedRequestToStartDash)
	{
		Result |= FLAG_Custom_0;
	}

	if (SavedRequestToStartAim)
	{
		Result |= FLAG_Custom_1;
	}

	if (SavedRequestToStartSlide)
	{
		Result |= FLAG_Custom_2;
	}

	return Result;
}

bool UGridironMovementComponent::FGridironSavedMove::CanCombineWith(const FSavedMovePtr & NewMove, ACharacter * Character, float MaxDelta) const
{
	//Set which moves can be combined together. This will depend on the bit flags that are used.
	if (SavedRequestToStartDash != ((FGridironSavedMove*)&NewMove)->SavedRequestToStartDash)
	{
		return false;
	}

	if (SavedRequestToStartAim != ((FGridironSavedMove*)&NewMove)->SavedRequestToStartAim)
	{
		return false;
	}

	if (SavedRequestToStartSlide != ((FGridironSavedMove*)&NewMove)->SavedRequestToStartSlide)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UGridironMovementComponent::FGridironSavedMove::SetMoveFor(ACharacter * Character, float InDeltaTime, FVector const & NewAccel, FNetworkPredictionData_Client_Character & ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UGridironMovementComponent* CharacterMovement = Cast<UGridironMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		SavedRequestToStartDash = CharacterMovement->RequestToStartDash;
		SavedRequestToStartAim = CharacterMovement->RequestToStartAim;
		SavedRequestToStartSlide = CharacterMovement->bIsSliding;
	}
}

void UGridironMovementComponent::FGridironSavedMove::PrepMoveFor(ACharacter * Character)
{
	Super::PrepMoveFor(Character);

	UGridironMovementComponent* CharacterMovement = Cast<UGridironMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
	}
}

UGridironMovementComponent::FGridironNetworkPredictionData_Client::FGridironNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UGridironMovementComponent::FGridironNetworkPredictionData_Client::AllocateNewMove()
{
	return FSavedMovePtr(new FGridironSavedMove());
}