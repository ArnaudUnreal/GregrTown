// GregrTown - character

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GregrCharacter.generated.h"

class UCameraComponent;
class UCombatComponent;
class UFallRecoveryComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
struct FInputActionValue;

/** Third person player character: camera, Enhanced Input and a combat component. */
UCLASS(Blueprintable)
class GREGRTOWN_API AGregrCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGregrCharacter();

	UFUNCTION(BlueprintPure, Category = "Combat")
	UCombatComponent* GetCombatComponent() const { return CombatComponent; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	UFallRecoveryComponent* GetFallRecoveryComponent() const { return FallRecoveryComponent; }

	/** Switches between walk and run speeds and jump velocities. */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetRunning(bool bRun);

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsRunning() const { return bRunning; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkJumpZVelocity = 600.f;

	/** Higher than walking: running jumps go higher and farther. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunJumpZVelocity = 1200.f;

	/** Camera FOV at full run speed. The camera boom gets shorter so the character keeps the same size on screen (dolly zoom). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "60", ClampMax = "150"))
	float RunFieldOfView = 105.f;

	/** How fast the camera follows the run effect. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0"))
	float RunCameraInterpSpeed = 5.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Attack();
	void StartRun() { SetRunning(true); }
	void StopRun() { SetRunning(false); }

	/** FOV and boom length for a run effect amount in [0, 1]. */
	void UpdateRunCamera(float Alpha);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UFallRecoveryComponent> FallRecoveryComponent;

	/** Added to the local player when possessed. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	/** Held to run. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> RunAction;

	bool bRunning = false;

	/** Camera values when walking, read at BeginPlay. */
	float BaseFieldOfView = 90.f;
	float BaseArmLength = 450.f;
	float RunCameraAlpha = 0.f;
};
