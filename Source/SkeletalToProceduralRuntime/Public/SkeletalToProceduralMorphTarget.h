// Copyright 2024 romaktion@gmail.com. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/MorphTarget.h"
#include "SkeletalToProceduralMorphTarget.generated.h"

USTRUCT()
struct FSkeletalToProceduralMorphTargetDelta
{
	GENERATED_BODY()

	FSkeletalToProceduralMorphTargetDelta() = default;
	FSkeletalToProceduralMorphTargetDelta(const FVector& PositionDelta, const FVector& TangentZDelta, uint32 SourceIdx) :
	PositionDelta(PositionDelta), TangentZDelta(TangentZDelta), SourceIdx(SourceIdx) {}
	
	/** change in position */
	UPROPERTY() FVector PositionDelta{};

	/** Tangent basis normal */
	UPROPERTY() FVector TangentZDelta{};

	/** index of source vertex to apply deltas to */
	UPROPERTY() uint32 SourceIdx{};

	/** pipe operator */
	friend FArchive& operator<<(FArchive& Ar, FSkeletalToProceduralMorphTargetDelta& V)
	{
#if ENGINE_MAJOR_VERSION >= 5
		if ((Ar.UEVer() < VER_UE4_MORPHTARGET_CPU_TANGENTZDELTA_FORMATCHANGE) && Ar.IsLoading())
#else
		if ((Ar.UE4Ver() < VER_UE4_MORPHTARGET_CPU_TANGENTZDELTA_FORMATCHANGE) && Ar.IsLoading())
#endif
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
#if ENGINE_MAJOR_VERSION < 5
	virtual void PostLoad() override;
	virtual void GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize) override;
#endif
#if ENGINE_MAJOR_VERSION >= 5
	virtual void SerializeMemoryArchive(FMemoryArchive& Ar) override;
#endif
	
	void Init(UMorphTarget* MorphTarget);
	
protected:
	void BackupDeltas();
	void SyncDeltas();
	
	UPROPERTY() TArray<FSkeletalToProceduralMorphTargetLODModel> SkeletalToProceduralMorphLODModels;
	
};
