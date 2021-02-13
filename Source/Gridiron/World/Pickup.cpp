// Created by Bruce Crum


#include "Gridiron/World/Pickup.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Gridiron/Characters/GridironCharacter.h"
#include "Gridiron/Player/GridironPlayerController.h"

// Sets default values
APickup::APickup()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	CollisionComp = CreateOptionalDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	CollisionComp->SetGenerateOverlapEvents(true);
	CollisionComp->SetCollisionProfileName("Pickup");

	SetRootComponent(CollisionComp);

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh3P"));
	Mesh3P->SetupAttachment(RootComponent);
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh3P->CastShadow = false;

	bEnabled = true;
	DisableTime = 15.f;
	PickupSound = nullptr;
}

void APickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickup, bEnabled);
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();

	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnOverlap);
}

void APickup::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	const auto Character = Cast<AGridironCharacter>(OtherActor);
	if (!Character || !CanPickup(Character))
	{
		return;
	}

	HandlePickup(Character);

	bEnabled = false;
	HandleVisibility();

	GetWorldTimerManager().SetTimer(DisableTimerHandle, this, &APickup::OnReenabled, DisableTime);
}

bool APickup::CanPickup(AGridironCharacter* ForUser) const
{
	return BlueprintCanPickup(ForUser);
}

void APickup::HandlePickup(AGridironCharacter* ForUser)
{
	const auto PC = ForUser->GetController<AGridironPlayerController>();
	if (PC)
	{
		PC->ClientPlaySound2D(PickupSound);
	}

	BlueprintHandlePickup(ForUser);
}

void APickup::OnRep_Enabled()
{
	HandleVisibility();
}

void APickup::HandleVisibility()
{
	SetActorHiddenInGame(!bEnabled);
	CollisionComp->SetGenerateOverlapEvents(bEnabled);
}

void APickup::OnReenabled()
{
	bEnabled = true;
	HandleVisibility();
}

