// Copyright Roman Kryvosheienko. All Rights Reserved.

#include "SkeletalToProceduralRuntimeLib.h"
#include "SkeletalRenderPublic.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/BillboardComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Runtime/Engine/Private/SkeletalRenderGPUSkin.h"
#include "Runtime/RawMesh/Public/RawMesh.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Engine/StaticMesh.h"

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

bool USkeletalToProceduralRuntime::SkeletalToProcedural(USkeletalMeshComponent* SkeletalMeshComponent,
	UProceduralMeshComponent* ProcMeshComponent)
{
	if (!IsValid(ProcMeshComponent) || !IsValid(SkeletalMeshComponent)) return {};
	
	TArray<UMaterialInterface*> Materials;
	for (int32 i = 0; i < SkeletalMeshComponent->GetNumMaterials(); ++i)
		Materials.Add(SkeletalMeshComponent->GetMaterial(i));
	
	const auto RawMeshes{CollectRawMeshes({SkeletalMeshComponent}, SkeletalMeshComponent->GetComponentTransform())};

	if (!ensureMsgf(RawMeshes.Num() > 0, TEXT("Bad mesh %s"), *SkeletalMeshComponent->GetName()))
		return {};

	CreateProcMesh(RawMeshes, ProcMeshComponent, {}, {});

	for (int32 i = 0; i < Materials.Num(); ++i)
		ProcMeshComponent->SetMaterial(i, Materials[i]);
	
	return true;
}

bool USkeletalToProceduralRuntime::SkeletalToProceduralActors(ASkeletalMeshActor* SkeletalMeshActor,
	AProceduralMeshActor* ProcMeshComponent)
{
	return SkeletalToProcedural(SkeletalMeshActor->GetSkeletalMeshComponent(), ProcMeshComponent->GetProceduralMeshComponent());
}

bool USkeletalToProceduralRuntime::SkeletalToProceduralWithParams(USkeletalMeshComponent* SkeletalMeshComponent,
                                                                  UProceduralMeshComponent* ProcMeshComponent,
                                                                  const FMyUVMapParameters& Params)
{
	if (!IsValid(ProcMeshComponent) || !IsValid(SkeletalMeshComponent)) return {};
	
	TArray<UMaterialInterface*> Materials;
	for (int32 i = 0; i < SkeletalMeshComponent->GetNumMaterials(); ++i)
		Materials.Add(SkeletalMeshComponent->GetMaterial(i));
	
	const auto RawMeshes{CollectRawMeshes({SkeletalMeshComponent}, SkeletalMeshComponent->GetComponentTransform())};

	if (!ensureMsgf(RawMeshes.Num() > 0, TEXT("Bad mesh %s"), *SkeletalMeshComponent->GetName()))
		return {};

	CreateProcMesh(RawMeshes, ProcMeshComponent, true, Params);

	for (int32 i = 0; i < Materials.Num(); ++i)
		ProcMeshComponent->SetMaterial(i, Materials[i]);
	
	return true;
}

bool USkeletalToProceduralRuntime::SkeletalToProceduralWithParamsActors(ASkeletalMeshActor* SkeletalMeshComponent,
	AProceduralMeshActor* ProcMeshComponent, const FMyUVMapParameters& Params)
{
	return SkeletalToProceduralWithParams(SkeletalMeshComponent->GetSkeletalMeshComponent(),
	                               ProcMeshComponent->GetProceduralMeshComponent(), Params);
}

