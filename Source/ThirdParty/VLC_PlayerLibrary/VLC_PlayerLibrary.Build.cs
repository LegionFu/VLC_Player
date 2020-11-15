// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class VLC_PlayerLibrary : ModuleRules
{
	public VLC_PlayerLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			PublicLibraryPaths.Add(Path.Combine(ModuleDirectory, "x64", "Release"));
            //PublicAdditionalLibraries.Add("ExampleLibrary.lib");

            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "Release", "libvlc.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "Release", "libvlccore.lib"));
            //PublicAdditionalLibraries.Add("libvlc.lib");
            //PublicAdditionalLibraries.Add("libvlccore.lib");

            // Delay-load the DLL, so we can load it from the right place first
            //PublicDelayLoadDLLs.Add("ExampleLibrary.dll");
            //PublicDelayLoadDLLs.Add("libvlc.dll");
            //PublicDelayLoadDLLs.Add("libvlccore.dll");
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "x64", "Release", "libvlc.dll"));
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "x64", "Release", "libvlccore.dll"));


            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(ModuleDirectory, "x64", "Release", "libvlc.dll")));
            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(ModuleDirectory, "x64", "Release", "libvlccore.dll")));

            // add VLC plug-ins
            string PluginDirectory = Path.Combine(ModuleDirectory, "x64", "Release", "plugins");

            if (Directory.Exists(PluginDirectory))
            {
                foreach (string Plugin in Directory.EnumerateFiles(PluginDirectory, "*.*", SearchOption.AllDirectories))
                {
                    RuntimeDependencies.Add(Path.Combine(PluginDirectory, Plugin));
                }
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            //PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libExampleLibrary.dylib"));
        }
	}
}
