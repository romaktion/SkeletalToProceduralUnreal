// Copyright Roman Kryvosheienko. All Rights Reserved.

#include "SkeletalToProceduralMorphTarget.h"
#include "EngineUtils.h"
#include "SkeletalToProceduralRuntime.h"
#include "Rendering/SkeletalMeshModel.h"

void USkeletalToProceduralMorphTarget::Serialize(FArchive& Ar)
{
#if ENGINE_MAJOR_VERSION >= 5
	UObject::Serialize( Ar );
#else
	LLM_SCOPE(ELLMTag::Animation);
	
	UObject::Serialize( Ar );
	Ar.UsingCustomVersion(FEditorObjectVersion::GUID);

	FStripDataFlags StripFlags( Ar );
	if( !StripFlags.IsDataStrippedForServer() )
	{
		Ar << MorphLODModels;
	}
#endif
	
	Ar.UsingCustomVersion(FSkeletalToProceduralVersion::GUID);
	
	if (!Ar.IsObjectReferenceCollector())
	{
		if (Ar.CustomVer(FSkeletalToProceduralVersion::GUID) >= FSkeletalToProceduralVersion::BeforeCustomVersionWasAdded)
		{
			if (Ar.IsLoading())
			{
				Ar << SkeletalToProceduralMorphLODModels;
				SyncDeltas();
			}
			else
			{
				BackupDeltas();
				Ar << SkeletalToProceduralMorphLODModels;
			}
		}
	}
}

#if ENGINE_MAJOR_VERSION < 5
void USkeletalToProceduralMorphTarget::PostLoad()
{
	UObject::PostLoad();
	
#if WITH_EDITOR
	if (GetLinkerCustomVersion(FEditorObjectVersion::GUID) < FEditorObjectVersion::AddedMorphTargetSectionIndices &&
		BaseSkelMesh)
	{
		const int32 MaxLOD = FMath::Min(BaseSkelMesh->GetImportedModel()->LODModels.Num(), MorphLODModels.Num());
		for (int32 LODIndex = 0; LODIndex < MaxLOD; ++LODIndex)
		{
			FMorphTargetLODModel& MorphLODModel = MorphLODModels[LODIndex];
			MorphLODModel.SectionIndices.Empty();
			const FSkeletalMeshLODModel& LODModel = BaseSkelMesh->GetImportedModel()->LODModels[LODIndex];
			TArray<int32> BaseIndexes;
			TArray<int32> LastIndexes;
			for (int32 SectionIdx = 0; SectionIdx < LODModel.Sections.Num(); ++SectionIdx)
			{
				const int32 BaseVertexBufferIndex = LODModel.Sections[SectionIdx].GetVertexBufferIndex();
				BaseIndexes.Add(BaseVertexBufferIndex);
				LastIndexes.Add(BaseVertexBufferIndex + LODModel.Sections[SectionIdx].GetNumVertices());
			}
			// brute force
			for (int32 VertIndex = 0; VertIndex < MorphLODModel.Vertices.Num() && MorphLODModel.SectionIndices.Num() < BaseIndexes.Num(); ++VertIndex)
			{
				int32 SourceVertexIdx = MorphLODModel.Vertices[VertIndex].SourceIdx;
				for (int32 SectionIdx = 0; SectionIdx < BaseIndexes.Num(); ++SectionIdx)
				{
					if (!MorphLODModel.SectionIndices.Contains(SectionIdx))
					{
						if (BaseIndexes[SectionIdx] <= SourceVertexIdx && SourceVertexIdx < LastIndexes[SectionIdx])
						{
							MorphLODModel.SectionIndices.AddUnique(SectionIdx);
							break;
						}
					}
				}
			}
		}
	}
#endif //#if WITH_EDITOR
}

void USkeletalToProceduralMorphTarget::GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize)
{
	UObject::GetResourceSizeEx(CumulativeResourceSize);

	for (const auto& LODModel : MorphLODModels)
	{
		CumulativeResourceSize.AddUnknownMemoryBytes(LODModel.Vertices.GetAllocatedSize() + sizeof(int32));
	}
}
#endif

#if ENGINE_MAJOR_VERSION >= 5
void USkeletalToProceduralMorphTarget::SerializeMemoryArchive(FMemoryArchive& Ar)
{
	Super::SerializeMemoryArchive(Ar);
	
	Ar << SkeletalToProceduralMorphLODModels;
}
#endif

void USkeletalToProceduralMorphTarget::Init(UMorphTarget* MorphTarget)
{
	BaseSkelMesh = MorphTarget->BaseSkelMesh;
#if ENGINE_MAJOR_VERSION >= 5
	for (const auto& LODModel : MorphTarget->GetMorphLODModels())
#endif
	for (const auto& LODModel : MorphTarget->MorphLODModels)
		MorphLODModels.Emplace(LODModel);
}

void USkeletalToProceduralMorphTarget::BackupDeltas()
{
	SkeletalToProceduralMorphLODModels.Empty();
	
	for (const auto& MorphLODModel : MorphLODModels)
	{
		FSkeletalToProceduralMorphTargetLODModel SkeletalToProceduralMorphTargetLODModel;
		for (const auto& Vertex : MorphLODModel.Vertices)
			SkeletalToProceduralMorphTargetLODModel.Deltas.Emplace(Vertex.PositionDelta, Vertex.TangentZDelta, Vertex.SourceIdx);
		SkeletalToProceduralMorphLODModels.Emplace(MoveTemp(SkeletalToProceduralMorphTargetLODModel));
	}
}

void USkeletalToProceduralMorphTarget::SyncDeltas()
{
	for (const auto& SkeletalToProceduralMorphLODModel : SkeletalToProceduralMorphLODModels)
	{
		auto& Model{MorphLODModels[&SkeletalToProceduralMorphLODModel - &SkeletalToProceduralMorphLODModels[0]]};
		const auto& Deltas{SkeletalToProceduralMorphLODModel.Deltas};
		auto& Vertices{Model.Vertices};
		Vertices.Empty();

#if ENGINE_MAJOR_VERSION >= 5
		check(Model.NumVertices == Deltas.Num());
#else
		check(Model.NumBaseMeshVerts == Deltas.Num());
#endif
		
		for (const auto& Delta : Deltas)
		{
			FMorphTargetDelta D;
			D.PositionDelta = Delta.PositionDelta;
			D.TangentZDelta = Delta.TangentZDelta;
			D.SourceIdx = Delta.SourceIdx;
			Vertices.Emplace(D);
		}
	}
}