bool USkeletalToProceduralRuntime::StaticToProcedural(UStaticMeshComponent* StaticMeshComponent,
	UProceduralMeshComponent* ProcMeshComponent)
{
	if (!IsValid(ProcMeshComponent) || !IsValid(StaticMeshComponent)) return {};
	
	TArray<UMaterialInterface*> Materials;
	for (int32 i = 0; i < StaticMeshComponent->GetNumMaterials(); ++i)
		Materials.Add(StaticMeshComponent->GetMaterial(i));
	
	const auto RawMeshes{CollectRawMeshes({StaticMeshComponent}, StaticMeshComponent->GetComponentTransform())};

	if (!ensureMsgf(RawMeshes.Num() > 0, TEXT("Bad mesh %s"), *StaticMeshComponent->GetName()))
		return {};

	CreateProcMesh(RawMeshes, ProcMeshComponent, {}, {});

	for (int32 i = 0; i < Materials.Num(); ++i)
		ProcMeshComponent->SetMaterial(i, Materials[i]);
	
	return true;
}

TArray<FRawMesh> USkeletalToProceduralRuntime::CollectRawMeshes(const TArray<UMeshComponent*>& InMeshComponents, const FTransform& Transform)
{
	TArray<FRawMesh> RawMeshes;
	TArray<UMaterialInterface*> Materials;

	// first do a pass to determine the max LOD level we will be combining meshes into
	int32 OverallMaxLODs = 0;
	for (UMeshComponent* MeshComponent : InMeshComponents)
	{
		const USkinnedMeshComponent* SkinnedMeshComponent = Cast<USkinnedMeshComponent>(MeshComponent);
		const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent);

		if (IsValidSkinnedMeshComponent(SkinnedMeshComponent))
		{
			OverallMaxLODs = FMath::Max(
				SkinnedMeshComponent->MeshObject->GetSkeletalMeshRenderData().LODRenderData.Num(), OverallMaxLODs);
		}
		else if (IsValidStaticMeshComponent(StaticMeshComponent))
		{
			OverallMaxLODs = FMath::Max(StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources.Num(),
			                            OverallMaxLODs);
		}
	}

	// Resize raw meshes to accommodate the number of LODs we will need
	RawMeshes.SetNum(OverallMaxLODs);
	const auto WorldToRoot = Transform.ToMatrixWithScale().Inverse();

	// Export all visible components
	for (UMeshComponent* MeshComponent : InMeshComponents)
	{
		FMatrix ComponentToWorld = MeshComponent->GetComponentTransform().ToMatrixWithScale() * WorldToRoot;

		USkinnedMeshComponent* SkinnedMeshComponent = Cast<USkinnedMeshComponent>(MeshComponent);
		UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent);

		if (IsValidSkinnedMeshComponent(SkinnedMeshComponent))
		{
			SkinnedMeshToRawMeshes(SkinnedMeshComponent, OverallMaxLODs, ComponentToWorld, RawMeshes, Materials);
		}
		else if (IsValidStaticMeshComponent(StaticMeshComponent))
		{
			StaticMeshToRawMeshes(StaticMeshComponent, OverallMaxLODs, ComponentToWorld, RawMeshes, Materials);
		}
	}
	
	return RawMeshes;
}

bool USkeletalToProceduralRuntime::IsValidSkinnedMeshComponent(const USkinnedMeshComponent* InComponent)
{
	return InComponent && InComponent->MeshObject && InComponent->IsVisible();
}

bool USkeletalToProceduralRuntime::IsValidStaticMeshComponent(const UStaticMeshComponent* InComponent)
{
	return InComponent && InComponent->GetStaticMesh() && InComponent->GetStaticMesh()->GetRenderData() && InComponent->IsVisible();
}

