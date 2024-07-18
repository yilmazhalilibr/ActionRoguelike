// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SInteractionComponent.h"
#include "SAttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComp");
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(SpringArmComp);

	AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AttributeComp");

	InteractionComp = CreateDefaultSubobject<USInteractionComponent>("InteractionComp");

	GetCharacterMovement()->bOrientRotationToMovement = true;

	bUseControllerRotationYaw = false;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();


}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);


	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Lookup", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("PrimaryAttack", IE_Pressed, this, &ASCharacter::PrimaryAttack);
	PlayerInputComponent->BindAction("PrimaryInteract", IE_Pressed, this, &ASCharacter::PrimaryInteract);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);

}


void ASCharacter::MoveForward(float value)
{
	FRotator controlRot = GetControlRotation();
	controlRot.Pitch = 0.0f;
	controlRot.Roll = 0.0f;

	AddMovementInput(controlRot.Vector(), value);
}

void ASCharacter::MoveRight(float value)
{
	FRotator controlRot = GetControlRotation();
	controlRot.Pitch = 0.0f;
	controlRot.Roll = 0.0f;

	FVector rightVector = FRotationMatrix(controlRot).GetScaledAxis(EAxis::Y);

	AddMovementInput(rightVector, value);
}

void ASCharacter::PrimaryAttack()
{
	PlayAnimMontage(AttackAnim);

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_PrimaryAttack, this, &ASCharacter::PrimaryAttack_TimeElapse, 0.2f);
	//GetWorld()->GetTimerManager().ClearTimer(TimerHandle_PrimaryAttack);

}

void ASCharacter::PrimaryInteract()
{
	if (InteractionComp)
	{
		InteractionComp->PrimaryInteract();
	}
}

void ASCharacter::PrimaryAttack_TimeElapse()
{
	if (ensure(ProjectileClass))
	{
		FVector HandLocation = GetMesh()->GetSocketLocation("Muzzle_01");

		FTransform SpawnTM = FTransform(GetControlRotation(), HandLocation);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = this;
		GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTM, SpawnParams);
	}

}

void ASCharacter::Jump()
{
	UE_LOG(LogTemp, Warning, TEXT("Jump trigged"));
}

void ASCharacter::SpawnProjectile(TSubclassOf<AActor> ClassToSpawn)
{
	if (ensure(ClassToSpawn))
	{
		FVector HandLocatino = GetMesh()->GetSocketLocation("Muzzle_01");

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = this;

		FCollisionShape Shape;
		Shape.SetSphere(20.0f);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjParams.AddObjectTypesToQuery(ECC_Pawn);

		FVector TraceStart = CameraComp->GetComponentLocation();

		FVector TraceEnd = CameraComp->GetComponentLocation() + (CameraComp->GetForwardVector() * 5000.0f);

		FHitResult HitResult;

		if (GetWorld()->SweepSingleByObjectType(HitResult, TraceStart, TraceEnd, FQuat::Identity, ObjParams, Shape, Params))
		{
			TraceEnd = HitResult.ImpactPoint;
		}


		FRotator ProjRotation = FRotationMatrix::MakeFromX(TraceEnd - HandLocatino).Rotator();

		FTransform SpawnTM = FTransform(ProjRotation, HandLocatino);
		GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnTM, SpawnParams);

	}
}

