// Copyright 2024 romaktion@gmail.com. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/MorphTarget.h"
#include "SkeletalToProceduralMorphTarget.generated.h"

USTRUCT()
struct FSkeletalToProceduralMorphTargetDelta
{
	GENERATED_BODY()
	/** change in position */
	UPROPERTY() FVector3f			PositionDelta{};

	/** Tangent basis normal */
	UPROPERTY() FVector3f			TangentZDelta{};

	/** index of source vertex to apply deltas to */
	UPROPERTY() uint32			SourceIdx{};

	/** pipe operator */
	friend FArchive& operator<<(FArchive& Ar, FSkeletalToProceduralMorphTargetDelta& V)
	{
		if ((Ar.UEVer() < VER_UE4_MORPHTARGET_CPU_TANGENTZDELTA_FORMATCHANGE) && Ar.IsLoading())
		{
			/** old format of change in tangent basis normal */
			FDeprecatedSerializedPackedNormal TangentZDelta_DEPRECATED;
			Ar << V.PositionDelta << TangentZDelta_DEPRECATED << V.SourceIdx;
			V.TangentZDelta = TangentZDelta_DEPRECATED;
		}
		else
		{
			Ar << V.PositionDelta << V.TangentZDelta << V.SourceIdx;
		}
		return Ar;
	}
};

USTRUCT()
struct FSkeletalToProceduralMorphTargetLODModel
{
	GENERATED_BODY()
public:
	UPROPERTY() TArray<FSkeletalToProceduralMorphTargetDelta> Deltas;

	friend FArchive& operator<<(FArchive& Ar, FSkeletalToProceduralMorphTargetLODModel& V)
	{
		Ar << V.Deltas;

		return Ar;
	}
};

/**
 * 
 */
UCLASS()
class SKELETALTOPROCEDURALRUNTIME_API USkeletalToProceduralMorphTarget : public UMorphTarget
{
	GENERATED_BODY()
public:
	virtual void Serialize(FArchive& Ar) override;
	virtual void SerializeMemoryArchive(FMemoryArchive& Ar) override;
	
	void Init(UMorphTarget* MorphTarget);
	
protected:
	void BackupDeltas();
	void SyncDeltas();
	
	UPROPERTY() TArray<FSkeletalToProceduralMorphTargetLODModel> SkeletalToProceduralMorphLODModels;
	
};
