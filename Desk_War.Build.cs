// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Desk_War : ModuleRules
{
	public Desk_War(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "DialoguePlugin",
            "EasyActionPlugin",
            "UMG",
            "PhysX",
            "APEX",
            "AIModule",
            "OnlineSubsystem",
            "﻿OnlineSubsystemSteam﻿",
            "VirtualCamera",
             "OnlineSubsystemUtils",
            "Steamworks"

        });

		PrivateDependencyModuleNames.AddRange(new string[] {
        "DialoguePlugin",
        "EasyActionPlugin",
        "AIModule",
            "NavigationSystem"
        });
        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
    }
}
