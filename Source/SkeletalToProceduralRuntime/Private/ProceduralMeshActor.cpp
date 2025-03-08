// Copyright Roman Kryvosheienko. All Rights Reserved.

#include "ProceduralMeshActor.h"
#include "ProceduralMeshComponent.h"
#include "Components/BillboardComponent.h"

// Sets default values
AProceduralMeshActor::AProceduralMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetCanBeDamaged(false);

	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMeshComponent"));
	ProceduralMeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	ProceduralMeshComponent->Mobility = EComponentMobility::Static;
	ProceduralMeshComponent->SetGenerateOverlapEvents(false);

	RootComponent = ProceduralMeshComponent;

	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComponent"));
	BillboardComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AProceduralMeshActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProceduralMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

