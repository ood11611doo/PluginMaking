// Copyright ood11611doo. All right reserved.


#include "WebVideoComponent.h"
#include "Components/MeshComponent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "GenericPlatform/GenericPlatformHttp.h"

UWebVideoComponent::UWebVideoComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWebVideoComponent::BeginPlay()
{
    Super::BeginPlay();

    DynamicTexture = UTexture2D::CreateTransient(VideoWidth, VideoHeight, PF_B8G8R8A8);
    DynamicTexture->UpdateResource();
    PixelBuffer.SetNumZeroed(VideoWidth * VideoHeight * 4);
    UpdateRegion = FUpdateTextureRegion2D(0, 0, 0, 0, VideoWidth, VideoHeight);

    const char* const vlc_args[] = { "--no-osd", "--no-video-title-show", "--vout=vmem" };
    VLCInstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);

    if (VLCInstance)
    {
        VLCMediaPlayer = libvlc_media_player_new(VLCInstance);

        if (VLCMediaPlayer)
        {
            libvlc_video_set_callbacks(VLCMediaPlayer, vlc_video_lock, vlc_video_unlock, vlc_video_display, this);
            libvlc_video_set_format(VLCMediaPlayer, "RV32", VideoWidth, VideoHeight, VideoWidth * 4);
            libvlc_audio_set_volume(VLCMediaPlayer, InitialVolume);
        }

        if (BaseMaterial)
        {
            if (UMeshComponent* Mesh = GetOwner()->FindComponentByClass<UMeshComponent>())
            {
                DynamicMat = Mesh->CreateDynamicMaterialInstance(0, BaseMaterial);
                DynamicMat->SetTextureParameterValue(FName("VideoInput"), DynamicTexture);
            }
        }

        if (bAutoPlayOnStart && !InitialYouTubeURL.IsEmpty()) PlayVideo(InitialYouTubeURL);
    }
}

void UWebVideoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!VLCMediaPlayer) return;

    EWebVideoState CurrentState = GetCurrentPlayerState();
    if (CurrentState != LastKnownState)
    {
        OnStateChanged.Broadcast(CurrentState);
        if (CurrentState == EWebVideoState::Ended)
        {
            OnPlaybackEnded.Broadcast();
            if (bLoopVideo) { SeekToTime(0.0f); libvlc_media_player_play(VLCMediaPlayer); }
        }
        LastKnownState = CurrentState;
    }

    UpdateTexture();
    Update3DAudio();
}

void UWebVideoComponent::PlayVideo(FString YouTubeURL)
{
    bIsLoading = true;
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    FString ApiURL = FString::Printf(TEXT("http://127.0.0.1:5000/get_link?url=%s"), *FGenericPlatformHttp::UrlEncode(YouTubeURL));
    
    Request->SetURL(ApiURL);
    Request->SetVerb(TEXT("GET"));
    Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
    {
        bIsLoading = false;
        if (bSuccess && Res.IsValid())
        {
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Res->GetContentAsString());
            if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject->GetStringField(TEXT("status")) == TEXT("success"))
            {
                CurrentVideoTitle = JsonObject->GetStringField(TEXT("title"));
                CurrentVideoDuration = JsonObject->GetNumberField(TEXT("duration"));
                Internal_StartVideo(JsonObject->GetStringField(TEXT("direct_url")));
            }
        }
    });
    Request->ProcessRequest();
}

void UWebVideoComponent::SetPaused(bool bPause)
{
    if (!VLCMediaPlayer) return;
    EWebVideoState State = GetCurrentPlayerState();
    if (bPause && State == EWebVideoState::Playing) libvlc_media_player_set_pause(VLCMediaPlayer, 1);
    else if (!bPause && State == EWebVideoState::Paused) libvlc_media_player_set_pause(VLCMediaPlayer, 0);
}

void UWebVideoComponent::StopVideo()
{
    if (VLCMediaPlayer) libvlc_media_player_stop(VLCMediaPlayer);
}

