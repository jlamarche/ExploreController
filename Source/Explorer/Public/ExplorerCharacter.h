// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "ExplorerCharacter.generated.h"

/**
 Character controller that supports third and first person modes. Third person supports zoom and both the default follow camera and an Arkham City / Dark Souls style smooth follow camera
 
References & Credits:
 
 Follow camera logic adapted from https://www.youtube.com/watch?v=UMcmqsMzcFg

 */

//////////////////////////////////////////////////////////////////////////
// Camera Mode Enumeration
#pragma mark Camera Mode Enumeration

UENUM(BlueprintType)
namespace ECharacterCameraMode
{
    enum    Type
    {
        ThirdPersonDefault      UMETA(DisplayName="Third Person (Default UE4)"),
        FirstPerson             UMETA(DisplayName="First Person"),
        ThirdPersonFollow       UMETA(DisplayName="Third Person Follow"),

        Max                     UMETA(Hidden),
    };
}

static inline bool IsFirstPerson(const ECharacterCameraMode::Type CameraMode)
{
    return (CameraMode == ECharacterCameraMode::FirstPerson);
}
static inline bool IsThirdPerson(const ECharacterCameraMode::Type CameraMode)
{
    return  !IsFirstPerson(CameraMode);
}
static inline FString GetNameForCameraMode(const ECharacterCameraMode::Type CameraMode)
{
    switch(CameraMode)
    {
        case ECharacterCameraMode::ThirdPersonDefault:
            return TEXT("Third Person (Default UE4)");
            break;
        case ECharacterCameraMode::FirstPerson:
            return TEXT("First Person");
            break;
        case ECharacterCameraMode::ThirdPersonFollow:
            return TEXT("Third Person Follow");
            break;
        default:
            return TEXT("Unknown Camera Mode");
    }
    return NULL;
}

#pragma mark - AExplorerCharacter

UCLASS(config=Game)
class AExplorerCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

    //////////////////////////////////////////////////////////////////////////
    // Public Attributes

public:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	TSubobjectPtr<class USpringArmComponent> CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	TSubobjectPtr<class UCameraComponent> FollowCamera;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

    /** The current camera mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
    TEnumAsByte<ECharacterCameraMode::Type> CameraModeEnum;

    /** Controls the follow camera turn angle. Only affects Third Person Follow mode. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SmoothFollowCamera)
    float CameraFollowTurnAngleExponent;

    /** Controls the follow camera turn speed. Only affects Third Person Follow mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SmoothFollowCamera)
    float CameraFollowTurnRate;

    /** Controls the speed that the camera resets in Third Person Follow mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SmoothFollowCamera)
    float CameraResetSpeed;

    /** Minimum distance for follow cameras */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraZoom)
    float CameraZoomMinimumDistance;

    /** Minimum distance for follow cameras */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraZoom)
    float CameraZoomMaximumDistance;

    /** Zoom increment for follow cameras */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraZoom)
    float CameraZoomIncrement;

    /** Whether the smooth follow camera should be reset to behind character after being idle for a time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SmoothFollowCameraReset)
    bool AutoResetSmoothFollowCameraWhenIdle;

    /** The delay to use if using Auto Reset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SmoothFollowCameraReset)
    float AutoResetDelaySeconds;

    /** The speed to use for Auto Resets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SmoothFollowCameraReset)
    float AutoResetSpeed;

    //////////////////////////////////////////////////////////////////////////
    // Protected Attributes

protected:
    /** Keeps track of whether the camera is currently being reset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraInternal)
    bool IsResetting;

    /** Controls the speed that the camera resets in Third Person Follow mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraInternal)
    float CameraZoomCurrent;

    /** Keeps track of when the last movement was */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraInternal)
    float LastMovementTime;

    /** Keeps track of whether a reset is automatic or manual */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraInternal)
    bool IsAutoReset;




    //////////////////////////////////////////////////////////////////////////
    // Method Declarations

protected:

    //////////////////////////////////////////////////////////////////////////
    // Camera Mode
#pragma mark Camera Mode

    /** Cycles to the next camera mode */
    void CycleCamera();

    /**
	 * Sets the camera mode to a specific value and updates mesh visibility for the new camera mode.
	 * @param newCameraMode	The new camera mode value
	 */
    void SetCameraMode(ECharacterCameraMode::Type newCameraMode);

    /** Handler for reset camera button */
    void ResetCamera();

    /** Handles setting of properties based on camera mode value */
    void UpdateForCameraMode();

    /**
     * Whether the current camera mode is a first person mode.
     */
    bool IsInFirstPersonMode();

    /**
     * Whether the current camera mode is a third person mode.
     */
    bool IsInThirdPersonMode();


    //////////////////////////////////////////////////////////////////////////
    // Camera Zoom
#pragma mark Camera Zoom

    /**
     * Zooms the camera in one increment
     */
    void ZoomCameraIn();

    /**
     * Zooms the camera out one increment
     */
    void ZoomCameraOut();


    //////////////////////////////////////////////////////////////////////////
    // Movement
#pragma mark Movement

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

    /**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

    /**
	 * Handles the Turn Axis, handles both third and first person movement.
	 * @param turnInput	The "Turn" axis value
	 */
    void HandleYawInput(float turnInput);

    /** Handles jump. Just passes on to &ACharacter::Jump, but time is tracked for Camera Auto Reset */
    void HandleJump();

    //////////////////////////////////////////////////////////////////////////
    // Touch Input
#pragma mark Touch Input

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);


    //////////////////////////////////////////////////////////////////////////
    // APawn Overrides
#pragma mark APawn Overrides

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;


    //////////////////////////////////////////////////////////////////////////
    // AActor Overrides
#pragma mark AActor Overrides

    virtual void Tick(float DeltaSeconds);

};

