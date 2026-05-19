#!/bin/bash
# ============================================================================
# AppImage 打包脚本
# 用法: ./build_appimage.sh
# 产出: build/背单词软件-x86_64.AppImage
# ============================================================================

set -e

PROJECT="背单词软件"
VERSION="5.0.0"
ARCH="x86_64"
BUILD_DIR="build"
APPDIR="${BUILD_DIR}/AppDir"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'
info()  { echo -e "${GREEN}[INFO]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }

# ---- 1. 确保二进制已编译 ----
if [ ! -f "${BUILD_DIR}/main_c" ]; then
    info "未找到编译产物，先执行编译..."
    ./build.sh build
fi
info "二进制: ${BUILD_DIR}/main_c"

# ---- 2. 清理旧 AppDir ----
rm -rf "${APPDIR}"
mkdir -p "${APPDIR}/usr/bin"
mkdir -p "${APPDIR}/usr/share/recite-english"
mkdir -p "${APPDIR}/usr/share/icons/hicolor/scalable/apps"

# ---- 3. 布局 AppDir ----
# 二进制
cp "${BUILD_DIR}/main_c" "${APPDIR}/usr/bin/"
chmod +x "${APPDIR}/usr/bin/main_c"

# 数据文件（与二进制同级，保持 ./data/ 相对路径）
cp -r data "${APPDIR}/usr/share/recite-english/"

# .desktop 文件
cp packaging/AppImage/背单词软件.desktop "${APPDIR}/"

# 图标（SVG 放 hicolor，同时复制到 AppDir 根目录供 appimagetool 使用）
cp packaging/AppImage/recite-english.svg "${APPDIR}/usr/share/icons/hicolor/scalable/apps/"
cp packaging/AppImage/recite-english.svg "${APPDIR}/recite-english.svg"

# AppRun 入口
cp packaging/AppImage/AppRun "${APPDIR}/"
chmod +x "${APPDIR}/AppRun"

info "AppDir 目录结构就绪"

# ---- 4. 获取 appimagetool ----
APPIMAGETOOL=""
if command -v appimagetool &>/dev/null; then
    APPIMAGETOOL="appimagetool"
elif [ -f "${BUILD_DIR}/appimagetool" ]; then
    APPIMAGETOOL="${BUILD_DIR}/appimagetool"
else
    info "下载 appimagetool..."
    TOOL_URL="https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage"
    curl -fSL -o "${BUILD_DIR}/appimagetool" "$TOOL_URL"
    chmod +x "${BUILD_DIR}/appimagetool"
    APPIMAGETOOL="${BUILD_DIR}/appimagetool"
    info "appimagetool 已下载到 ${BUILD_DIR}/"
fi

# ---- 5. 打包 ----
OUTPUT="${BUILD_DIR}/背单词软件-${ARCH}.AppImage"
rm -f "$OUTPUT"

info "正在生成 AppImage..."
ARCH=${ARCH} "$APPIMAGETOOL" --appimage-extract-and-run --no-appstream "${APPDIR}" "$OUTPUT" 2>&1

if [ -f "$OUTPUT" ]; then
    chmod +x "$OUTPUT"
    SIZE=$(du -h "$OUTPUT" | cut -f1)
    echo ""
    info "===================================="
    info "AppImage 打包成功!"
    info "  ${OUTPUT}  (${SIZE})"
    info "===================================="
    echo ""
    info "运行方式: chmod +x ${OUTPUT} && ./${OUTPUT}"
else
    error "AppImage 生成失败"
fi
