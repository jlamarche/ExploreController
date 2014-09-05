// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Explorer.h"
#include "ExplorerCharacter.h"
#include "Engine.h"
//////////////////////////////////////////////////////////////////////////
// AExplorerCharacter
#pragma mark Constructor
AExplorerCharacter::AExplorerCharacter(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// Set size for collision capsule
	CapsuleComponent->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	CharacterMovement->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	CharacterMovement->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	CharacterMovement->JumpZVelocity = 600.f;
	CharacterMovement->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = PCIP.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);

	CameraBoom->bUseControllerViewRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FollowCamera"));
	FollowCamera->AttachTo(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUseControllerViewRotation = false; // Camera does not rotate relative to arm

    Mesh->bCastDynamicShadow = true;
    Mesh->CastShadow = true;

    CameraFollowTurnAngleExponent = .5f;
    CameraFollowTurnRate = .6f;
    CameraResetSpeed = 2.f;

    CameraModeEnum = ECharacterCameraMode::ThirdPersonDefault;

    IsResetting = false;


    CameraZoomMaximumDistance = 600.f;
    CameraZoomMinimumDistance = 100.f;
    CameraZoomCurrent = 300.f;
    CameraZoomIncrement = 20.f;
    CameraBoom->TargetArmLength = CameraZoomCurrent;

    AutoResetSmoothFollowCameraWhenIdle = true;
    AutoResetDelaySeconds = 5.f;

    IsAutoReset = false;
    AutoResetSpeed = .15f;
}


//////////////////////////////////////////////////////////////////////////
// Camera Mode
#pragma mark - Camera Mode

void AExplorerCharacter::CycleCamera()
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, "Cycle Camera Requested");
    int newCameraMode = (int)CameraModeEnum + 1;

    if (newCameraMode >= ECharacterCameraMode::Max) newCameraMode = ECharacterCameraMode::ThirdPersonDefault;
    SetCameraMode((ECharacterCameraMode::Type)newCameraMode);
}

void AExplorerCharacter::SetCameraMode(ECharacterCameraMode::Type newCameraMode)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, "Setting Camera Mode to " + GetNameForCameraMode(newCameraMode));
    CameraModeEnum = newCameraMode;
    UpdateForCameraMode();

}
void AExplorerCharacter::ResetCamera()
{
    if (CameraModeEnum != ECharacterCameraMode::ThirdPersonFollow) return;

    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, "Reset Camera Requested");

    IsResetting = true;
}

void AExplorerCharacter::UpdateForCameraMode()
{
    // Changes visibility of first and third person meshes
    switch (CameraModeEnum)
    {
        case ECharacterCameraMode::ThirdPersonDefault:
            IsResetting = false;
            // no break is intentional
        case ECharacterCameraMode::ThirdPersonFollow:
            FollowCamera->AttachTo(CameraBoom, USpringArmComponent::SocketName);
            CameraBoom->TargetArmLength = CameraZoomCurrent;
            Mesh->SetOwnerNoSee(false);
            bUseControllerRotationPitch = false;
            bUseControllerRotationYaw = false;
            bUseControllerRotationRoll = false;
            break;
        case ECharacterCameraMode::FirstPerson:
            CameraBoom->TargetArmLength = 0.f;
            IsResetting = false;
            bUseControllerRotationPitch = true;
            bUseControllerRotationYaw = true;
            bUseControllerRotationRoll = true;
            break;

        default:
            break;
    }
}

bool AExplorerCharacter::IsInFirstPersonMode()
{
    return IsFirstPerson(CameraModeEnum);
}

bool AExplorerCharacter::IsInThirdPersonMode()
{
    return IsThirdPerson(CameraModeEnum);
}

//////////////////////////////////////////////////////////////////////////
// Camera Zoom
#pragma mark - Camera Zoom
void AExplorerCharacter::ZoomCameraIn()
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, "Zoom Camera In Requested");
    CameraZoomCurrent-=CameraZoomIncrement;
    if (CameraZoomCurrent < CameraZoomMinimumDistance)
        CameraZoomCurrent = CameraZoomMinimumDistance;

    CameraBoom->TargetArmLength = CameraZoomCurrent;
}
void AExplorerCharacter::ZoomCameraOut()
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, "Zoom Camera Out Requested");
    CameraZoomCurrent+=CameraZoomIncrement;
    if (CameraZoomCurrent > CameraZoomMaximumDistance)
        CameraZoomCurrent = CameraZoomMaximumDistance;

    CameraBoom->TargetArmLength = CameraZoomCurrent;
}


//////////////////////////////////////////////////////////////////////////
// Movement
#pragma mark - Movement

void AExplorerCharacter::TurnAtRate(float Rate)
{
    if (Rate == 0.f) return;

	// calculate delta for this frame from the rate information
    if (!IsResetting)
    {
        LastMovementTime = FApp::GetCurrentTime();
        AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
    }
}

