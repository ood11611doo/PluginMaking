# PluginMaking

이 프로젝트는 **Unreal용 동영상 재생 플러그인**과 **로컬 URL 추출 Python 서버**를 함께 사용합니다.

## 문서 바로가기

- 플러그인 문서: `Plugins/WebVideoStream/README.md`
- Python 서버 문서: `PythonServer/README.md`

## 전체 동작 흐름(요약)

1. `PythonServer/run_server.bat` 실행
2. Unreal에서 `WebVideoComponent`가 YouTube URL로 `PlayVideo` 호출
3. 플러그인이 `http://127.0.0.1:5000/get_link`로 direct URL 요청
4. 서버가 `yt-dlp`로 `direct_url` 반환
5. 플러그인이 `libVLC`로 영상/오디오 재생

## 빠른 시작 체크

- Python 서버가 먼저 실행 중인지 확인
- Actor에 `WebVideoComponent` 부착
- `TargetMesh`와 `BaseMaterial` 설정 확인
- `PlayVideo("https://...")` 호출

