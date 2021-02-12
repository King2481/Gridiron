// Created by Bruce Crum


#include "Gridiron/World/JumpPad.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gridiron/Characters/GridironCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

// Sets default values
AJumpPad::AJumpPad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneRoot;

	// Setup the mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JumpPadMesh"));
	Mesh->SetupAttachment(RootComponent);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AJumpPad::TriggerBeginOverlap);

	JumpTarget = FVector(100.0f, 0.0f, 0.0f);
	JumpTime = 1.0f;
}

void AJumpPad::PostInitProperties()
{
	Super::PostInitProperties();

	// compatibility
	if (AuthoredGravityZ == 0.0f)
	{
		AuthoredGravityZ = GetDefault<UWorld>()->GetDefaultGravityZ();
	}
}

void AJumpPad::Tick(float DeltaTime)
{
	// TODO: I know this is how UT does it, but does it really need to be tied to tick?

	Super::Tick(DeltaTime);

	// Launch the pending actors
	if (PendingJumpActors.Num() > 0)
	{
		for (auto& Actor : PendingJumpActors)
		{
			Launch(Actor);
		}

		PendingJumpActors.Reset();
	}
}

void AJumpPad::TriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Add the actor to be launched if it hasn't already
	if (!PendingJumpActors.Contains(OtherActor) && CanLaunch(OtherActor))
	{
		PendingJumpActors.Add(OtherActor);
	}
}

bool AJumpPad::CanLaunch(AActor* TestActor)
{
	return (Cast<AGridironCharacter>(TestActor) && TestActor->GetLocalRole() >= ROLE_AutonomousProxy);
}

void AJumpPad::Launch(AActor* Actor)
{
	ACharacter* Character = Cast<AGridironCharacter>(Actor);
	if (Character && !Character->IsPendingKillPending())
	{
		//Launch the character to the target
		Character->LaunchCharacter(CalculateJumpVelocity(Character), false, true);
	}
}

FVector AJumpPad::CalculateJumpVelocity(AActor* JumpActor)
{
	FVector Target = ActorToWorld().TransformPosition(JumpTarget) - JumpActor->GetActorLocation();
	const float GravityZ = GetWorld()->GetDefaultGravityZ();
	if (GravityZ > AuthoredGravityZ)
	{
		Target.Z += GetDefault<AGridironCharacter>()->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	}

	float SizeZ = Target.Z / JumpTime + 0.5f * -GravityZ * JumpTime;
	float SizeXY = Target.Size2D() / JumpTime;

	FVector Velocity = Target.GetSafeNormal2D() * SizeXY + FVector(0.0f, 0.0f, SizeZ);

	// Scale the velocity if Character has gravity scaled
	ACharacter* Character = Cast<AGridironCharacter>(JumpActor);
	if (Character && Character->GetCharacterMovement() != NULL && Character->GetCharacterMovement()->GravityScale != 1.0f)
	{
		Velocity *= FMath::Sqrt(Character->GetCharacterMovement()->GravityScale);
	}
	return Velocity;
}
