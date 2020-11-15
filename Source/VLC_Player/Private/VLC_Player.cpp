// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "VLC_Player.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FVLC_PlayerModule"


FPDEFINEFUNC(libvlc_new)
FPDEFINEFUNC(libvlc_release)
FPDEFINEFUNC(libvlc_log_set)
FPDEFINEFUNC(libvlc_log_unset)
FPDEFINEFUNC(libvlc_audio_set_volume)
FPDEFINEFUNC(libvlc_media_new_path)
FPDEFINEFUNC(libvlc_media_new_location)
FPDEFINEFUNC(libvlc_errmsg)
FPDEFINEFUNC(libvlc_media_release)
FPDEFINEFUNC(libvlc_media_add_option)
FPDEFINEFUNC(libvlc_media_player_new_from_media)
FPDEFINEFUNC(libvlc_video_set_callbacks)
FPDEFINEFUNC(libvlc_video_set_format_callbacks)
FPDEFINEFUNC(libvlc_media_player_event_manager)
FPDEFINEFUNC(libvlc_media_player_play)
FPDEFINEFUNC(libvlc_media_player_stop)
FPDEFINEFUNC(libvlc_media_player_release)
FPDEFINEFUNC(libvlc_media_player_get_length)
FPDEFINEFUNC(libvlc_video_get_size)
FPDEFINEFUNC(libvlc_log_get_context)
FPDEFINEFUNC(libvlc_event_attach)
FPDEFINEFUNC(libvlc_event_detach)
FPDEFINEFUNC(libvlc_audio_set_callbacks)
FPDEFINEFUNC(libvlc_audio_set_track)


void FVLC_PlayerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString BaseDir = IPluginManager::Get().FindPlugin("VLC_Player")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString libvlcPath;
	FString libvlccorePath;

#if PLATFORM_WINDOWS
	libvlccorePath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/VLC_PlayerLibrary/x64/Release/libvlccore.dll"));
	libvlcPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/VLC_PlayerLibrary/x64/Release/libvlc.dll"));

	libvlccoreHandle = !libvlccorePath.IsEmpty() ? FPlatformProcess::GetDllHandle(*libvlccorePath) : nullptr;
	libvlcHandle = !libvlcPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*libvlcPath) : nullptr;

	FPINITFUNC(libvlc_new)
	FPINITFUNC(libvlc_release)
	FPINITFUNC(libvlc_log_set)
	FPINITFUNC(libvlc_log_unset)
	FPINITFUNC(libvlc_audio_set_volume)
	FPINITFUNC(libvlc_media_new_path)
	FPINITFUNC(libvlc_media_new_location)
	FPINITFUNC(libvlc_errmsg)
	FPINITFUNC(libvlc_media_release)
	FPINITFUNC(libvlc_media_add_option)
	FPINITFUNC(libvlc_media_player_new_from_media)
	FPINITFUNC(libvlc_video_set_callbacks)
	FPINITFUNC(libvlc_video_set_format_callbacks)
	FPINITFUNC(libvlc_media_player_event_manager)
	FPINITFUNC(libvlc_media_player_play)
	FPINITFUNC(libvlc_media_player_stop)
	FPINITFUNC(libvlc_media_player_release)
	FPINITFUNC(libvlc_media_player_get_length)
	FPINITFUNC(libvlc_video_get_size)
	FPINITFUNC(libvlc_log_get_context)
	FPINITFUNC(libvlc_event_attach)
	FPINITFUNC(libvlc_event_detach)
	FPINITFUNC(libvlc_audio_set_callbacks)
	FPINITFUNC(libvlc_audio_set_track)

	if (libvlcHandle)//libvlccoreHandle && 
	{
		FPIMPORTFUNC(libvlcHandle, libvlc_new)
		FPIMPORTFUNC(libvlcHandle, libvlc_release)
		FPIMPORTFUNC(libvlcHandle, libvlc_log_set)
		FPIMPORTFUNC(libvlcHandle, libvlc_log_unset)
		FPIMPORTFUNC(libvlcHandle, libvlc_audio_set_volume)
		FPIMPORTFUNC(libvlcHandle, libvlc_media_new_path)
		FPIMPORTFUNC(libvlcHandle, libvlc_media_new_location)
		FPIMPORTFUNC(libvlcHandle, libvlc_errmsg)
		FPIMPORTFUNC(libvlcHandle, libvlc_media_release)
		FPIMPORTFUNC(libvlcHandle, libvlc_media_add_option)
		FPIMPORTFUNC(libvlcHandle, libvlc_media_player_new_from_media)
		FPIMPORTFUNC(libvlcHandle, libvlc_video_set_callbacks)
		FPIMPORTFUNC(libvlcHandle, libvlc_video_set_format_callbacks)
		FPIMPORTFUNC(libvlcHandle, libvlc_media_player_event_manager)
		FPIMPORTFUNC(libvlcHandle, libvlc_media_player_play)
		FPIMPORTFUNC(libvlcHandle, libvlc_media_player_stop)
		FPIMPORTFUNC(libvlcHandle, libvlc_media_player_release)
		FPIMPORTFUNC(libvlcHandle, libvlc_media_player_get_length)
		FPIMPORTFUNC(libvlcHandle, libvlc_video_get_size)
		FPIMPORTFUNC(libvlcHandle, libvlc_log_get_context)
		FPIMPORTFUNC(libvlcHandle, libvlc_event_attach)
		FPIMPORTFUNC(libvlcHandle, libvlc_event_detach)
		FPIMPORTFUNC(libvlcHandle, libvlc_audio_set_callbacks)
		FPIMPORTFUNC(libvlcHandle, libvlc_audio_set_track)
	
		UE_LOG(LogTemp, Warning, TEXT("Load VLCLibrary Successed"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load VLCLibrary"));
	}


//#elif PLATFORM_MAC
//    LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/VLC_PlayerLibrary/Mac/Release/libExampleLibrary.dylib"));
#endif // PLATFORM_WINDOWS
}

void FVLC_PlayerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FPlatformProcess::FreeDllHandle(libvlccoreHandle);
	libvlccoreHandle = nullptr;
	FPlatformProcess::FreeDllHandle(libvlcHandle);
	libvlcHandle = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("Unload VLCLibrary Successed"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVLC_PlayerModule, VLC_Player)