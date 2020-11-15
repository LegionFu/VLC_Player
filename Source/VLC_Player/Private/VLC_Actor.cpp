// Fill out your copyright notice in the Description page of Project Settings.

#include "VLC_Actor.h"
#include "Core.h"
#include "Engine.h"
#include <stdio.h>
#include <stdarg.h>
#include "VLC_Player.h"

//#define _CRT_SECURE_NO_WARNINGS

// Sets default values
AVLC_Actor::AVLC_Actor() :
	global_init(false)
	, inst(nullptr)
	, media_player(nullptr)
	, media(nullptr)
	, event_manager(nullptr)
	, in_buffer(nullptr)
	, CreateTexture(true)
	, GotPicture(false)
	, PictureReady(false)
	, PlaySuccessed(false)
	, bLocalFile(false)
	, auto_play(false)
	, enable_hw_acceleration(true)
	, update_texture_method(EVLCPlayerUpdateTextureMethod::RHICommand)
	, video_URL(FString())
	, enable_custom_resolution(false)
	, custom_resolution(FVector2D(0, 0))
	, output_resolution(FVector2D(0, 0))
	, video_resolution(FVector2D(0, 0))
	, MaterialParameterName("texture")
	, enable_audio(false)
	, audio_volume(0)
	, loop(false)
	, VideoTimeDelay(0)
	, output_log(false)
	, texture(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetMaterial(0, parent_material);

	PlayerState = EVLCPlayerActorState::Stopped;
	MediaState = EVLCMediaState::Stopped;
}

AVLC_Actor::~AVLC_Actor()
{
	//UE_LOG(LogTemp, Warning, TEXT("VLC  video destruction start"));

	VLC_StopVideo();
	VLC_Uninit();

	//UE_LOG(LogTemp, Warning, TEXT("VLC  video destruction end"));
}

// Called when the game starts or when spawned
void AVLC_Actor::BeginPlay()
{
	Super::BeginPlay();

	if (parent_material && video_material == NULL)
	{
		video_material = UMaterialInstanceDynamic::Create(parent_material, this);
		MeshComponent->SetMaterial(0, video_material);
	}

	VLC_Init();

	if (auto_play)
	{
		VLC_PlayVideo(video_URL);
	}
}

// Called every frame
void AVLC_Actor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GotPicture && PlayerState == EVLCPlayerActorState::Playing)
	{
		//UE_LOG(LogTemp, Warning, TEXT("VLC  video tick"));

		switch (update_texture_method)
		{
		case EVLCPlayerUpdateTextureMethod::Memcpy:
			texture->UpdateResource();
			video_material->SetTextureParameterValue(MaterialParameterName, texture);
			break;
		case EVLCPlayerUpdateTextureMethod::RHICommand:
			break;
		default:
			break;
		}

		GotPicture = false;
	}
}

bool AVLC_Actor::VLC_Init()
{
	if (global_init)
		return true;

	UE_LOG(LogTemp, Warning, TEXT("VLC  init"));
	inst = FPGETFUNC(libvlc_new)(0, NULL);
	
	if(output_log == true)
		FPGETFUNC(libvlc_log_set)(inst, video_log_callback, this);

	global_init = true;
	return true;
}

bool AVLC_Actor::VLC_Uninit()
{
	if (!global_init)
		return true;

	UE_LOG(LogTemp, Warning, TEXT("VLC  uninit"));
	
	FPGETFUNC(libvlc_log_unset)(inst);

	FPGETFUNC(libvlc_release)(inst);

	global_init = false;
	return true;
}

bool AVLC_Actor::VLC_PlayVideo(FString videoURL)
{
	return PlayVideo(videoURL);
}

bool AVLC_Actor::VLC_StopVideo()
{
	StopVideo();
	//UE_LOG(LogTemp, Warning, TEXT("VLC  video stop"));
	return true;
}

bool AVLC_Actor::VLC_SetAudioVolume(int volume)
{

	volume = volume > 0 ? volume : 0;
	volume = volume < 100 ? volume : 100;

	audio_volume = volume;
	if (media_player)
	{
		FPGETFUNC(libvlc_audio_set_volume)(media_player, audio_volume);
	}

	return true;
}

int AVLC_Actor::VLC_GetAudioVolume()
{
	return audio_volume;
}