void UWebVideoComponent::SeekToTime(float Seconds)
{
    if (VLCMediaPlayer) libvlc_media_player_set_time(VLCMediaPlayer, (int64)(Seconds * 1000.0f));
}

void UWebVideoComponent::SetVolume(int32 Volume)
{
    InitialVolume = FMath::Clamp(Volume, 0, 100);
    if (VLCMediaPlayer) libvlc_audio_set_volume(VLCMediaPlayer, InitialVolume);
}

EWebVideoState UWebVideoComponent::GetCurrentPlayerState()
{
    if (!VLCMediaPlayer) return EWebVideoState::Idle;
    switch (libvlc_media_player_get_state(VLCMediaPlayer))
    {
        case libvlc_Opening:   return EWebVideoState::Opening;
        case libvlc_Buffering: return EWebVideoState::Buffering;
        case libvlc_Playing:   return EWebVideoState::Playing;
        case libvlc_Paused:    return EWebVideoState::Paused;
        case libvlc_Stopped:   return EWebVideoState::Stopped;
        case libvlc_Ended:     return EWebVideoState::Ended;
        case libvlc_Error:     return EWebVideoState::Error;
        default:               return EWebVideoState::Idle;
    }
}

float UWebVideoComponent::GetCurrentTime()
{
    return (VLCMediaPlayer) ? (float)libvlc_media_player_get_time(VLCMediaPlayer) / 1000.0f : 0.0f;
}

float UWebVideoComponent::GetTotalDuration()
{
    if (!VLCMediaPlayer) return 0.0f;
    int64 Length = libvlc_media_player_get_length(VLCMediaPlayer);
    return (Length <= 0) ? CurrentVideoDuration : (float)Length / 1000.0f;
}

void UWebVideoComponent::Update3DAudio()
{
    if (!VLCMediaPlayer) return;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            float Dist = FVector::Dist(Pawn->GetActorLocation(), GetOwner()->GetActorLocation());
            float Alpha = 1.0f - FMath::Clamp((Dist - MinSoundDistance) / (MaxSoundDistance - MinSoundDistance), 0.0f, 1.0f);
            libvlc_audio_set_volume(VLCMediaPlayer, FMath::RoundToInt(Alpha * InitialVolume));
        }
    }
}

void UWebVideoComponent::UpdateTexture()
{
    if (!DynamicTexture || !DynamicTexture->GetResource() || PixelBuffer.Num() == 0) return;
    if (RenderMutex.TryLock())
    {
        DynamicTexture->UpdateTextureRegions(0, 1, &UpdateRegion, (uint32)(VideoWidth * 4), (uint32)4, PixelBuffer.GetData());
        RenderMutex.Unlock();
    }
}

void UWebVideoComponent::Internal_StartVideo(FString DirectURL)
{
    if (!VLCInstance || !VLCMediaPlayer) return;
    FTCHARToUTF8 Converter(*DirectURL);
    if (libvlc_media_t* Media = libvlc_media_new_location(VLCInstance, Converter.Get()))
    {
        libvlc_media_player_set_media(VLCMediaPlayer, Media);
        libvlc_media_player_play(VLCMediaPlayer);
        libvlc_media_release(Media);
    }
}

void* UWebVideoComponent::vlc_video_lock(void* data, void** p_pixels)
{
    UWebVideoComponent* Self = static_cast<UWebVideoComponent*>(data);
    if (Self) { Self->RenderMutex.Lock(); *p_pixels = Self->PixelBuffer.GetData(); }
    return nullptr;
}

void UWebVideoComponent::vlc_video_unlock(void* data, void* id, void* const* p_pixels)
{
    if (UWebVideoComponent* Self = static_cast<UWebVideoComponent*>(data)) Self->RenderMutex.Unlock();
}

void UWebVideoComponent::vlc_video_display(void* data, void* id) {}

void UWebVideoComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (VLCMediaPlayer) { libvlc_media_player_stop(VLCMediaPlayer); libvlc_media_player_release(VLCMediaPlayer); }
    if (VLCInstance) libvlc_release(VLCInstance);
    Super::EndPlay(EndPlayReason);
}