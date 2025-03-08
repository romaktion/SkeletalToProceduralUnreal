// Copyright Roman Kryvosheienko. All Rights Reserved.

#include "SkeletalToProceduralMorphTarget.h"
#include "SkeletalToProceduralRuntime.h"

void USkeletalToProceduralMorphTarget::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

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

void USkeletalToProceduralMorphTarget::SerializeMemoryArchive(FMemoryArchive& Ar)
{
	Super::SerializeMemoryArchive(Ar);
	
	Ar << SkeletalToProceduralMorphLODModels;
}

void USkeletalToProceduralMorphTarget::Init(UMorphTarget* MorphTarget)
{
	BaseSkelMesh = MorphTarget->BaseSkelMesh;
	for (const auto& LODModel : MorphTarget->GetMorphLODModels())
		MorphLODModels.Emplace(LODModel);
}

void USkeletalToProceduralMorphTarget::BackupDeltas()
{
	SkeletalToProceduralMorphLODModels.Empty();
	
	for (const auto& MorphLODModel : MorphLODModels)
	{
		FSkeletalToProceduralMorphTargetLODModel SkeletalToProceduralMorphTargetLODModel;
		for (const auto& [PositionDelta, TangentZDelta, SourceIdx] : MorphLODModel.Vertices)
			SkeletalToProceduralMorphTargetLODModel.Deltas.Emplace(PositionDelta, TangentZDelta, SourceIdx);
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
		
		check(Model.NumVertices == Deltas.Num());
		
		for (const auto& [PositionDelta, TangentZDelta, SourceIdx] : Deltas)
			Vertices.Emplace(PositionDelta, TangentZDelta, SourceIdx);
	}
}
