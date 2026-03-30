# WebVideoStream Plugin

Unreal에서 YouTube URL 기반 영상을 재생하기 위한 Runtime 플러그인입니다.

핵심은 `UWebVideoComponent` 하나로 구성되어 있고, 실제 재생은 `libVLC`로 처리합니다. YouTube 원본 URL은 직접 재생하지 않고, 로컬 Python 서버(`PythonServer/yt_server.py`)에 질의해 `direct_url`을 받아 재생합니다.

## 1) 무엇을 하는 플러그인인가

- Actor에 `UWebVideoComponent`를 붙여 웹 영상 재생
- 메시(Material)에 동영상 텍스처를 실시간 갱신
- 3D Spatial Audio 출력
- Blueprint 이벤트(`OnStateChanged`, `OnPlaybackEnded`) 제공
- YouTube URL -> 직접 스트림 URL 변환은 외부 로컬 서버 사용

## 2) 프로젝트 내 위치

- 플러그인 루트: `Plugins/WebVideoStream`
- 주요 코드:
  - `Plugins/WebVideoStream/Source/WebVideoStream/Public/WebVideoComponent.h`
  - `Plugins/WebVideoStream/Source/WebVideoStream/Private/WebVideoComponent.cpp`
  - `Plugins/WebVideoStream/Source/WebVideoStream/WebVideoStream.Build.cs`
- 기본 머티리얼 에셋: `Plugins/WebVideoStream/Content/M_WebVideoMaterial.uasset`

## 3) 빠른 사용 순서

1. `PythonServer/run_server.bat`로 로컬 서버를 실행합니다.
2. Unreal Actor에 `WebVideoComponent`를 추가합니다.
3. 화면용 Mesh를 `TargetMesh`로 지정합니다.
4. 필요 시 `InitialYouTubeURL`, `bAutoPlayOnStart`를 설정합니다.
5. Blueprint에서 `PlayVideo("https://...")` 호출로 재생합니다.

## 4) 주요 설정값

### Rendering

- `TargetMesh`: 비디오를 표시할 메시 컴포넌트
- `BaseMaterial`: 비디오 입력용 머티리얼(기본값으로 `M_WebVideoMaterial` 로드 시도)
- `TextureParameterName` (기본 `VideoInput`): 동적 텍스처 파라미터명
- `VideoRatioParameterName` (기본 `VideoAspectRatio`): 원본 영상 비율 파라미터명
- `ScreenRatioParameterName` (기본 `ScreenAspectRatio`): 메시 표면 비율 파라미터명
- `bUseMaterialSlot`, `TargetMaterialSlot`: 특정 머티리얼 슬롯에만 적용할지 여부

### Playback

- `InitialYouTubeURL`: 시작 시 사용할 URL
- `bAutoPlayOnStart`: BeginPlay에서 자동 재생 여부
- `bLoopVideo`: 재생 종료 시 처음으로 되감아 반복 재생

### Audio

- `InitialVolume` (0~100): 초기 볼륨
- `MinSoundDistance`: 감쇠 시작 거리
- `MaxSoundDistance`: 감쇠 종료 거리

## 5) Blueprint API

- `PlayVideo(FString URL)`: YouTube URL 입력 -> 로컬 서버 호출 -> direct URL 재생
- `SetPaused(bool bPause)`: 일시정지/재개
- `StopVideo()`: 정지
- `SeekToTime(float Seconds)`: 특정 시점 이동
- `SetVolume(int32 Volume)`: 볼륨 변경(클램프 0~100)
- `GetCurrentPlayerState()`: 현재 재생 상태 반환
- `GetCurrentTime()`: 현재 재생 시간(초)
- `GetTotalDuration()`: 전체 길이(초)

이벤트:

- `OnStateChanged(EWebVideoState)`
- `OnPlaybackEnded()`

## 6) 핵심 로직 요약

1. `BeginPlay`에서 동적 텍스처/오디오 스트림/`libVLC` 플레이어를 초기화합니다.
2. `PlayVideo` 호출 시 `http://127.0.0.1:5000/get_link?url=...`로 GET 요청을 보냅니다.
3. 성공 응답(`status=success`)이면 JSON의 `direct_url`로 `PrivatePlayVideo`를 호출합니다.
4. VLC 비디오 콜백(`VLCVidLock/Unlock`)으로 픽셀 버퍼를 채우고, `TickComponent`에서 텍스처를 갱신합니다.
5. 오디오 콜백(`VLCAudPlay`)으로 PCM 데이터를 `USoundWaveProcedural`에 큐잉해 재생합니다.

## 7) Build/런타임 관련 참고

- `WebVideoStream.Build.cs`에서 `ThirdParty/LibVLC`의 include/lib를 링크합니다.
- `libvlc.dll`, `libvlccore.dll`, `plugins/*`를 RuntimeDependencies로 복사하도록 설정되어 있습니다.
- 서버 엔드포인트는 코드상 고정값 `127.0.0.1:5000`입니다.

## 8) 트러블슈팅

- 재생이 시작되지 않음:
  - Python 서버가 실행 중인지 확인
  - YouTube URL이 유효한지 확인
  - 서버 응답 JSON에 `status: success`가 오는지 확인
- 화면이 검거나 비율이 이상함:
  - `BaseMaterial`과 텍스처 파라미터 이름이 맞는지 확인
  - `TargetMesh`가 올바르게 지정되었는지 확인
- 소리가 안 남:
  - `InitialVolume` 값 확인
  - 오디오 컴포넌트가 메시에 attach 되었는지 확인
  - 거리 감쇠(`MinSoundDistance`, `MaxSoundDistance`) 값 확인

