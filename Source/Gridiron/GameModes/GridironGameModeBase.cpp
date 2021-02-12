// Created by Bruce Crum

#include "GridironGameModeBase.h"
#include "GridironGameState.h"
#include "Gridiron/Player/GridironPlayerController.h"
#include "Gridiron/Player/GridironPlayerState.h"
#include "Gridiron/Characters/GridironCharacter.h"
#include "Gridiron/UI/GridironHUD.h"

AGridironGameModeBase::AGridironGameModeBase()
{
	static ConstructorHelpers::FClassFinder<AGridironCharacter>BPPawnClass(TEXT("/Game/Game/Actors/Characters/BP_Character_Player"));
	DefaultPawnClass = BPPawnClass.Class;

	GameStateClass = AGridironGameState::StaticClass();
	PlayerControllerClass = AGridironPlayerController::StaticClass();
	PlayerStateClass = AGridironPlayerState::StaticClass();
	HUDClass = AGridironHUD::StaticClass();
}

void AGridironGameModeBase::OnCharacterKilled(AGridironCharacter* Victim, float KillingDamage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!Victim)
	{
		UE_LOG(LogGameMode, Warning, TEXT("A character was killed, but the victim was nullptr."));
		return;
	}

	const auto VictimPS = Cast<AGridironPlayerState>(Victim->GetPlayerState());
	const auto KillerPS = EventInstigator ? EventInstigator->GetPlayerState<AGridironPlayerState>() : nullptr;
	const auto GS = Cast<AGridironGameState>(GameState);
	const bool bSelfKill = VictimPS == KillerPS;

	if (VictimPS && KillerPS)
	{
		UE_LOG(LogGameMode, VeryVerbose, TEXT("%s was killed by %s"), *VictimPS->GetPlayerName(), *KillerPS->GetPlayerName());
	}
}

float AGridironGameModeBase::OnCharacterTakeDamage(AGridironCharacter* Reciever, float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float AlteredDamage = Damage;

	return AlteredDamage;
}