void USkeletalToProceduralRuntime::SkinnedMeshToRawMeshes(USkinnedMeshComponent* InSkinnedMeshComponent, const int32 InOverallMaxLODs,
                            const FMatrix& InComponentToWorld, TArray<FRawMesh>& OutRawMeshes, TArray<UMaterialInterface*>& OutMaterials)
{
	const int32 BaseMaterialIndex = OutMaterials.Num();

	// Export all LODs to raw meshes
	const int32 NumLODs = InSkinnedMeshComponent->GetNumLODs();

	for (int32 OverallLODIndex = 0; OverallLODIndex < InOverallMaxLODs; OverallLODIndex++)
	{
		int32 LODIndexRead = FMath::Min(OverallLODIndex, NumLODs - 1);

		FRawMesh& RawMesh = OutRawMeshes[OverallLODIndex];
		const int32 BaseVertexIndex = RawMesh.VertexPositions.Num();
		OutMaterials.Add(InSkinnedMeshComponent->GetMaterial(0));

#if ENGINE_MAJOR_VERSION < 5
		FSkeletalMeshLODInfo& SrcLODInfo = *(InSkinnedMeshComponent->SkeletalMesh->GetLODInfo(LODIndexRead));
#else
		FSkeletalMeshLODInfo& SrcLODInfo = *(InSkinnedMeshComponent->GetSkinnedAsset()->GetLODInfo(LODIndexRead));
#endif

		// Get the CPU skinned verts for this LOD
		TArray<FFinalSkinVertex> FinalVertices;
		InSkinnedMeshComponent->GetCPUSkinnedVertices(FinalVertices, LODIndexRead);

		//const auto& PositionVertexBuffer = InSkinnedMeshComponent->GetSkeletalMeshRenderData()->LODRenderData[LODIndexRead].StaticVertexBuffers.PositionVertexBuffer;
		//for (uint32 i = 0; i < PositionVertexBuffer.GetNumVertices(); ++i)
		//	RawMesh.VertexPositions.Add(PositionVertexBuffer.VertexPosition(i));
		
		//const auto & T = InSkinnedMeshComponent->GetSkeletalMeshRenderData()->LODRenderData[LODIndexRead].MorphTargetVertexInfoBuffers.GetNumBatches();
		//const auto & T2 = InSkinnedMeshComponent->GetSkeletalMeshRenderData()->LODRenderData[LODIndexRead].MorphTargetVertexInfoBuffers.GetNumMorphs();
		//const auto T3 = InSkinnedMeshComponent->GetSkeletalMeshRenderData()->LODRenderData[LODIndexRead].MorphTargetVertexInfoBuffers;
//
		//const auto T4 = InSkinnedMeshComponent->ActiveMorphTargets;
		
		for (const auto & M : InSkinnedMeshComponent->GetSkinnedAsset()->GetMorphTargets())
		{
			const auto MO = M.Get();
			int32 Num;
			const auto& T =  MO->GetMorphTargetDelta(LODIndexRead, Num);
			const auto& T2 =  MO->GetMorphTargetDelta(LODIndexRead, Num);
		}
		
		FSkeletalMeshRenderData& SkeletalMeshRenderData = InSkinnedMeshComponent->MeshObject->GetSkeletalMeshRenderData();
		FSkeletalMeshLODRenderData& LODData = SkeletalMeshRenderData.LODRenderData[LODIndexRead];

		//Collect UVs, Normals, Tangents
		const uint32 NumTexCoords = FMath::Min(LODData.StaticVertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords(),
		                                       static_cast<uint32>(MAX_MESH_TEXTURE_COORDS));
		FSkeletalMeshRenderData* SkMeshRenderData = InSkinnedMeshComponent->GetSkeletalMeshRenderData();
		const FSkeletalMeshLODRenderData& DataArray = SkMeshRenderData->LODRenderData[LODIndexRead];
		for (const auto& RenderSection : DataArray.RenderSections)
		{
			//get num vertices
			int32 NumSourceVertices = RenderSection.NumVertices;
		
			for (int32 i = 0; i < NumSourceVertices; i++)
			{
				const auto RealInd = i + RenderSection.BaseVertexIndex;

				for (uint32 TexCoordIndex = 0; TexCoordIndex < MAX_MESH_TEXTURE_COORDS; TexCoordIndex++)
				{
					if (TexCoordIndex >= NumTexCoords)
						RawMesh.WedgeTexCoords[TexCoordIndex].AddDefaulted();
					else
						RawMesh.WedgeTexCoords[TexCoordIndex].Add(
							DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(RealInd, TexCoordIndex));
				}

				//Calc normals and tangents from the static version instead of the skeletal one
				FVector3f ZTangentStatic = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(RealInd);
				FVector3f XTangentStatic = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(RealInd);
				FVector3f YTangentStatic = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentY(RealInd);

				//add normals from the static mesh version instead because using the skeletal one doesn't work right.
				RawMesh.WedgeTangentZ.Add(ZTangentStatic);

				//add tangents
				RawMesh.WedgeTangentX.Add(XTangentStatic);

				//add Y
				RawMesh.WedgeTangentY.Add(YTangentStatic);
			}
		}
		
		// Copy skinned vertex positions
		for (int32 VertIndex = 0; VertIndex < FinalVertices.Num(); ++VertIndex)
		{
#if ENGINE_MAJOR_VERSION >= 5
			RawMesh.VertexPositions.Add(
				static_cast<FVector4f>(InComponentToWorld.TransformPosition(
					static_cast<FVector>(FinalVertices[VertIndex].Position))));
#else
			RawMesh.VertexPositions.Add(InComponentToWorld.TransformPosition(FinalVertices[VertIndex].Position));
#endif
		}

		const int32 NumSections = LODData.RenderSections.Num();
		FRawStaticIndexBuffer16or32Interface& IndexBuffer = *LODData.MultiSizeIndexContainer.GetIndexBuffer();

		for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
		{
			if (const FSkelMeshRenderSection& SkeletalMeshSection = LODData.RenderSections[SectionIndex];
				InSkinnedMeshComponent->IsMaterialSectionShown(SkeletalMeshSection.MaterialIndex, LODIndexRead))
			{
				// Build 'wedge' info
				const int32 NumWedges = SkeletalMeshSection.NumTriangles * 3;
				for (int32 WedgeIndex = 0; WedgeIndex < NumWedges; WedgeIndex++)
				{
					const int32 VertexIndexForWedge = IndexBuffer.Get(SkeletalMeshSection.BaseIndex + WedgeIndex);

					RawMesh.WedgeIndices.Add(BaseVertexIndex + VertexIndexForWedge);

					if (LODData.StaticVertexBuffers.ColorVertexBuffer.IsInitialized())
						RawMesh.WedgeColors.Add(LODData.StaticVertexBuffers.ColorVertexBuffer.VertexColor(VertexIndexForWedge));
					else
						RawMesh.WedgeColors.Add(FColor::White);
				}

				int32 MaterialIndex = SkeletalMeshSection.MaterialIndex;
				// use the remapping of material indices if there is a valid value
				if (SrcLODInfo.LODMaterialMap.IsValidIndex(SectionIndex) && SrcLODInfo.LODMaterialMap[SectionIndex] != INDEX_NONE)
				{
#if ENGINE_MAJOR_VERSION >= 5
					MaterialIndex = FMath::Clamp<int32>(SrcLODInfo.LODMaterialMap[SectionIndex], 0, InSkinnedMeshComponent->GetSkinnedAsset()->GetMaterials().Num());
#else
					MaterialIndex = FMath::Clamp<int32>(SrcLODInfo.LODMaterialMap[SectionIndex], 0, InSkinnedMeshComponent->SkeletalMesh->GetMaterials().Num());
#endif
				}

				// copy face info
				for (uint32 TriIndex = 0; TriIndex < SkeletalMeshSection.NumTriangles; TriIndex++)
				{
					RawMesh.FaceMaterialIndices.Add(BaseMaterialIndex + MaterialIndex);
					RawMesh.FaceSmoothingMasks.Add(0); // Assume this is ignored as bRecomputeNormals is false
				}
			}
		}
	}
}

