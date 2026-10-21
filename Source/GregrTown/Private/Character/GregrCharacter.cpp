// GregrTown - character

#include "Character/GregrCharacter.h"

#include "Camera/CameraComponent.h"
#include "Character/FallRecoveryComponent.h"
#include "Combat/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"

AGregrCharacter::AGregrCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// Character turns towards movement, camera is free.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 500.f, 0.f);
	Movement->JumpZVelocity = WalkJumpZVelocity;
	Movement->AirControl = 0.35f;
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->MinAnalogWalkSpeed = 20.f;
	Movement->BrakingDecelerationWalking = 2000.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 450.f;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 60.f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	FallRecoveryComponent = CreateDefaultSubobject<UFallRecoveryComponent>(TEXT("FallRecovery"));
}

void AGregrCharacter::BeginPlay()
{
	Super::BeginPlay();

	BaseFieldOfView = FollowCamera->FieldOfView;
	BaseArmLength = CameraBoom->TargetArmLength;
	SetRunning(false);
}

void AGregrCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// The run effect follows the actual horizontal speed, so it fades in while accelerating
	// and stays on during a running jump.
	const float SpeedRange = RunSpeed - WalkSpeed;
	const float TargetAlpha = SpeedRange > 0.f ? FMath::Clamp((GetVelocity().Size2D() - WalkSpeed) / SpeedRange, 0.f, 1.f) : 0.f;
	const float NewAlpha = FMath::FInterpTo(RunCameraAlpha, bRunning ? TargetAlpha : 0.f, DeltaSeconds, RunCameraInterpSpeed);
	if (NewAlpha != RunCameraAlpha)
	{
		RunCameraAlpha = NewAlpha;
		UpdateRunCamera(RunCameraAlpha);
	}
}

void AGregrCharacter::SetRunning(bool bRun)
{
	bRunning = bRun;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->JumpZVelocity = bRun ? RunJumpZVelocity : WalkJumpZVelocity;
	// The combat component slows the character down during attacks: let it apply the new speed.
	CombatComponent->SetBaseWalkSpeed(bRun ? RunSpeed : WalkSpeed);
}

void AGregrCharacter::UpdateRunCamera(float Alpha)
{
	const float FieldOfView = FMath::Lerp(BaseFieldOfView, RunFieldOfView, Alpha);
	FollowCamera->SetFieldOfView(FieldOfView);

	// Dolly zoom: keep the frame width at the character constant, ArmLength * tan(FOV / 2).
	const float HalfFOVTan = FMath::Tan(FMath::DegreesToRadians(FieldOfView * 0.5f));
	if (HalfFOVTan > UE_KINDA_SMALL_NUMBER)
	{
		CameraBoom->TargetArmLength = BaseArmLength * FMath::Tan(FMath::DegreesToRadians(BaseFieldOfView * 0.5f)) / HalfFOVTan;
	}
}

void AGregrCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			for (UInputMappingContext* Context : DefaultMappingContexts)
			{
				if (Context)
				{
					Subsystem->AddMappingContext(Context, 0);
				}
			}
		}
	}
}

void AGregrCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		return;
	}

	if (MoveAction)
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGregrCharacter::Move);
	}
	if (LookAction)
	{
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGregrCharacter::Look);
	}
	if (MouseLookAction)
	{
		Input->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AGregrCharacter::Look);
	}
	if (JumpAction)
	{
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	if (AttackAction)
	{
		Input->BindAction(AttackAction, ETriggerEvent::Started, this, &AGregrCharacter::Attack);
	}
	if (RunAction)
	{
		Input->BindAction(RunAction, ETriggerEvent::Started, this, &AGregrCharacter::StartRun);
		Input->BindAction(RunAction, ETriggerEvent::Completed, this, &AGregrCharacter::StopRun);
	}
}

void AGregrCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller)
	{
		return;
	}

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Axis.X);
}

void AGregrCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AGregrCharacter::Attack()
{
	CombatComponent->TryAttack();
}
