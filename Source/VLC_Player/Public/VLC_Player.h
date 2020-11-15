// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "vlc/vlc.h" 


#define FPINITFUNC(func) \
			ProcFP_##func = nullptr;

#define FPEXTERNFUNC(func) \
			extern FP_##func ProcFP_##func;

//FFFMPEG_PlayerModule::
#define FPDEFINEFUNC(func) \
			FP_##func ProcFP_##func;

#define FPIMPORTFUNC(pDllHandler,func)	\
			ProcFP_##func = (FP_##func)FPlatformProcess::GetDllExport(pDllHandler, TEXT(#func)); \
			check(ProcFP_##func);

//FFFMPEG_PlayerModule::
#define FPGETFUNC(func) \
			ProcFP_##func

typedef libvlc_instance_t *(*FP_libvlc_new)(int argc, const char *const *argv);
typedef void (*FP_libvlc_release)(libvlc_instance_t *p_instance);
typedef void (*FP_libvlc_log_set)(libvlc_instance_t *p_instance,libvlc_log_cb cb, void *data);
typedef void (*FP_libvlc_log_unset)(libvlc_instance_t *p_instance);
typedef int (*FP_libvlc_audio_set_volume)(libvlc_media_player_t *p_mi, int i_volume);
typedef libvlc_media_t *(*FP_libvlc_media_new_path)(libvlc_instance_t *p_instance, const char *path);
typedef libvlc_media_t *(*FP_libvlc_media_new_location)(libvlc_instance_t *p_instance, const char * psz_mrl);
typedef const char *(*FP_libvlc_errmsg)(void);
typedef void (*FP_libvlc_media_release)(libvlc_media_t *p_md);
typedef void (*FP_libvlc_media_add_option)(libvlc_media_t *p_md, const char * psz_options);
typedef libvlc_media_player_t * (*FP_libvlc_media_player_new_from_media)(libvlc_media_t *p_md);
typedef void (*FP_libvlc_video_set_callbacks)(libvlc_media_player_t *mp,libvlc_video_lock_cb lock,libvlc_video_unlock_cb unlock,libvlc_video_display_cb display,void *opaque);
typedef void (*FP_libvlc_video_set_format_callbacks)(libvlc_media_player_t *mp,libvlc_video_format_cb setup,libvlc_video_cleanup_cb cleanup);
typedef libvlc_event_manager_t * (*FP_libvlc_media_player_event_manager)(libvlc_media_player_t *p_mi);
typedef int (*FP_libvlc_media_player_play)(libvlc_media_player_t *p_mi);
typedef void (*FP_libvlc_media_player_stop)(libvlc_media_player_t *p_mi);
typedef void (*FP_libvlc_media_player_release)(libvlc_media_player_t *p_mi);
typedef libvlc_time_t(*FP_libvlc_media_player_get_length)(libvlc_media_player_t *p_mi);
typedef int (*FP_libvlc_video_get_size)(libvlc_media_player_t *p_mi, unsigned num,unsigned *px, unsigned *py);
typedef void (*FP_libvlc_log_get_context)(const libvlc_log_t *ctx,const char **module, const char **file, unsigned *line);
typedef int (*FP_libvlc_event_attach)(libvlc_event_manager_t *p_event_manager,libvlc_event_type_t i_event_type,libvlc_callback_t f_callback,void *user_data);
typedef void (*FP_libvlc_event_detach)(libvlc_event_manager_t *p_event_manager,libvlc_event_type_t i_event_type,libvlc_callback_t f_callback,void *p_user_data);
typedef void (*FP_libvlc_audio_set_callbacks)(libvlc_media_player_t *mp,libvlc_audio_play_cb play,libvlc_audio_pause_cb pause,libvlc_audio_resume_cb resume,libvlc_audio_flush_cb flush,libvlc_audio_drain_cb drain,void *opaque);
typedef int(*FP_libvlc_audio_set_track)(libvlc_media_player_t *p_mi, int i_track);

//libvlc_media_player_pause
//libvlc_media_player_get_time
//libvlc_media_player_set_time

FPEXTERNFUNC(libvlc_new)
FPEXTERNFUNC(libvlc_release)
FPEXTERNFUNC(libvlc_log_set)
FPEXTERNFUNC(libvlc_log_unset)
FPEXTERNFUNC(libvlc_audio_set_volume)
FPEXTERNFUNC(libvlc_media_new_path)
FPEXTERNFUNC(libvlc_media_new_location)
FPEXTERNFUNC(libvlc_errmsg)
FPEXTERNFUNC(libvlc_media_release)
FPEXTERNFUNC(libvlc_media_add_option)
FPEXTERNFUNC(libvlc_media_player_new_from_media)
FPEXTERNFUNC(libvlc_video_set_callbacks)
FPEXTERNFUNC(libvlc_video_set_format_callbacks)
FPEXTERNFUNC(libvlc_media_player_event_manager)
FPEXTERNFUNC(libvlc_media_player_play)
FPEXTERNFUNC(libvlc_media_player_stop)
FPEXTERNFUNC(libvlc_media_player_release)
FPEXTERNFUNC(libvlc_media_player_get_length)
FPEXTERNFUNC(libvlc_video_get_size)
FPEXTERNFUNC(libvlc_log_get_context)
FPEXTERNFUNC(libvlc_event_attach)
FPEXTERNFUNC(libvlc_event_detach)
FPEXTERNFUNC(libvlc_audio_set_callbacks)
FPEXTERNFUNC(libvlc_audio_set_track)

class FVLC_PlayerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override; 

public:
	/** Handle to the test dll we will load */
	void*	libvlccoreHandle;
	void*	libvlcHandle;
};