void AExplorerCharacter::LookUpAtRate(float Rate)
{
    if (Rate == 0.f) return;

    LastMovementTime = FApp::GetCurrentTime();
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AExplorerCharacter::MoveForward(float Value)
{
    if (Value == 0.f) return;

    if (IsInThirdPersonMode())
    {
        if (Controller != NULL)
        {
            // find out which way is forward
            const FRotator Rotation = Controller->GetControlRotation();
            const FRotator YawRotation(0, Rotation.Yaw, 0);

            // get forward vector
            const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
            AddMovementInput(Direction, Value);
        }
    }
    else
    {
            AddMovementInput(GetActorForwardVector(), Value);
    }
    LastMovementTime = FApp::GetCurrentTime();

}

void AExplorerCharacter::MoveRight(float Value)
{
    if (Value == 0.f) return;

    if (IsInThirdPersonMode())
    {
        if ( Controller != NULL )
        {
            // find out which way is right
            const FRotator Rotation = Controller->GetControlRotation();
            const FRotator YawRotation(0, Rotation.Yaw, 0);

            // get right vector
            const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
            // add movement in that direction
            AddMovementInput(Direction, Value);
        }
    }
    else
    {
            AddMovementInput(GetActorRightVector(), Value);

    }
    LastMovementTime = FApp::GetCurrentTime();

}

void AExplorerCharacter::HandleYawInput(float turnInput)
{
    if (!IsResetting)
    {
        if (turnInput != 0.f)
        {
            AddControllerYawInput(turnInput);
            LastMovementTime = FApp::GetCurrentTime();
        }

    }
}

void AExplorerCharacter::HandleJump()
{
    Super::Jump();
    LastMovementTime = FApp::GetCurrentTime();
}

//////////////////////////////////////////////////////////////////////////
// Touch Input
#pragma mark - Touch Input

void AExplorerCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// jump, but only on the first touch
	if (FingerIndex == ETouchIndex::Touch1)
	{
		Jump();
        LastMovementTime = FApp::GetCurrentTime();
	}
}

//////////////////////////////////////////////////////////////////////////
// APawn Overrides
#pragma mark - APawn Overrides

void AExplorerCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// Set up gameplay key bindings
	check(InputComponent);
	InputComponent->BindAction("Jump", IE_Pressed, this, &AExplorerCharacter::HandleJump);
    InputComponent->BindAction("ResetCamera", IE_Pressed, this, &AExplorerCharacter::ResetCamera);
    InputComponent->BindAction("ToggleCameraMode", IE_Pressed, this, &AExplorerCharacter::CycleCamera);
    InputComponent->BindAction("ZoomIn", IE_Pressed, this, &AExplorerCharacter::ZoomCameraIn);
    InputComponent->BindAction("ZoomOut", IE_Pressed, this, &AExplorerCharacter::ZoomCameraOut);

	InputComponent->BindAxis("MoveForward", this, &AExplorerCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AExplorerCharacter::MoveRight);


	InputComponent->BindAxis("Turn", this, &AExplorerCharacter::HandleYawInput);
	InputComponent->BindAxis("TurnRate", this, &AExplorerCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AExplorerCharacter::LookUpAtRate);

	// handle touch devices
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AExplorerCharacter::TouchStarted);
}


//////////////////////////////////////////////////////////////////////////
// AActor Overrides
#pragma mark - AActor Overrides

void AExplorerCharacter::Tick(float DeltaSeconds)
{

    Super::Tick(DeltaSeconds);

    if (CameraModeEnum != ECharacterCameraMode::ThirdPersonFollow) return;

    const FRotator Rotation = Controller->GetControlRotation();

    float currentTime = FApp::GetCurrentTime();
//    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("current time: %f"), currentTime));
//    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("last movement time: %f"), LastMovementTime));
    if (currentTime > LastMovementTime + AutoResetDelaySeconds)
    {
        IsAutoReset = true;
        IsResetting = true;
    }


    if (IsResetting)
    {

        FRotator meshRotation = Mesh->GetTransformMatrix().Rotator();
        meshRotation.Yaw += 90.f;
        meshRotation.Pitch = Controller->GetControlRotation().Pitch;

        float delta = meshRotation.Yaw - Rotation.Yaw;


        // Prevent going "the long way around"
        if (fabsf(delta) >= 180.f)
        {
            if (delta <= 0.f)
                delta +=360.f;
            else
                delta -= 360.f;
        }

        if (fabsf(delta) <= 1.f)
        {
            IsResetting = false;
            IsAutoReset = false;
        }
        else
        {
            float resetSpeed = (IsAutoReset) ? AutoResetSpeed : CameraResetSpeed;
            AddControllerYawInput(delta * DeltaSeconds * resetSpeed);
        }

        return;
    }

    const FRotator YawRotation(0, Rotation.Yaw, 0);
    float forwardAxis = GetInputAxisValue("MoveForward");
    float rightAxis = GetInputAxisValue("MoveRight");

    float inputVectorLength = UKismetMathLibrary::FClamp((fabsf(forwardAxis) + fabsf(rightAxis)), 0.f, 1.f);

    if (inputVectorLength == 0.f) return;

    const FVector forward = UKismetMathLibrary::GetForwardVector(YawRotation);
    const FVector right = UKismetMathLibrary::GetRightVector(YawRotation);

    const FVector forwardVector = UKismetMathLibrary::Multiply_VectorFloat(forward, forwardAxis);
    const FVector rightVector = UKismetMathLibrary::Multiply_VectorFloat(right, rightAxis);

    const FVector combinedVector = UKismetMathLibrary::Normal(forwardVector + rightVector);

    // 1 = character is perpindicular to camera vector, 0 when parallel to camera vector
    float dotProduct = 1 - fabsf(UKismetMathLibrary::Dot_VectorVector(forwardVector, combinedVector));
    dotProduct = UKismetMathLibrary::MultiplyMultiply_FloatFloat(dotProduct, CameraFollowTurnAngleExponent);

    FRotator combined = UKismetMathLibrary::Conv_VectorToRotator(combinedVector);
    FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(combined, Rotation);
    AddControllerYawInput(UKismetMathLibrary::FClamp(inputVectorLength * DeltaSeconds * dotProduct * CameraFollowTurnRate, 0.f, 1.f) * delta.Yaw);


}