void USkeletalToProceduralRuntime::StaticMeshToRawMeshes(const UStaticMeshComponent* InStaticMeshComponent,
                                                     int32 InOverallMaxLODs, const FMatrix& InComponentToWorld,
                                                     TArray<FRawMesh>& OutRawMeshes,
                                                     TArray<UMaterialInterface*>& OutMaterials)
{
	const int32 BaseMaterialIndex = OutMaterials.Num();

	const int32 NumLODs = InStaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources.Num();

	for (int32 OverallLODIndex = 0; OverallLODIndex < InOverallMaxLODs; OverallLODIndex++)
	{
		int32 LODIndexRead = FMath::Min(OverallLODIndex, NumLODs - 1);

		FRawMesh& RawMesh = OutRawMeshes[OverallLODIndex];
		const FStaticMeshLODResources& LODResource = InStaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[LODIndexRead];
		const int32 BaseVertexIndex = RawMesh.VertexPositions.Num();

		for (int32 VertIndex = 0; VertIndex < LODResource.GetNumVertices(); ++VertIndex)
		{
#if ENGINE_MAJOR_VERSION >= 5
			RawMesh.VertexPositions.Add(FVector4f(InComponentToWorld.TransformPosition((FVector)LODResource.VertexBuffers.PositionVertexBuffer.VertexPosition((uint32)VertIndex))));
#else
			RawMesh.VertexPositions.Add(InComponentToWorld.TransformPosition(LODResource.VertexBuffers.PositionVertexBuffer.VertexPosition((uint32)VertIndex)));
#endif
		}

		const FIndexArrayView IndexArrayView = LODResource.IndexBuffer.GetArrayView();
		const FStaticMeshVertexBuffer& StaticMeshVertexBuffer = LODResource.VertexBuffers.StaticMeshVertexBuffer;
		const int32 NumTexCoords = FMath::Min(StaticMeshVertexBuffer.GetNumTexCoords(), (uint32)MAX_MESH_TEXTURE_COORDS);
		const int32 NumSections = LODResource.Sections.Num();

		for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
		{
			const FStaticMeshSection& StaticMeshSection = LODResource.Sections[SectionIndex];

			const int32 NumIndices = StaticMeshSection.NumTriangles * 3;
			for (int32 IndexIndex = 0; IndexIndex < NumIndices; IndexIndex++)
			{
				int32 Index = IndexArrayView[StaticMeshSection.FirstIndex + IndexIndex];
				RawMesh.WedgeIndices.Add(BaseVertexIndex + Index);
#if ENGINE_MAJOR_VERSION >= 5
				RawMesh.WedgeTangentX.Add(FVector4f(InComponentToWorld.TransformVector(FVector(StaticMeshVertexBuffer.VertexTangentX(Index)))));
				RawMesh.WedgeTangentY.Add(FVector4f(InComponentToWorld.TransformVector(FVector(StaticMeshVertexBuffer.VertexTangentY(Index)))));
				RawMesh.WedgeTangentZ.Add(FVector4f(InComponentToWorld.TransformVector(FVector(StaticMeshVertexBuffer.VertexTangentZ(Index)))));
#else
				RawMesh.WedgeTangentX.Add(InComponentToWorld.TransformVector(StaticMeshVertexBuffer.VertexTangentX(Index)));
				RawMesh.WedgeTangentY.Add(InComponentToWorld.TransformVector(StaticMeshVertexBuffer.VertexTangentY(Index)));
				RawMesh.WedgeTangentZ.Add(InComponentToWorld.TransformVector(StaticMeshVertexBuffer.VertexTangentZ(Index)));
#endif

				for (int32 TexCoordIndex = 0; TexCoordIndex < MAX_MESH_TEXTURE_COORDS; TexCoordIndex++)
				{
					if (TexCoordIndex >= NumTexCoords)
						RawMesh.WedgeTexCoords[TexCoordIndex].AddDefaulted();
					else
						RawMesh.WedgeTexCoords[TexCoordIndex].Add(StaticMeshVertexBuffer.GetVertexUV(Index, TexCoordIndex));
				}

				if (LODResource.VertexBuffers.ColorVertexBuffer.IsInitialized())
					RawMesh.WedgeColors.Add(LODResource.VertexBuffers.ColorVertexBuffer.VertexColor(Index));
				else
					RawMesh.WedgeColors.Add(FColor::White);
			}

			// copy face info
			for (uint32 TriIndex = 0; TriIndex < StaticMeshSection.NumTriangles; TriIndex++)
			{
				RawMesh.FaceMaterialIndices.Add(BaseMaterialIndex + StaticMeshSection.MaterialIndex);
				RawMesh.FaceSmoothingMasks.Add(0); // Assume this is ignored as bRecomputeNormals is false
			}
		}
	}
}

