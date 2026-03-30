# PythonServer (yt-dlp Local Helper)

`PythonServer`는 Unreal 플러그인이 YouTube URL을 직접 재생 가능한 스트림 URL로 변환할 때 사용하는 로컬 보조 서버입니다.

메인 기능은 `yt_server.py`의 `/get_link` API 하나입니다.

## 1) 파일 구성

- 서버 스크립트: `PythonServer/yt_server.py`
- 실행 배치: `PythonServer/run_server.bat`
- 로컬 파이썬 런타임: `PythonServer/python_embed/python.exe`

## 2) 실행 방법

`run_server.bat`를 실행하면, 배치 파일 기준 상대 경로로 `python_embed/python.exe`를 찾아 `yt_server.py`를 실행합니다.

정상 실행 시 콘솔에 다음 안내가 출력됩니다.

- `PORTABLE WEB-JOB SERVER IS ONLINE`
- `URL: http://127.0.0.1:5000/get_link`

서버 종료는 콘솔에서 `Ctrl + C`로 할 수 있습니다.

## 3) API 스펙

### GET `/get_link`

Query:

- `url` (필수): YouTube 페이지 URL

예시:

`http://127.0.0.1:5000/get_link?url=https%3A%2F%2Fwww.youtube.com%2Fwatch%3Fv%3Dxxxx`

성공 응답 예시:

```json
{
  "status": "success",
  "title": "video title",
  "direct_url": "https://...",
  "duration": 123,
  "thumbnail": "https://..."
}
```

실패 응답 예시:

```json
{
  "status": "error",
  "message": "No URL provided"
}
```

## 4) 스크립트 핵심 로직

- `request.args.get('url')`로 입력 URL을 받습니다.
- `yt_dlp.YoutubeDL` 옵션:
  - `format: best[ext=mp4]` (VLC 친화적인 MP4 우선)
  - `quiet: True`
  - `no_warnings: True`
- `extract_info(..., download=False)`로 다운로드 없이 메타데이터/직접 URL만 추출합니다.
- Unreal에서 필요한 필드(`direct_url`, `title`, `duration`)를 JSON으로 반환합니다.

## 5) 플러그인 연동 포인트

Unreal 플러그인(`UWebVideoComponent::PlayVideo`)은 다음 고정 주소로 요청합니다.

- `http://127.0.0.1:5000/get_link?url=...`

즉, 플러그인 재생 전에 이 서버가 먼저 떠 있어야 합니다.

## 6) 주의사항

- 현재 인증/권한 제어 없이 로컬 호출 전제로 동작합니다.
- 코드상 Flask 바인딩은 `0.0.0.0:5000`이므로 네트워크 정책에 따라 외부 접근이 가능할 수 있습니다.
- 운영 배포 용도보다는 개발/로컬 테스트 보조 용도에 가깝습니다.

