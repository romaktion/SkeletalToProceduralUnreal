// Copyright 2024 romaktion@gmail.com. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSkeletalToProceduralRuntime, Log, All);

struct FSkeletalToProceduralVersion
{
	enum Type
	{
		// Before any version changes were made
		BeforeCustomVersionWasAdded = 0,
		CustomVerticesDelta,
		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	// The GUID for this custom version number
	SKELETALTOPROCEDURALRUNTIME_API const static FGuid GUID;

private:
	FSkeletalToProceduralVersion() {}
};

class SKELETALTOPROCEDURALRUNTIME_API FSkeletalToProceduralRuntimeModule : public IModuleInterface{};