bool AVLC_Actor::PlayVideo(FString videoURL)
{
	if (PlayerState != EVLCPlayerActorState::Stopped)
		return false;

	if (videoURL != "")
	{
		video_URL = videoURL;
	}

	if (video_URL == "")
		return false;

	GotPicture = false;
	PictureReady = false;
	//------------------------------------------------------------------------------------处理视频路径 判断是视频链接还是视频文件路径 如果是相对路径就填充成绝对路径
	int index = -1;
	bool found = video_URL.FindChar(TCHAR(':'), index); //是否是网络视频链接
	if (found && index > 1) // 如果包含:字符 并且在第二个字母之后 就是网络视频链接
	{
		bLocalFile = false;
	}
	else
	{
		bLocalFile = true;
	}

	while (bLocalFile)
	{
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*video_URL))
		{
			break; //URL是本地文件的绝对路径
		}
		
		FString str1 = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		str1 = str1 + video_URL;

		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*str1))
		{
			video_URL = str1; //URL是本地文件的相对路径
			break;
		}

		break;
	}

	if (bLocalFile)
	{
		video_URL = video_URL.Replace(UTF8_TO_TCHAR("//"), UTF8_TO_TCHAR("\\"));
		video_URL = video_URL.Replace(UTF8_TO_TCHAR("/"), UTF8_TO_TCHAR("\\"));
	}

	UE_LOG(LogTemp, Warning, TEXT("VLC  video URL = %s"), *video_URL);
	//------------------------------------------------------------------------------------处理视频路径

	if (bLocalFile)
		media = FPGETFUNC(libvlc_media_new_path)(inst, TCHAR_TO_UTF8(*video_URL));
	else
		media = FPGETFUNC(libvlc_media_new_location)(inst, TCHAR_TO_UTF8(*video_URL));

	if (!media)
	{
		const char * errorMSG = FPGETFUNC(libvlc_errmsg)();
		UE_LOG(LogTemp, Warning, TEXT("VLC  create media failed. %s"), ANSI_TO_TCHAR(errorMSG));

		FPGETFUNC(libvlc_media_release)(media);
		return false;
	}

	//------------------------------------------------------------------------------------其他设置
	//网络延迟时间可以根据需要修改，这个值越小，实时性越好，当然太小了播放会很卡，或者设置的值失效，一般建议在100-300之间
	FPGETFUNC(libvlc_media_add_option)(media, ":network-caching=100");

	if(!enable_audio)
		FPGETFUNC(libvlc_media_add_option)(media, ":no-audio");//禁用音频

	if(enable_hw_acceleration)
		FPGETFUNC(libvlc_media_add_option)(media, ":avcodec-hw=d3d11va");//:avcodec-hw=dxva2
	//------------------------------------------------------------------------------------其他设置

	/* Create a media player playing environment */
	media_player = FPGETFUNC(libvlc_media_player_new_from_media)(media);

	/* No need to keep the media now */
	FPGETFUNC(libvlc_media_release)(media);

	VLC_SetAudioVolume(audio_volume);

	//------------------------------------------------------------------------------------设置数据回调接口
	/*##comment the followint 2 lines , if you want the out frame display in screen*/
	FPGETFUNC(libvlc_video_set_callbacks)(media_player, lock, unlock, display, this);
	//FPGETFUNC(libvlc_video_set_format)(media_player, "RGBA", 1920, 1080, 1920 * 4);
	FPGETFUNC(libvlc_video_set_format_callbacks)(media_player, video_format_callback, video_cleanup_callback);

	//FPGETFUNC(libvlc_audio_set_track)(media_player, -1);
	//------------------------------------------------------------------------------------设置数据回调接口


	//------------------------------------------------------------------------------------设置回调事件接口
	event_manager = FPGETFUNC(libvlc_media_player_event_manager)(media_player);//不需要显式调用libvlc_event_manager_release，libvlc_media_player_release()释放媒体播放器时就会调用该方法释放媒体播放器事件管理器

	register_event_notification();//注册播放器的状态回调事件
	//------------------------------------------------------------------------------------设置回调事件接口

	// play the media_player
	int ret = FPGETFUNC(libvlc_media_player_play)(media_player);
	if (ret != 0)
	{
		const char * errorMSG = FPGETFUNC(libvlc_errmsg)();
		if (errorMSG)
			UE_LOG(LogTemp, Warning, TEXT("VLC  video play failed. %s"), errorMSG);
		PlayerState = EVLCPlayerActorState::Stopped;
		return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("VLC  video start play"));

	startTime = UKismetMathLibrary::Now();
	endTime = UKismetMathLibrary::Now();

	PlayerState = EVLCPlayerActorState::Playing;

	return true;
}

