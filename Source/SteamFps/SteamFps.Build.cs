// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SteamFps : ModuleRules
{
	public SteamFps(TargetInfo Target)
	{
        PublicDependencyModuleNames.AddRange(new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "Slate",
                "SlateCore",
                "UMG",
                "HeadMountedDisplay"
            }
        );

    }
}
