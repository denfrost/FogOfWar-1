// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDown/TopDownCamera.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

// Sets default values
ATopDownCamera::ATopDownCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = Collision;

	Handle = CreateDefaultSubobject<USpringArmComponent>("Handle");
	Handle->SetupAttachment(Collision);
	Handle->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Handle);

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
}

void ATopDownCamera::BeginPlay()
{
	Super::BeginPlay();

	TargetDistance = MaxDistance;
	TargetFOV = MinFOV;
	TargetPitch = MaxPitch;
	TargetSpeed = MaxSpeed;

	Handle->TargetArmLength = TargetDistance;
	Camera->FieldOfView = TargetFOV;
	Handle->SetRelativeRotation(FRotator(-TargetPitch, 0.0f, 0.0f));
	Movement->MaxSpeed = TargetSpeed;
}

// Called to bind functionality to input
void ATopDownCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATopDownCamera::OnMoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATopDownCamera::OnMoveRight);
	PlayerInputComponent->BindAxis("ZoomCamera", this, &ATopDownCamera::OnZoomCamera);
}

void ATopDownCamera::OnMoveForward(float Value)
{
	// Yaw ���� ����
	FRotator Rotation(0.0f, Handle->GetRelativeRotation().Yaw, 0.0f);

	AddMovementInput(Rotation.Vector(), Value);
}

void ATopDownCamera::OnMoveRight(float Value)
{
	// Yaw ���� ����
	FRotator Rotation(0.0f, Handle->GetRelativeRotation().Yaw, 0.0f);

	// ������ ���� ����
	const FVector& Right = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);

	AddMovementInput(Right, Value);
}

void ATopDownCamera::OnZoomCamera(float Value)
{
	if (GetWorld() == nullptr)
	{
		return;
	}

	float AxisValue = Value;

	auto InterpValue = [&](float Min, float Max, float Current, float& Target)
	{
		float Strength = (Max - Min) / static_cast<float>(NumberOfZoomLevel) * AxisValue;

		Target = FMath::Clamp(Target - Strength, Min, Max);

		return FMath::FInterpTo(Current, Target, GetWorld()->GetDeltaSeconds(), InterpSpeed);
	};

	// ī�޶� Ȯ��/��� ����
	Handle->TargetArmLength = InterpValue(MinDistance, MaxDistance, Handle->TargetArmLength, TargetDistance);

	// ī�޶� �̵� �ӵ� ����
	Movement->MaxSpeed = InterpValue(MinSpeed, MaxSpeed, Movement->MaxSpeed, TargetSpeed);

	// ���� ����
	FRotator Rotation = Handle->GetRelativeRotation();

	// ���� ������ ���
	float PitchStrength = (MaxPitch - MinPitch) / static_cast<float>(NumberOfZoomLevel) * AxisValue;

	// ���� ����
	TargetPitch = FMath::Clamp(TargetPitch - PitchStrength, MinPitch, MaxPitch);

	// ���� ��������
	Rotation.Pitch = -FMath::FInterpTo(-Rotation.Pitch, TargetPitch, GetWorld()->GetDeltaSeconds(), InterpSpeed);
	Handle->SetRelativeRotation(Rotation);

	// FOV ����
	float FOV = Camera->FieldOfView;

	// FOV ������ ���
	float FOVStrength = (MaxFOV - MinFOV) / static_cast<float>(NumberOfZoomLevel) * AxisValue;

	// FOV ����
	TargetFOV = FMath::Clamp(TargetFOV + FOVStrength, MinFOV, MaxFOV);

	// FOV ��������
	FOV = FMath::FInterpTo(FOV, TargetFOV, GetWorld()->GetDeltaSeconds(), InterpSpeed);
	Camera->FieldOfView = FOV;
}
