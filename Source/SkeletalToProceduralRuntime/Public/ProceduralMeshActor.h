// Copyright Roman Kryvosheienko. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshActor.generated.h"

UCLASS()
class SKELETALTOPROCEDURALRUNTIME_API AProceduralMeshActor : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(Category = StaticMeshActor, VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Mesh,Rendering,Physics,Components|StaticMesh", AllowPrivateAccess = "true"))
	TObjectPtr<class UProceduralMeshComponent> ProceduralMeshComponent;

	UPROPERTY(Category = StaticMeshActor, VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Mesh,Rendering,Physics,Components|StaticMesh", AllowPrivateAccess = "true"))
	TObjectPtr<class UBillboardComponent> BillboardComponent;
	
public:	
	// Sets default values for this actor's properties
	AProceduralMeshActor();

	UProceduralMeshComponent* GetProceduralMeshComponent() const { return ProceduralMeshComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
