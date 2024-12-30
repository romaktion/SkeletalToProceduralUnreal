// Copyright 2024 romaktion@gmail.com. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSkeletalToProcedural, Log, All);

class FSkeletalToProceduralModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
};
