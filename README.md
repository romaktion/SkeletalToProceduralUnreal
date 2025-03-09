# Runtime Skeletal Mesh (with/out morphs) to Procedural Mesh Unreal Engine Plugin

<!-- Plugin description -->
Creates a procedural mesh from skeletal mesh at runtime  taking morphs into account.
<!-- Plugin description end -->

## Install

1. Download SkeletalToProcedural Unreal Plugin release zip for you Unreal Engine version https://github.com/romaktion/SkeletalToProceduralUnreal/releases/
2. Extract to [Engine directory]/Engine/Plugins/Marketplace folder

## How to use

You need to just call SkeletalToProcedural passing your SkeletalMeshComponent/SkeletalMeshActor and ProceduralMeshComponent/ProceduralActor
<img width="1049" alt="Screenshot 2025-03-10 at 01 54 48" src="https://github.com/user-attachments/assets/679b9e9d-5158-49e8-bd95-905cee85eb3b" />
or you can use advanced node with params for UV regeneration if needed
<img width="1051" alt="Screenshot 2025-03-10 at 01 57 32" src="https://github.com/user-attachments/assets/76fa6ee3-c544-4335-a5ff-cf8d999aa926" />

I made an example map with functions, which is located in the Сontent of the plugin

P.S. StaticToProcedural function is currently in experimental mode and does not work in the build.
