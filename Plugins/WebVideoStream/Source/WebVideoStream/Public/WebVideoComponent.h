// Copyright ood11611doo. All right reserved.


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "vlc/vlc.h"
#include "WebVideoComponent.generated.h"

UENUM(BlueprintType)
enum class EWebVideoState : uint8
{
    Idle, Opening, Buffering, Playing, Paused, Stopped, Ended, Error
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVideoStateChanged, EWebVideoState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVideoPlaybackEnded);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WEBVIDEOSTREAM_API UWebVideoComponent : public UActorComponent
{
    GENERATED_BODY()

public:	
    UWebVideoComponent();

    // --- Settings ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo")
    FString InitialYouTubeURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo")
    bool bAutoPlayOnStart = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo")
    bool bLoopVideo = false;

    UPROPERTY(EditAnywhere, Category = "WebVideo")
    UMaterialInterface* BaseMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo", meta = (ClampMin = "0", ClampMax = "100"))
    int32 InitialVolume = 75;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo")
    float MinSoundDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo")
    float MaxSoundDistance = 3500.0f;

    // --- Events ---
    UPROPERTY(BlueprintAssignable, Category = "WebVideo")
    FOnVideoStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "WebVideo")
    FOnVideoPlaybackEnded OnPlaybackEnded;

    // --- API ---
    UFUNCTION(BlueprintCallable, Category = "WebVideo")
    void PlayVideo(FString URL);

    UFUNCTION(BlueprintCallable, Category = "WebVideo")
    void SetPaused(bool bPause);

    UFUNCTION(BlueprintCallable, Category = "WebVideo")
    void StopVideo();

    UFUNCTION(BlueprintCallable, Category = "WebVideo")
    void SeekToTime(float Seconds);

    UFUNCTION(BlueprintCallable, Category = "WebVideo")
    void SetVolume(int32 Volume);

    UFUNCTION(BlueprintPure, Category = "WebVideo")
    EWebVideoState GetCurrentPlayerState();

    UFUNCTION(BlueprintPure, Category = "WebVideo")
    float GetCurrentTime();

    UFUNCTION(BlueprintPure, Category = "WebVideo")
    float GetTotalDuration();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "WebVideo")
    bool bIsLoading = false;

    UPROPERTY(BlueprintReadOnly, Category = "WebVideo")
    FString CurrentVideoTitle;

    UPROPERTY(BlueprintReadOnly, Category = "WebVideo")
    float CurrentVideoDuration = 0.0f;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    libvlc_instance_t* VLCInstance = nullptr;
    libvlc_media_player_t* VLCMediaPlayer = nullptr;

    void UpdateTexture();
    static void* vlc_video_lock(void* data, void** p_pixels);
    static void vlc_video_unlock(void* data, void* id, void* const* p_pixels);
    static void vlc_video_display(void* data, void* id);

    UPROPERTY()
    UTexture2D* DynamicTexture;
    
    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMat;

    TArray<uint8> PixelBuffer;
    FUpdateTextureRegion2D UpdateRegion;
    const int32 VideoWidth = 1280;
    const int32 VideoHeight = 720;
    FCriticalSection RenderMutex;

    void Update3DAudio();
    void Internal_StartVideo(FString DirectURL);
    EWebVideoState LastKnownState = EWebVideoState::Idle;
};