TArray<FVector> USkeletalToProceduralRuntime::Vectors3fToVectors(const TArray<FVector3f>& Source)
{
	TArray<FVector> Res;
	for (const auto& V : Source)
	{
		Res.Emplace(static_cast<FVector>(V));
	}
	return Res;
}

TArray<FVector2D> USkeletalToProceduralRuntime::Vectors2fToVectors2D(const TArray<FVector2f>& Source)
{
	if (Source.Num() == 0) return {};
	
	TArray<FVector2D> Res;
	for (const auto& V : Source)
	{
		Res.Emplace(static_cast<FVector2D>(V));
	}
	return Res;
}

bool USkeletalToProceduralRuntime::CreateProcMesh(const TArray<FRawMesh>& RawMeshes,
                                              UProceduralMeshComponent* ProcMeshComponent, const bool GenerateUV,
                                              const FMyUVMapParameters& Params)
{
	ProcMeshComponent->ClearAllMeshSections();
	
	TArray<int32> Tris;
	TArray<FProcMeshTangent> Tangents;
	TArray<FColor> VertexColors;
	TArray<FVector3f> TangZ;
	
	//Build procedural mesh
	for (const auto& RawMesh : RawMeshes)
	{
		for (int32 i = 0; i < RawMesh.WedgeIndices.Num(); ++i)
		{
			Tris.Emplace(RawMesh.WedgeIndices[i]);
			VertexColors.Emplace(0, 0, 0, 255);
		}

		for (int32 i = 0; i < RawMesh.VertexPositions.Num(); ++i)
		{
			Tangents.Emplace(static_cast<FVector>(RawMesh.WedgeTangentY[i]), false);
			TangZ.Add(RawMesh.WedgeTangentZ[i]);
		}
		
		TArray<FVector2f> UVs;
		if (GenerateUV)
		{
			//Experimental generate UV
			// Project along X-axis (left view), UV along Z Y axes
			FVector U = FVector::UpVector;
			FVector V = FVector::RightVector;

			const auto VertexPositions = RawMesh.VertexPositions;

			UVs.Reserve(RawMesh.VertexPositions.Num());

			FVector Size(Params.Size * Params.Scale);
			const FVector Offset = Params.Position - Size / 2.f;

			for (const auto& Ver : VertexPositions)
			{
				FVector Vertex(Ver);

				// Apply the gizmo transforms
				Vertex = Params.Rotation.RotateVector(Vertex);
				Vertex -= Offset;
				Vertex /= Size;

				float UCoord = FVector::DotProduct(Vertex, U) * Params.UVTile.X;
				float VCoord = FVector::DotProduct(Vertex, V) * Params.UVTile.Y;
				UVs.Add(FVector2f(UCoord, VCoord));
			}
			
			//
		}

		TArray<FVector2D> EmptyArray;
		ProcMeshComponent->CreateMeshSection(&RawMesh - &RawMeshes[0], Vectors3fToVectors(RawMesh.VertexPositions)
		                                     , Tris, Vectors3fToVectors(TangZ)
		                                     , Vectors2fToVectors2D(GenerateUV ? UVs : RawMesh.WedgeTexCoords[0]),
		                                     Vectors2fToVectors2D(RawMesh.WedgeTexCoords[1])
		                                     , Vectors2fToVectors2D(RawMesh.WedgeTexCoords[2]),
		                                     Vectors2fToVectors2D(RawMesh.WedgeTexCoords[3])
		                                     , VertexColors, Tangents,
		                                     true);
	}

	return true;
}
