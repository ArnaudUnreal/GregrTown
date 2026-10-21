// GregrTown - AI

#include "AI/GregrMonster.h"

#include "AI/MonsterAIController.h"
#include "AI/MonsterSubsystem.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Camera/PlayerCameraManager.h"
#include "Combat/HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Scoring/ScoreSubsystem.h"
#include "TimerManager.h"
#include "UI/GregrMonsterHealthWidget.h"

AGregrMonster::AGregrMonster()
{
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = AMonsterAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 540.f, 0.f);
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->BrakingDecelerationWalking = 1500.f;
	// The stairs out of the water are a 45 degree ramp (NavLink_WaterStairs in the level).
	Movement->SetWalkableFloorAngle(46.f);
	// Crowds of monsters slide around each other instead of piling up.
	Movement->bUseRVOAvoidance = true;
	Movement->AvoidanceConsiderationRadius = 300.f;

	// A crowd around the player must not pull the camera boom in.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Hundreds of monsters: animate only what is on screen.
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
	GetMesh()->bEnableUpdateRateOptimizations = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	HealthComponent->MaxHealth = 45.f;

	HealthBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBar->SetupAttachment(RootComponent);
	HealthBar->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBar->SetWidgetClass(UGregrMonsterHealthWidget::StaticClass());
	HealthBar->SetDrawSize(FVector2D(80.f, 10.f));
	HealthBar->SetPivot(FVector2D(0.5f, 1.f));
	HealthBar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBar->SetGenerateOverlapEvents(false);
}

void AGregrMonster::BeginPlay()
{
	Super::BeginPlay();

	SetRunning(false);
	HealthComponent->OnHealthChanged.AddUniqueDynamic(this, &AGregrMonster::HandleHealthChanged);
	HealthComponent->OnDied.AddUniqueDynamic(this, &AGregrMonster::HandleDied);

	// Above the head, whatever the capsule size of the child class.
	HealthBar->SetRelativeLocation(FVector(0.f, 0.f, GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() + 30.f));
	HealthBar->InitWidget();
	if (UGregrMonsterHealthWidget* Widget = Cast<UGregrMonsterHealthWidget>(HealthBar->GetUserWidgetObject()))
	{
		Widget->SetPercent(1.f);
	}
	// Spread the checks of the crowd over the frames.
	GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AGregrMonster::UpdateHealthBar, 0.2f, true, FMath::FRandRange(0.01f, 0.2f));
	HealthBar->SetVisibility(false);

	if (UMonsterSubsystem* Monsters = UWorld::GetSubsystem<UMonsterSubsystem>(GetWorld()))
	{
		Monsters->RegisterMonster(this);
	}
}

void AGregrMonster::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	if (UMonsterSubsystem* Monsters = UWorld::GetSubsystem<UMonsterSubsystem>(GetWorld()))
	{
		Monsters->UnregisterMonster(this);
	}
	Super::EndPlay(EndPlayReason);
}

bool AGregrMonster::IsDead() const
{
	return HealthComponent->IsDead();
}

bool AGregrMonster::IsStunned() const
{
	return GetWorld()->GetTimeSeconds() < StunnedUntil;
}

bool AGregrMonster::IsHealthBarVisible() const
{
	return HealthBar->IsVisible();
}

void AGregrMonster::SetRunning(bool bRun)
{
	bRunning = bRun;
	GetCharacterMovement()->MaxWalkSpeed = bRun ? RunSpeed : WalkSpeed;
}

void AGregrMonster::SetFaceFocus(bool bFace)
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = !bFace;
	Movement->bUseControllerDesiredRotation = bFace;
}

bool AGregrMonster::CanAttack() const
{
	return !IsDead() && !bAttacking && !IsStunned() && GetWorld()->GetTimeSeconds() - LastAttackTime >= AttackInterval;
}

bool AGregrMonster::Attack(AActor* Target)
{
	if (!Target || !CanAttack())
	{
		return false;
	}

	const float Length = PlayAnim(AttackAnim);
	if (Length <= 0.f)
	{
		return false;
	}

	bAttacking = true;
	AttackTarget = Target;
	LastAttackTime = GetWorld()->GetTimeSeconds();
	SetFaceFocus(true);
	GetWorldTimerManager().SetTimer(AttackImpactTimer, this, &AGregrMonster::ApplyAttackDamage, FMath::Min(AttackImpactTime, Length), false);
	GetWorldTimerManager().SetTimer(AttackEndTimer, this, &AGregrMonster::EndAttack, Length, false);
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
	}
	return true;
}

