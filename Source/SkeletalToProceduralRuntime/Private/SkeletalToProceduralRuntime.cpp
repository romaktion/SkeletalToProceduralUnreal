// Copyright 2024 romaktion@gmail.com. All Rights Reserved.

#include "SkeletalToProceduralRuntime.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogSkeletalToProceduralRuntime);

IMPLEMENT_MODULE(FSkeletalToProceduralRuntimeModule, SkeletalToProceduralRuntime)

// Unique Framework Object version id
const FGuid FSkeletalToProceduralVersion::GUID(0xAB62897E, 0x04488BAD, 0xF47AF18A, 0xF1DA4670);
// Register Framework custom version with Core
FDevVersionRegistration GRegisterSkeletalToProceduralObjectVersion(FSkeletalToProceduralVersion::GUID, FSkeletalToProceduralVersion::LatestVersion, TEXT("Dev-SkeletalToProcedural"));
