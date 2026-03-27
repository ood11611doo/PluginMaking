// Copyright ood11611doo. All right reserved.


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "vlc/vlc.h"
#include "WebVideoComponent.generated.h"

class UAudioComponent;
class USoundWaveProcedural;

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
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
public:
    // --- Settings: Rendering ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo|Settings")
    FComponentReference TargetMesh;

    UPROPERTY(EditAnywhere, Category = "WebVideo|Settings")
    UMaterialInterface* BaseMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo|Settings")
    FName TextureParameterName = FName("VideoInput");
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo|Settings")
    FName VideoRatioParameterName = FName("VideoAspectRatio");
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo|Settings")
    FName ScreenRatioParameterName = FName("ScreenAspectRatio");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo|Settings")
    bool bUseMaterialSlot = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo|Settings", meta = (EditCondition = "bUseMaterialSlot"))
    int32 TargetMaterialSlot = 0;
    
    // --- Settings: Playback ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo|Settings")
    FString InitialYouTubeURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo|Settings")
    bool bAutoPlayOnStart = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo|Settings")
    bool bLoopVideo = false;

    // --- Settings: Audio ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo|Audio", meta = (ClampMin = "0", ClampMax = "100"))
    int32 InitialVolume = 75;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo|Audio")
    float MinSoundDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebVideo|Audio")
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
    
    UPROPERTY()
    UAudioComponent* AudioComponent;

    UPROPERTY()
    USoundWaveProcedural* AudioStream;


private:
    UPROPERTY()
    UMeshComponent* ResolvedMesh;

    UPROPERTY()
    UTexture2D* DynamicTexture;
    
    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMat;
    
    libvlc_instance_t* VLCInstance = nullptr;
    libvlc_media_player_t* VLCMediaPlayer = nullptr;
    TArray<uint8> PixelBuffer;
    FUpdateTextureRegion2D UpdateRegion;
    const int32 VideoWidth = 1280;
    const int32 VideoHeight = 720;
    FCriticalSection RenderMutex;
    EWebVideoState LastKnownState = EWebVideoState::Idle;

    void UpdateTexture();
    void UpdateSurfaceAspectRatio();
    void PrivatePlayVideo(FString DirectURL);
    
    static void* VLCVidLock(void* data, void** p_pixels);
    static void VLCVidUnlock(void* data, void* id, void* const* p_pixels);
    static void VLCVidDisplay(void* data, void* id);
    static void VLCAudPlay(void* Data, const void* Samples, uint32_t Count, int64_t PTS);
    
    // --- Editors only ---
#if WITH_EDITOR
#endif
};