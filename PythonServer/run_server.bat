@echo off
SETLOCAL
TITLE Web-Job: YouTube Link Extractor

:: Get the current directory of the batch file
SET "SERVER_DIR=%~dp0"
CD /D "%SERVER_DIR%"

echo [INFO] Starting Portable Python Environment...

:: Check if the embed python exists
IF NOT EXIST ".\python_embed\python.exe" (
    echo [ERROR] python_embed\python.exe not found!
    echo [ERROR] Please check your folder structure.
    pause
    exit /b
)

:: Run the script using the local engine
".\python_embed\python.exe" "yt_server.py"

echo.
echo [INFO] Server has stopped.
pause