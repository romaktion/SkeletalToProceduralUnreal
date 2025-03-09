// Copyright Roman Kryvosheienko. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SkeletalToProceduralRuntimeLib.generated.h"

class AProceduralActor;
class ASkeletalMeshActor;
struct FRawMesh;
class UProceduralMeshComponent;
class USkeletalMeshComponent;
class UBillboardComponent;

UCLASS()
class SKELETALTOPROCEDURALRUNTIME_API AProceduralActor : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(Category = StaticMeshActor, VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Mesh,Rendering,Physics,Components|StaticMesh", AllowPrivateAccess = "true"))
	UProceduralMeshComponent* ProceduralMeshComponent;

	UPROPERTY(Category = StaticMeshActor, VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Mesh,Rendering,Physics,Components|StaticMesh", AllowPrivateAccess = "true"))
	UBillboardComponent* BillboardComponent;
	
public:	
	// Sets default values for this actor's properties
	AProceduralActor();

	UProceduralMeshComponent* GetProceduralMeshComponent() const { return ProceduralMeshComponent; }
};

USTRUCT(BlueprintType)
struct FMyUVMapParameters
{
	GENERATED_BODY()

public:
	/** Length, width, height of the UV mapping gizmo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SkeletalToProcedural) FVector Size;

	/** Tiling of the UV mapping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SkeletalToProcedural) FVector2D UVTile;

	/** Position of the UV mapping gizmo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SkeletalToProcedural) FVector Position;

	/** Rotation of the UV mapping gizmo (angles in degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SkeletalToProcedural) FQuat Rotation;

	/** Scale of the UV mapping gizmo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SkeletalToProcedural) FVector Scale;

	/** Default settings */
	FMyUVMapParameters()
		: Size(1.0f)
		, UVTile(1.0f, 1.0f)
		, Position(0.0f)
		, Rotation(ForceInit)
		, Scale(1.0f)
	{
	}

	FMyUVMapParameters(const FVector& InPosition, const FQuat& InRotation, const FVector& InSize, const FVector& InScale, const FVector2D& InUVTile)
		: Size(InSize)
		, UVTile(InUVTile)
		, Position(InPosition)
		, Rotation(InRotation)
		, Scale(InScale)
	{
	}
};

UCLASS()
class SKELETALTOPROCEDURALRUNTIME_API USkeletalToProceduralRuntime : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = SkeletalToProcedural)
	static bool SkeletalToProcedural(USkeletalMeshComponent* SkeletalMeshComponent,
	                                 UProceduralMeshComponent* ProcMeshComponent, bool WithMaterials = true);
	UFUNCTION(BlueprintCallable, Category = SkeletalToProcedural, meta = (DisplayName = "SkeletalToProcedural"))
	static bool SkeletalToProceduralActors(ASkeletalMeshActor* SkeletalMeshActor,
									 AProceduralActor* ProcMeshComponent, bool WithMaterials = true);

	UFUNCTION(BlueprintCallable, Category = SkeletalToProcedural)
	static bool SkeletalToProceduralWithParams(USkeletalMeshComponent* SkeletalMeshComponent,
	                                 UProceduralMeshComponent* ProcMeshComponent, const FMyUVMapParameters& Params, bool WithMaterials = true);
	UFUNCTION(BlueprintCallable, Category = SkeletalToProcedural, meta = (DisplayName = "SkeletalToProceduralWithParams"))
	static bool SkeletalToProceduralWithParamsActors(ASkeletalMeshActor* SkeletalMeshComponent,
									 AProceduralActor* ProcMeshComponent, const FMyUVMapParameters& Params, bool WithMaterials = true);

	UFUNCTION(BlueprintCallable, Category = SkeletalToProcedural)
	static bool StaticToProcedural(UStaticMeshComponent* StaticMeshComponent,
									 UProceduralMeshComponent* ProcMeshComponent);
	
private:
	static TArray<FRawMesh> CollectRawMeshes(const TArray<UMeshComponent*>& InMeshComponents, const FTransform& Transform);
	static bool IsValidSkinnedMeshComponent(const USkinnedMeshComponent* InComponent);
	static bool IsValidStaticMeshComponent(const UStaticMeshComponent* InComponent);
	static void SkinnedMeshToRawMeshes(USkinnedMeshComponent* InSkinnedMeshComponent, int32 InOverallMaxLODs,
	                                   const FMatrix& InComponentToWorld, TArray<FRawMesh>& OutRawMeshes, TArray<UMaterialInterface*>& OutMaterials);
	static void StaticMeshToRawMeshes(const UStaticMeshComponent* InStaticMeshComponent, int32 InOverallMaxLODs,
	                                  const FMatrix& InComponentToWorld, TArray<FRawMesh>& OutRawMeshes, TArray<UMaterialInterface*>& OutMaterials);
	static bool CreateProcMesh(const TArray<FRawMesh>& RawMeshes, UProceduralMeshComponent* ProcMeshComponent,
	                           bool GenerateUV, const FMyUVMapParameters& Params);
};
