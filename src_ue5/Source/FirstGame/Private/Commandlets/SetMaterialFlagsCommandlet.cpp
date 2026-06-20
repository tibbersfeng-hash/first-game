// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Commandlets/SetMaterialFlagsCommandlet.h"
#include "FirstGame.h"
#include "Materials/Material.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"

USetMaterialFlagsCommandlet::USetMaterialFlagsCommandlet()
{
	LogToConsole = true;
}

int32 USetMaterialFlagsCommandlet::Main(const FString& Params)
{
	UE_LOG(LogTemp, Log, TEXT("SetMaterialFlagsCommandlet: Starting..."));
	UE_LOG(LogTemp, Log, TEXT("  Raw params: %s"), *Params);

	// Parse params — expects material path(s)
	TArray<FString> Tokens;
	TArray<FString> Switches;
	UCommandlet::ParseCommandLine(*Params, Tokens, Switches);

	UE_LOG(LogTemp, Log, TEXT("  Tokens (%d):"), Tokens.Num());
	for (const FString& T : Tokens)
	{
		UE_LOG(LogTemp, Log, TEXT("    [%s]"), *T);
	}
	UE_LOG(LogTemp, Log, TEXT("  Switches (%d):"), Switches.Num());
	for (const FString& S : Switches)
	{
		UE_LOG(LogTemp, Log, TEXT("    [%s]"), *S);
	}

	// Use Tokens if available, otherwise try Switches
	TArray<FString> MaterialPaths;
	if (Tokens.Num() > 0)
	{
		MaterialPaths = Tokens;
	}
	else
	{
		// Fallback: use Switches (paths might be parsed as switches if they start with /)
		for (const FString& S : Switches)
		{
			if (S.StartsWith(TEXT("/Game/")) || S.StartsWith(TEXT("/Engine/")))
			{
				MaterialPaths.Add(S);
			}
		}
	}

	if (MaterialPaths.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Usage: -run=SetMaterialFlags /Game/Path/To/Material1 [/Game/Path/To/Material2 ...]"));
		return 1;
	}

	int32 FixedCount = 0;

	for (const FString& MaterialPath : MaterialPaths)
	{
		UE_LOG(LogTemp, Log, TEXT("Processing: %s"), *MaterialPath);

		// Load the material
		UMaterial* Material = LoadObject<UMaterial>(nullptr, *MaterialPath);
		if (!Material)
		{
			UE_LOG(LogTemp, Warning, TEXT("  Failed to load: %s"), *MaterialPath);
			continue;
		}

		UE_LOG(LogTemp, Log, TEXT("  Loaded: %s"), *Material->GetName());
		UE_LOG(LogTemp, Log, TEXT("  Current bUsedWithSkeletalMesh: %s"),
			Material->bUsedWithSkeletalMesh ? TEXT("true") : TEXT("false"));

		if (!Material->bUsedWithSkeletalMesh)
		{
			Material->bUsedWithSkeletalMesh = true;
			Material->bUsedWithStaticLighting = false;

			// Save the asset using Editor API
			Material->MarkPackageDirty();
			Material->PostEditChange();

			UE_LOG(LogTemp, Log, TEXT("  Set bUsedWithSkeletalMesh = true"));
			FixedCount++;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("  Already has bUsedWithSkeletalMesh = true"));
		}
	}

	// Save all dirty packages
	if (FixedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Saving dirty packages..."));
		for ( TObjectIterator<UPackage> It; It; ++It )
		{
			UPackage* Pkg = *It;
			if ( Pkg->IsDirty() && Pkg->GetOutermost() == Pkg )
			{
				FSavePackageArgs SaveArgs;
				SaveArgs.TopLevelFlags = RF_Standalone | RF_Public;
				SaveArgs.Error = GError;
				UPackage::SavePackage(Pkg, nullptr, *Pkg->GetPathName(), SaveArgs);
				UE_LOG(LogTemp, Log, TEXT("  Saved: %s"), *Pkg->GetPathName());
			}
		}
		UE_LOG(LogTemp, Log, TEXT("Packages saved."));
	}

	UE_LOG(LogTemp, Log, TEXT("SetMaterialFlagsCommandlet: Fixed %d material(s)"), FixedCount);
	return 0;
}
