// Copyright 2024 romaktion@gmail.com. All Rights Reserved.

#include "SkeletalToProcedural.h"
#include "SkeletalToProceduralMorphTarget.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "UObject/ObjectSaveContext.h"
#endif

DEFINE_LOG_CATEGORY(LogSkeletalToProcedural);

IMPLEMENT_MODULE(FSkeletalToProceduralModule, SkeletalToProcedural)

void FSkeletalToProceduralModule::StartupModule()
{
	IModuleInterface::StartupModule();

#if ENGINE_MAJOR_VERSION >= 5
	FCoreUObjectDelegates::OnObjectPreSave.AddLambda([](UObject* Object, FObjectPreSaveContext)
#else
	FCoreUObjectDelegates::OnAssetLoaded.AddLambda([](UObject* Object)
#endif
	{
		if (const auto SkeletalMesh{Cast<USkeletalMesh>(Object)})
		{
			TArray<UMorphTarget*> NewMorphTargets;
			for (const auto MorphTarget : SkeletalMesh->GetMorphTargets())
			{
				const auto MorphName{MorphTarget->GetFName()};

				MorphTarget->Rename();

				const auto NewMorphTarget{NewObject<USkeletalToProceduralMorphTarget>(MorphTarget->GetOuter(), MorphName)};
				NewMorphTarget->Init(MorphTarget);

				NewMorphTargets.Add(NewMorphTarget);
			}

			SkeletalMesh->SetMorphTargets(NewMorphTargets);
		}
	});
}