bool AVLC_Actor::StopVideo()  //手动调用停止视频
{
	if (PlayerState == EVLCPlayerActorState::Stopped)
		return true;

	PlayerState = EVLCPlayerActorState::Stopped;

	if (media_player)
	{
		unregister_event_notification();
		FPGETFUNC(libvlc_video_set_callbacks)(media_player, 0, 0, 0, 0);
		// Stop playing  
		FPGETFUNC(libvlc_media_player_stop)(media_player);

		// Free the media_player  
		FPGETFUNC(libvlc_media_player_release)(media_player);

		media_player = nullptr;
	}

	PictureReady = false;
	CreateTexture = true;
	output_resolution = FVector2D(0, 0);
	video_resolution = FVector2D(0, 0);
	VideoTimeDelay = 0;

	if (in_buffer)
	{
		FMemory::Free(in_buffer);
		in_buffer = nullptr;
	}

	//FPlatformProcess::Sleep(1);

	return true;
}

bool AVLC_Actor::StopVideoCallback() //视频播放完成后自动调用
{
	if (PlayerState == EVLCPlayerActorState::Stopped)
		return true;

	UE_LOG(LogTemp, Warning, TEXT("VLC  video StopVideoCallback"));
	PlayerState = EVLCPlayerActorState::Stopped;

	PictureReady = false;
	CreateTexture = true;
	
	if (media_player)
	{
		unregister_event_notification();
		FPGETFUNC(libvlc_video_set_callbacks)(media_player, 0, 0, 0, 0);
	}

	AsyncTask(ENamedThreads::GameThread, [&]() {
		if (media_player)
		{
			FPGETFUNC(libvlc_media_player_stop)(media_player);

			FPGETFUNC(libvlc_media_player_release)(media_player);

			media_player = nullptr;
		}

		output_resolution = FVector2D(0, 0);
		video_resolution = FVector2D(0, 0);
		VideoTimeDelay = 0;

		if (in_buffer)
		{
			FMemory::Free(in_buffer);
			in_buffer = nullptr;
		}
	
		if (loop)
		{
			if (bLocalFile)
			{
				if (PlaySuccessed)//本地文件 播放失败的话不再继续尝试
				{
					PlayVideo(); 
				}
			}
			else
			{
				PlayVideo(); // 视频流自动重连
			}
		}
	});
	return false;
}

void * AVLC_Actor::lock(void * opaque, void ** p_pixels)
{
	AVLC_Actor* ThisActor = (AVLC_Actor*)opaque;
	{
		ThisActor->m_mutex.Lock();
		if (ThisActor->in_buffer == nullptr)
		{
			//------------------------------------------------------VideoTimeDelay
			ThisActor->endTime = UKismetMathLibrary::Now();
			auto span = ThisActor->endTime - ThisActor->startTime;
			ThisActor->VideoTimeDelay = span.GetTotalSeconds();

			UE_LOG(LogTemp, Warning, TEXT("VLC  VideoTimeDelay = %f"), ThisActor->VideoTimeDelay);
			//------------------------------------------------------

			libvlc_time_t length = FPGETFUNC(libvlc_media_player_get_length)(ThisActor->media_player);
			UE_LOG(LogTemp, Warning, TEXT("VLC  video length = %d"), length);

			unsigned int X, Y;
			FPGETFUNC(libvlc_video_get_size)(ThisActor->media_player, 0, &X, &Y);
			ThisActor->video_resolution.X = X;//视频原始分辨率
			ThisActor->video_resolution.Y = Y;
			UE_LOG(LogTemp, Warning, TEXT("VLC  video origin resolution = %f,%f"), ThisActor->video_resolution.X, ThisActor->video_resolution.Y);
			UE_LOG(LogTemp, Warning, TEXT("VLC  video output resolution = %f,%f"), ThisActor->output_resolution.X, ThisActor->output_resolution.Y);
			
			int width = ThisActor->output_resolution.X;
			int height = ThisActor->output_resolution.Y;

			if (height != 0 && width != 0)
			{
				ThisActor->in_buffer = (char *)FMemory::Malloc(width * height * 4);
				
				AsyncTask(ENamedThreads::GameThread, [=]() {

					if (ThisActor->CreateTexture)
					{
						ThisActor->CreateTexture = false;
						ThisActor->texture = UTexture2D::CreateTransient(width, height, PF_B8G8R8A8);
						void* TextureData = ThisActor->texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
						FMemory::Memset(TextureData, 0, width * height * 4);
						ThisActor->texture->PlatformData->Mips[0].BulkData.Unlock();
						
						ThisActor->texture->UpdateResource();
						ThisActor->video_material->SetTextureParameterValue(ThisActor->MaterialParameterName, ThisActor->texture);
						ThisActor->PictureReady = true;
					}
				});
			}
		}

		if (ThisActor->in_buffer != nullptr)
		{
			*p_pixels = ThisActor->in_buffer;  /*tell VLC to put decoded data to this buffer*/
		}

		ThisActor->m_mutex.Unlock();
	}
	return 0;
}

