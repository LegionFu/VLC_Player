// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "VLC_Player.h"
#include "VLC_Actor.generated.h"

UENUM(BlueprintType)
enum class EVLCPlayerUpdateTextureMethod : uint8
{
	Memcpy UMETA(DisplayName = "Memcpy"),
	RHICommand UMETA(DisplayName = "RHICommand")
};

UENUM(BlueprintType)
enum class EVLCPlayerActorState : uint8
{
	Playing UMETA(DisplayName = "Playing"),
	Paused UMETA(DisplayName = "Paused"),
	Stopped UMETA(DisplayName = "Stopped")
};

UENUM(BlueprintType)
enum class EVLCMediaState : uint8
{
	Opening UMETA(DisplayName = "Opening"),
	Buffering UMETA(DisplayName = "Buffering"),
	Playing UMETA(DisplayName = "Playing"),
	Paused UMETA(DisplayName = "Paused"),
	Stopped UMETA(DisplayName = "Stopped")
};

UCLASS()
class VLC_PLAYER_API AVLC_Actor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVLC_Actor();
	~AVLC_Actor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	FCriticalSection m_mutex;

	bool global_init;
	libvlc_instance_t * inst;
	libvlc_media_player_t *media_player;
	libvlc_media_t *media;
	libvlc_event_manager_t *event_manager;
	//libvlc_time_t media_length;

	char* in_buffer;
	bool CreateTexture;
	bool GotPicture;
	bool PictureReady;
	bool PlaySuccessed;
	bool bLocalFile;
	FDateTime startTime;
	FDateTime endTime;
	FUpdateTextureRegion2D region;

	//FVector2D output_plane_resolution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "自动播放视频", AllowPrivateAccess = "true", ExposeOnSpawn = true))
		bool auto_play;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "硬件解码", AllowPrivateAccess = "true", ExposeOnSpawn = true))
		bool enable_hw_acceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "texture更新方法", AllowPrivateAccess = "true", ExposeOnSpawn = true))
		EVLCPlayerUpdateTextureMethod update_texture_method;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "视频地址", AllowPrivateAccess = "true", ExposeOnSpawn = true))
		FString video_URL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "是否使用自定义分辨率", AllowPrivateAccess = "true", ExposeOnSpawn = true))
		bool enable_custom_resolution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "自定义的视频分辨率", AllowPrivateAccess = "true", ExposeOnSpawn = true))
		FVector2D custom_resolution;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (Tooltip = "输出的视频分辨率", AllowPrivateAccess = "true"))
		FVector2D output_resolution;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (Tooltip = "当前视频的原始分辨率", AllowPrivateAccess = "true"))
		FVector2D video_resolution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "母材质", AllowPrivateAccess = "true", ExposeOnSpawn = true))
		UMaterialInterface* parent_material;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (Tooltip = "视频播放材质实例(自动生成)", AllowPrivateAccess = "true"))
		UMaterialInstanceDynamic* video_material;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (Tooltip = "材质参数名", AllowPrivateAccess = "true"))
		FName MaterialParameterName;

	UPROPERTY(EditInstanceOnly, meta = (Tooltip = "播放音频", AllowPrivateAccess = "true"))
		bool enable_audio;

	UPROPERTY(EditInstanceOnly, meta = (Tooltip = "音量(0-100)", AllowPrivateAccess = "true", UIMin = "0", UIMax = "100", ClampMin = "0", ClampMax = "100"))
		int audio_volume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "循环播放(视频流自动重连)", AllowPrivateAccess = "true"))
		bool loop;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (Tooltip = "视频延时", AllowPrivateAccess = "true"))
		float VideoTimeDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "输出日志", AllowPrivateAccess = "true"))
		bool output_log;

public:

	char logbuffer[255];

	UPROPERTY(BlueprintReadOnly)
		UTexture2D* texture;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		EVLCPlayerActorState PlayerState;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		EVLCMediaState MediaState;
public:

	UFUNCTION(BlueprintCallable, Category = "VLCPlayer", meta = (Tooltip = "初始化VLC(一般情况不需要手动调用)"))
	bool VLC_Init();

	UFUNCTION(BlueprintCallable, Category = "VLCPlayer", meta = (Tooltip = "反初始化VLC(一般情况不需要手动调用)"))
	bool VLC_Uninit();

	UFUNCTION(BlueprintCallable, Category = "VLCPlayer", meta = (Tooltip = "播放视频(videoURL为空表示不修改视频地址,使用最后播放的视频地址或之前设置好的视频地址)"))
	bool VLC_PlayVideo(FString videoURL = FString(""));

	UFUNCTION(BlueprintCallable, Category = "VLCPlayer", meta = (Tooltip = "停止视频"))
	bool VLC_StopVideo();
	
	UFUNCTION(BlueprintCallable, Category = "VLCPlayer", meta = (Tooltip = "设置音量"))
	bool VLC_SetAudioVolume(int volume);

	UFUNCTION(BlueprintPure, Category = "VLCPlayer", meta = (Tooltip = "获取音量"))
	int VLC_GetAudioVolume();

private:
	
	bool PlayVideo(FString videoURL = "");

	bool StopVideo();

	bool StopVideoCallback();

	static void* lock(void *opaque, void **p_pixels);

	/*##get the argb picture AND save to file*/
	static void unlock(void *opaque, void *id, void *const *p_pixels);

	static void display(void *opaque, void *id);

	static unsigned video_format_callback(void **opaque, char *chroma,unsigned *width, unsigned *height,unsigned *pitches,unsigned *lines);

	static void video_cleanup_callback(void *opaque);

	static void video_log_callback(void *data, int level, const libvlc_log_t *ctx, const char *fmt, va_list args);

	static void video_event_callback(const struct libvlc_event_t *p_event, void *p_data);

	void register_event_notification();

	void unregister_event_notification();
};