void AGregrMonster::ApplyAttackDamage()
{
	AActor* Target = AttackTarget.Get();
	if (!Target || IsDead())
	{
		return;
	}

	// The target may have stepped away or around during the swing.
	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	const bool bInReach = ToTarget.Size2D() <= AttackRange + 60.f;
	const bool bInFront = FVector::DotProduct(ToTarget.GetSafeNormal2D(), GetActorForwardVector()) > 0.2f;
	if (bInReach && bInFront)
	{
		UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
	}
}

void AGregrMonster::EndAttack()
{
	bAttacking = false;
	AttackTarget.Reset();
	GetWorldTimerManager().ClearTimer(AttackImpactTimer);
	GetWorldTimerManager().ClearTimer(AttackEndTimer);
	SetFaceFocus(false);
}

void AGregrMonster::ReceiveDestructionImpact_Implementation(const FDestructionImpact& Impact)
{
	const FVector Location = GetActorLocation();
	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, HitEffect, Location);
	}
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, Location);
	}

	// Pushed away from whoever hit it.
	FVector Away = Impact.Instigator ? Location - Impact.Instigator->GetActorLocation() : -GetActorForwardVector();
	Away = Away.GetSafeNormal2D();
	LaunchCharacter(Away * KnockbackSpeed + FVector(0.f, 0.f, 150.f), true, true);

	APawn* InstigatorPawn = Cast<APawn>(Impact.Instigator.Get());
	AController* InstigatorController = InstigatorPawn ? InstigatorPawn->GetController() : nullptr;
	// Hit from behind: turns on whoever hit it.
	AMonsterAIController* AI = Cast<AMonsterAIController>(GetController());
	if (AI && InstigatorPawn && InstigatorPawn->IsPlayerControlled())
	{
		AI->SetTarget(InstigatorPawn);
	}
	UGameplayStatics::ApplyDamage(this, DamagePerImpact * Impact.Strength, InstigatorController,
		Impact.Causer ? Cast<AActor>(Impact.Causer.Get()) : Impact.Instigator.Get(), nullptr);
}

bool AGregrMonster::CanReceiveDestructionImpact_Implementation(const FDestructionImpact& Impact) const
{
	return !IsDead();
}

void AGregrMonster::HandleHealthChanged(float NewHealth, float Delta)
{
	if (UGregrMonsterHealthWidget* Widget = Cast<UGregrMonsterHealthWidget>(HealthBar->GetUserWidgetObject()))
	{
		Widget->SetPercent(HealthComponent->GetHealthPercent());
	}

	if (Delta < 0.f && NewHealth > 0.f)
	{
		// The reaction cancels the attack in progress.
		EndAttack();
		const float Length = PlayAnim(HitReactAnim);
		StunnedUntil = GetWorld()->GetTimeSeconds() + Length;
		if (AController* MonsterController = GetController())
		{
			MonsterController->StopMovement();
		}
	}
}

void AGregrMonster::HandleDied(AActor* Killer)
{
	EndAttack();
	HealthBar->SetVisibility(false);
	GetWorldTimerManager().ClearTimer(HealthBarTimer);

	if (AController* MonsterController = GetController())
	{
		MonsterController->StopMovement();
	}
	GetCharacterMovement()->StopMovementImmediately();
	// The body no longer blocks the player.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
	const float Length = PlayAnim(DeathAnim, true);
	SetLifeSpan(FMath::Max(Length, 0.1f) + CorpseDelay);

	if (UScoreSubsystem* Score = UWorld::GetSubsystem<UScoreSubsystem>(GetWorld()))
	{
		const FVector Head = GetActorLocation() + FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		Score->AddScore(KillPoints, this, Killer, Head);
	}
	if (UMonsterSubsystem* Monsters = UWorld::GetSubsystem<UMonsterSubsystem>(GetWorld()))
	{
		Monsters->NotifyMonsterKilled(this);
	}
}

void AGregrMonster::UpdateHealthBar()
{
	bool bShow = !IsDead();
	if (bShow)
	{
		const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
		bShow = Camera && FVector::DistSquared(Camera->GetCameraLocation(), GetActorLocation()) < FMath::Square(HealthBarMaxDistance)
			&& GetMesh()->WasRecentlyRendered(0.25f);
	}
	if (bShow != HealthBar->IsVisible())
	{
		HealthBar->SetVisibility(bShow);
	}
}

float AGregrMonster::PlayAnim(UAnimSequenceBase* Anim, bool bHoldLastFrame)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!Anim || !AnimInstance)
	{
		return 0.f;
	}

	UAnimMontage* Montage = UAnimMontage::CreateSlotAnimationAsDynamicMontage(Anim, TEXT("DefaultSlot"), 0.15f, 0.2f);
	if (!Montage)
	{
		return 0.f;
	}
	Montage->bEnableAutoBlendOut = !bHoldLastFrame;
	return AnimInstance->Montage_Play(Montage);
}
