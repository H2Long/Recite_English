@echo off
REM ============================================================================
REM Windows NSIS 安装程序打包脚本
REM 前置条件:
REM   1. 已安装 NSIS (https://nsis.sourceforge.io/Download)
REM   2. 已用 CMake 编译出 build\Release\main_c.exe
REM 用法: 双击运行 或 在命令行执行 build_nsis.bat
REM ============================================================================

echo [INFO] 检查编译产物...

if not exist "build\Release\main_c.exe" (
    echo [ERROR] 未找到 build\Release\main_c.exe
    echo [INFO]  请先编译:
    echo         mkdir build ^&^& cd build
    echo         cmake .. -DCMAKE_BUILD_TYPE=Release
    echo         cmake --build . --config Release
    pause
    exit /b 1
)

echo [INFO] 检查 NSIS...
where makensis >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] 未找到 makensis，请先安装 NSIS
    echo         https://nsis.sourceforge.io/Download
    pause
    exit /b 1
)

echo [INFO] 正在生成安装程序...
makensis /V2 packaging\NSIS\installer.nsi

if exist "build\背单词软件-Setup-v5.0.0.exe" (
    echo.
    echo [INFO] ====================================
    echo [INFO] 安装程序已生成!
    echo [INFO]   build\背单词软件-Setup-v5.0.0.exe
    echo [INFO] ====================================
) else (
    echo [ERROR] 生成失败，请检查 NSIS 输出
)

pause