void AVLC_Actor::unlock(void * opaque, void * id, void * const * p_pixels)
{
	AVLC_Actor* ThisActor = (AVLC_Actor*)opaque;

	{
		ThisActor->m_mutex.Lock();
		UE_LOG(LogTemp, Warning, TEXT("VLC  video unlock!"));
		unsigned int width = ThisActor->output_resolution.X;
		unsigned int height = ThisActor->output_resolution.Y;
		void* TextureData = nullptr;

		if (
			ThisActor->PictureReady
			&& !ThisActor->GotPicture
			&& ThisActor->texture->IsValidLowLevel() 
			&& *p_pixels != nullptr
			&& ThisActor->PlayerState == EVLCPlayerActorState::Playing
			&& ThisActor->texture->PlatformData != nullptr)
			//&& !ThisActor->texture->PlatformData->Mips[0].BulkData.IsLocked())
		{
			switch (ThisActor->update_texture_method)
			{
			case EVLCPlayerUpdateTextureMethod::Memcpy:

				TextureData = (ThisActor->texture)->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(TextureData, *p_pixels, width * height * 4);
				(ThisActor->texture)->PlatformData->Mips[0].BulkData.Unlock();
				ThisActor->GotPicture = true;
				break;

			case EVLCPlayerUpdateTextureMethod::RHICommand:

				ThisActor->region.SrcX = 0;
				ThisActor->region.SrcY = 0;
				ThisActor->region.DestX = 0;
				ThisActor->region.DestY = 0;
				ThisActor->region.Width = width;
				ThisActor->region.Height = height;
				(ThisActor->texture)->UpdateTextureRegions(0, 1, &ThisActor->region, ThisActor->region.Width * 4, 4, (uint8*)*p_pixels);

				ThisActor->GotPicture = true;
				break;

			default:
				break;
			}

		}

		ThisActor->m_mutex.Unlock();
	}

}

void AVLC_Actor::display(void * opaque, void * id)
{
}

unsigned AVLC_Actor::video_format_callback(void ** opaque, char * chroma, unsigned * width, unsigned * height, unsigned * pitches, unsigned * lines)
{
	AVLC_Actor* ThisActor = *(AVLC_Actor**)opaque;
	{
		ThisActor->m_mutex.Lock();
		UE_LOG(LogTemp, Warning, TEXT("VLC  video format callback"));
		FMemory::Memcpy(chroma, "BGRA", 4);

		if (ThisActor->enable_custom_resolution)
		{
			unsigned W = ThisActor->custom_resolution.X;
			unsigned H = ThisActor->custom_resolution.Y;

			if (W % 32 != 0 || H % 32 != 0) //保证pitch和line是32的倍数
			{
				unsigned temp = 0;
				temp = W % 32;
				temp = temp == 0 ? 0 : 32;
				W = W / 32 * 32;
				W += temp;

				/*temp = H % 32;
				temp = temp == 0 ? 0 : 32;
				H = H / 32 * 32;
				H += temp;*/
			}

			*width = W;
			*height = H;
		}

		pitches[0] = *width * 4;
		lines[0] = *height;

		ThisActor->output_resolution.X = *width;
		ThisActor->output_resolution.Y = *height;

		ThisActor->m_mutex.Unlock();
	}
	return 1;
}

void AVLC_Actor::video_cleanup_callback(void * opaque)
{
}

