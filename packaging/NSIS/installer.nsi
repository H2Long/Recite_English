; ============================================================================
; 背单词软件 NSIS 安装脚本
; 用法: 在 Windows 上用 NSIS 编译此脚本生成安装程序
; 前置: 先用 CMake 编译出 build\Release\main_c.exe
; ============================================================================

!include "MUI2.nsh"
!include "FileFunc.nsh"

; ---- 基本信息 ----
Name "背单词软件"
OutFile "build\背单词软件-Setup-v5.0.0.exe"
InstallDir "$PROGRAMFILES\ReciteEnglish"
InstallDirRegKey HKLM "Software\ReciteEnglish" "InstallDir"
RequestExecutionLevel admin
Unicode True

; ---- 版本信息 ----
VIProductVersion "5.0.0.0"
VIAddVersionKey "ProductName" "背单词软件"
VIAddVersionKey "ProductVersion" "5.0.0"
VIAddVersionKey "FileDescription" "英语单词记忆桌面应用"
VIAddVersionKey "LegalCopyright" ""

; ---- 界面 ----
!define MUI_ABORTWARNING

; ---- 安装页面 ----
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "packaging\NSIS\LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; ---- 卸载页面 ----
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "English"

; ============================================================================
; 安装区段
; ============================================================================
Section "安装主程序" SecMain
    SectionIn RO

    SetOutPath "$INSTDIR"

    ; 主程序（MSYS2 产出在 build\，VS2022 产出在 build\Release\，优先找 Release）
    !if /FileExists "build\Release\main_c.exe"
        File "build\Release\main_c.exe"
    !else
        File "build\main_c.exe"
    !endif

    ; 数据文件
    SetOutPath "$INSTDIR\data"
    File "data\words.txt"
    File "data\accounts.txt"
    File /nonfatal "data\plans.txt"
    File /nonfatal "data\progress.txt"

    SetOutPath "$INSTDIR\data\fonts"
    File "data\fonts\NotoSansCJK.otf"
    File "data\fonts\DejaVuSans.ttf"
    File "data\fonts\DejaVuSansMono.ttf"

    ; 写入注册表
    WriteRegStr HKLM "Software\ReciteEnglish" "InstallDir" "$INSTDIR"

    ; 写入卸载信息
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReciteEnglish" \
        "DisplayName" "背单词软件"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReciteEnglish" \
        "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReciteEnglish" \
        "DisplayVersion" "5.0.0"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReciteEnglish" \
        "Publisher" "ReciteEnglish"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReciteEnglish" \
        "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReciteEnglish" \
        "NoRepair" 1

    ; 估算安装大小
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReciteEnglish" \
        "EstimatedSize" "$0"

    ; 写入卸载程序
    WriteUninstaller "$INSTDIR\uninstall.exe"

    ; ---- 开始菜单快捷方式 ----
    CreateDirectory "$SMPROGRAMS\背单词软件"
    CreateShortcut "$SMPROGRAMS\背单词软件\背单词软件.lnk" "$INSTDIR\main_c.exe"
    CreateShortcut "$SMPROGRAMS\背单词软件\卸载.lnk" "$INSTDIR\uninstall.exe"

    ; ---- 桌面快捷方式 ----
    CreateShortcut "$DESKTOP\背单词软件.lnk" "$INSTDIR\main_c.exe"
SectionEnd

; ============================================================================
; 卸载区段
; ============================================================================
Section "Uninstall"
    ; 删除文件
    Delete "$INSTDIR\main_c.exe"
    Delete "$INSTDIR\data\words.txt"
    Delete "$INSTDIR\data\accounts.txt"
    Delete "$INSTDIR\data\plans.txt"
    Delete "$INSTDIR\data\progress.txt"
    Delete "$INSTDIR\data\fonts\NotoSansCJK.otf"
    Delete "$INSTDIR\data\fonts\DejaVuSans.ttf"
    Delete "$INSTDIR\data\fonts\DejaVuSansMono.ttf"
    RMDir "$INSTDIR\data\fonts"
    RMDir "$INSTDIR\data"
    Delete "$INSTDIR\uninstall.exe"
    RMDir "$INSTDIR"

    ; 删除快捷方式
    Delete "$DESKTOP\背单词软件.lnk"
    Delete "$SMPROGRAMS\背单词软件\背单词软件.lnk"
    Delete "$SMPROGRAMS\背单词软件\卸载.lnk"
    RMDir "$SMPROGRAMS\背单词软件"

    ; 删除注册表
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReciteEnglish"
    DeleteRegKey HKLM "Software\ReciteEnglish"
SectionEnd
