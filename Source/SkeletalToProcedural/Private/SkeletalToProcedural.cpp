// Copyright 2024 romaktion@gmail.com. All Rights Reserved.

#include "SkeletalToProcedural.h"
#include "SkeletalToProceduralMorphTarget.h"
#include "UObject/ObjectSaveContext.h"

DEFINE_LOG_CATEGORY(LogSkeletalToProcedural);

IMPLEMENT_MODULE(FSkeletalToProceduralModule, SkeletalToProcedural)

void FSkeletalToProceduralModule::StartupModule()
{
	IModuleInterface::StartupModule();

	FCoreUObjectDelegates::OnObjectPreSave.AddLambda([](UObject* Object, FObjectPreSaveContext)
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