void AVLC_Actor::video_log_callback(void * data, int level, const libvlc_log_t * ctx, const char * fmt, va_list args)
{
	//char buffer[255] = "";
	//memset(buffer, 0, 255);
	//vsprintf_s(buffer, fmt, args);
	//UE_LOG(LogTemp, Warning, TEXT("VLC  log output:%s"), UTF8_TO_TCHAR(buffer));
	FString LogContext;

	// get context information
	if (ctx != nullptr)
	{
		const char* Module = nullptr;
		const char* File = nullptr;
		unsigned Line = 0;

		FPGETFUNC(libvlc_log_get_context)(ctx, &Module, &File, &Line);
		LogContext = FString::Printf(TEXT("%s: "), (Module != nullptr) ? ANSI_TO_TCHAR(Module) : TEXT("unknown module"));

		LogContext += FString::Printf(TEXT("%s, line %s: "),
			(File != nullptr) ? ANSI_TO_TCHAR(File) : TEXT("unknown file"),
			(Line != 0) ? *FString::Printf(TEXT("%i"), Line) : TEXT("n/a")
		);
	}
	else
	{
		LogContext = TEXT("generic: ");
	}
	
	LogContext = LogContext+"VLC log output ";

	// forward message to log
	ANSICHAR Message[1024];

	FCStringAnsi::GetVarArgs(Message, UE_ARRAY_COUNT(Message), fmt, args);

	switch (level)
	{
	case libvlc_log_level::LIBVLC_DEBUG:
		UE_LOG(LogTemp, VeryVerbose, TEXT("%s%s"), *LogContext, ANSI_TO_TCHAR(Message));
		break;

	case libvlc_log_level::LIBVLC_ERROR:
		UE_LOG(LogTemp, Error, TEXT("%s%s"), *LogContext, ANSI_TO_TCHAR(Message));
		break;

	case libvlc_log_level::LIBVLC_NOTICE:
		UE_LOG(LogTemp, Verbose, TEXT("%s%s"), *LogContext, ANSI_TO_TCHAR(Message));
		break;

	case libvlc_log_level::LIBVLC_WARNING:
		UE_LOG(LogTemp, Warning, TEXT("%s%s"), *LogContext, ANSI_TO_TCHAR(Message));
		break;

	default:
		UE_LOG(LogTemp, Log, TEXT("%s%s"), *LogContext, ANSI_TO_TCHAR(Message));
		break;
	}
}

void AVLC_Actor::video_event_callback(const libvlc_event_t * p_event, void * p_data)
{
	AVLC_Actor* ThisActor = (AVLC_Actor*)p_data;
	switch (p_event->type)
	{
	case libvlc_MediaPlayerOpening:
		ThisActor->MediaState = EVLCMediaState::Opening;
		ThisActor->PlaySuccessed = false;
		UE_LOG(LogTemp, Warning, TEXT("VLC event MediaPlayerOpening"));
		break;

	case libvlc_MediaPlayerBuffering:
		ThisActor->MediaState = EVLCMediaState::Buffering;
		UE_LOG(LogTemp, Warning, TEXT("VLC event MediaPlayerBuffering"));
		break;

	case libvlc_MediaPlayerPlaying:
		ThisActor->MediaState = EVLCMediaState::Playing;
		ThisActor->PlaySuccessed = true;
		UE_LOG(LogTemp, Warning, TEXT("VLC event MediaPlayerPlaying"));
		break;

	case libvlc_MediaPlayerPaused:
		ThisActor->MediaState = EVLCMediaState::Paused;
		UE_LOG(LogTemp, Warning, TEXT("VLC event MediaPlayerPaused"));
		break;

	case libvlc_MediaPlayerStopped:
		ThisActor->MediaState = EVLCMediaState::Stopped;
		UE_LOG(LogTemp, Warning, TEXT("VLC event MediaPlayerStopped"));
		if (ThisActor->loop && ThisActor->PlayerState != EVLCPlayerActorState::Stopped)
		{
			ThisActor->StopVideoCallback();
		}
		break;

	default:
		break;
	}
}

void AVLC_Actor::register_event_notification()
{
	FPGETFUNC(libvlc_event_attach)(event_manager, libvlc_MediaPlayerOpening, video_event_callback, this);
	FPGETFUNC(libvlc_event_attach)(event_manager, libvlc_MediaPlayerBuffering, video_event_callback, this);
	FPGETFUNC(libvlc_event_attach)(event_manager, libvlc_MediaPlayerPlaying, video_event_callback, this);
	FPGETFUNC(libvlc_event_attach)(event_manager, libvlc_MediaPlayerPaused, video_event_callback, this);
	FPGETFUNC(libvlc_event_attach)(event_manager, libvlc_MediaPlayerStopped, video_event_callback, this);
}

void AVLC_Actor::unregister_event_notification()
{
	FPGETFUNC(libvlc_event_detach)(event_manager, libvlc_MediaPlayerOpening, video_event_callback, this);
	FPGETFUNC(libvlc_event_detach)(event_manager, libvlc_MediaPlayerBuffering, video_event_callback, this);
	FPGETFUNC(libvlc_event_detach)(event_manager, libvlc_MediaPlayerPlaying, video_event_callback, this);
	FPGETFUNC(libvlc_event_detach)(event_manager, libvlc_MediaPlayerPaused, video_event_callback, this);
	FPGETFUNC(libvlc_event_detach)(event_manager, libvlc_MediaPlayerStopped, video_event_callback, this);
}
