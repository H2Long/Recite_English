#!/bin/bash
# ============================================================================
# 一键构建脚本
# 用法: ./build.sh [install|package|clean]
#   (无参数)   → 只编译
#   install    → 编译 + 安装到 build/dist/
#   package    → 编译 + 打包成可分发压缩包
#   clean      → 清理构建产物
# ============================================================================

set -e

PROJECT="背单词软件"
VERSION="5.0.0"
BUILD_DIR="build"
DIST_DIR="${BUILD_DIR}/dist"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info()  { echo -e "${GREEN}[INFO]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; }

# 检查依赖
check_deps() {
    info "检查依赖..."
    
    if ! command -v cmake &> /dev/null; then
        error "cmake 未安装！请先安装: sudo apt install cmake"
        exit 1
    fi
    
    if ! command -v ninja &> /dev/null; then
        warn "ninja 未安装，使用 make 替代"
        GENERATOR=""
    else
        GENERATOR="-G Ninja"
    fi
    
    # 检查 raylib 是否可用
    if ! pkg-config --exists raylib 2>/dev/null && \
       ! ldconfig -p | grep -q libraylib 2>/dev/null && \
       [ ! -f /usr/local/lib/libraylib.a ] && \
       [ ! -f /usr/lib/x86_64-linux-gnu/libraylib.so ]; then
        warn "未检测到 raylib，尝试自动安装..."
        if command -v apt &> /dev/null; then
            sudo apt install -y libraylib-dev 2>/dev/null || {
                error "自动安装失败，请手动安装 raylib:"
                echo "  sudo apt install libraylib-dev"
                exit 1
            }
        else
            error "请先安装 raylib 库"
            exit 1
        fi
    fi
    
    info "依赖检查通过"
}

# 清理
do_clean() {
    info "清理构建目录..."
    rm -rf "${BUILD_DIR}"
    info "清理完成"
}

# 编译
do_build() {
    info "开始编译..."
    
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    cmake .. ${GENERATOR} -DCMAKE_BUILD_TYPE=Release
    cmake --build . -- -j$(nproc)
    cd ..
    
    info "编译完成！"
}

# 安装到 dist 目录
do_install() {
    info "打包程序文件..."
    
    rm -rf "${DIST_DIR}"
    mkdir -p "${DIST_DIR}"
    
    # 1. 复制可执行文件
    cp "${BUILD_DIR}/main_c" "${DIST_DIR}/"
    
    # 2. 复制数据文件（字体 + 单词库）
    cp -r data "${DIST_DIR}/"
    
    # 3. 复制运行说明
    cat > "${DIST_DIR}/使用说明.txt" << EOF
${PROJECT} v${VERSION}

运行方法：
  Linux:  终端执行 ./main_c
  Windows:双击 main_c.exe（需配套 .dll）

数据文件说明：
  data/words.txt          ← 单词库（可用记事本编辑）
  data/accounts.txt       ← 账号数据
  data/progress*.txt      ← 学习进度
  data/fonts/             ← 字体文件

提示：按 F11 可全屏，按 Esc 退出。
EOF
    
    # 4. Linux 下设置可执行权限
    chmod +x "${DIST_DIR}/main_c"
    
    info "打包完成！程序在 ${DIST_DIR}/"
    echo "直接运行: ${DIST_DIR}/main_c"
}

# 打包成压缩包
do_package() {
    do_install
    
    local PKG_NAME="${PROJECT}-v${VERSION}-linux"
    cd "${BUILD_DIR}"
    
    # 创建 tar.gz 包
    tar czf "${PKG_NAME}.tar.gz" dist/
    echo ""
    info "===================================="
    info "分发包已创建:"
    info "  ${BUILD_DIR}/${PKG_NAME}.tar.gz"
    info "===================================="
    echo ""
    info "用户解压后直接运行:"
    echo "  tar xzf ${PKG_NAME}.tar.gz"
    echo "  cd dist"
    echo "  ./main_c"
}

# ============================================================================
# 主流程
# ============================================================================
case "${1:-build}" in
    build)
        check_deps
        do_build
        ;;
    install)
        check_deps
        do_build
        do_install
        ;;
    package)
        check_deps
        do_build
        do_package
        ;;
    clean)
        do_clean
        ;;
    *)
        echo "用法: $0 [build|install|package|clean]"
        exit 1
        ;;
esac
