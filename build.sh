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
    if ! ldconfig -p | grep -q libraylib 2>/dev/null && \
       [ ! -f /usr/local/lib/libraylib.a ] && \
       [ ! -f /usr/local/lib/libraylib.so ] && \
       [ ! -f /usr/lib/x86_64-linux-gnu/libraylib.so ]; then
        warn "未检测到 raylib，尝试自动安装..."
        if command -v apt &> /dev/null; then
            sudo apt install -y libraylib-dev 2>/dev/null || {
                error "自动安装失败，请手动安装:"
                echo "  Ubuntu/Debian: sudo apt install libraylib-dev"
                echo "  macOS:          brew install raylib"
                echo "  Windows:        vcpkg install raylib"
                exit 1
            }
        else
            error "请先安装 raylib 库，安装方法见: https://github.com/raysan5/raylib"
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

    # 检测链接方式：静态还是动态
    if ldd "${BUILD_DIR}/main_c" 2>/dev/null | grep -q libraylib; then
        LINK_TYPE="dynamic"
        info "链接方式: 动态链接（需附带 libraylib.so）"
    else
        LINK_TYPE="static"
        info "链接方式: 静态链接（完全独立，无需额外文件）"
    fi
    echo "${LINK_TYPE}" > "${BUILD_DIR}/.link_type"

    info "编译完成！"
}

# 找 raylib.so 的路径
find_raylib_so() {
    # 从 ldd 输出中提取路径
    local so_path=$(ldd "${BUILD_DIR}/main_c" 2>/dev/null | grep libraylib | awk '{print $3}')
    if [ -n "$so_path" ] && [ -f "$so_path" ]; then
        echo "$so_path"
        return 0
    fi
    # 备选：从常见路径找
    for p in /usr/local/lib/libraylib.so /usr/lib/x86_64-linux-gnu/libraylib.so; do
        if [ -f "$p" ]; then
            echo "$p"
            return 0
        fi
    done
    return 1
}

# 安装到 dist 目录
do_install() {
    info "打包程序文件..."

    rm -rf "${DIST_DIR}"
    mkdir -p "${DIST_DIR}"

    # 1. 复制可执行文件
    cp "${BUILD_DIR}/main_c" "${DIST_DIR}/"

    # 2. 如果是动态链接，附带 libraylib.so
    local link_type
    if [ -f "${BUILD_DIR}/.link_type" ]; then
        link_type=$(cat "${BUILD_DIR}/.link_type")
    else
        link_type="static"
    fi

    if [ "$link_type" = "dynamic" ]; then
        local so_path
        if so_path=$(find_raylib_so); then
            cp "$so_path" "${DIST_DIR}/"
            info "已附带动态库: $(basename "$so_path")"
        else
            warn "找不到 libraylib.so，用户需自行安装 raylib"
        fi
    fi

    # 3. 复制数据文件（字体 + 单词库）
    cp -r data "${DIST_DIR}/"

    # 4. 复制运行说明
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

提示：按 Esc 退出程序。
EOF

    # 5. Linux 下设置可执行权限
    chmod +x "${DIST_DIR}/main_c" 2>/dev/null || true

    # 6. 显示 dist 目录大小
    local dist_size=$(du -sh "${DIST_DIR}" | cut -f1)
    info "打包完成！程序在 ${DIST_DIR}/ (共 ${dist_size})"
    echo "直接运行: ${DIST_DIR}/main_c"
}

# 打包成压缩包
do_package() {
    do_install

    local PKG_NAME="${PROJECT}-v${VERSION}-linux"
    cd "${BUILD_DIR}"

    # 创建 tar.gz 包
    tar czf "${PKG_NAME}.tar.gz" dist/
    local pkg_size=$(du -h "${PKG_NAME}.tar.gz" | cut -f1)
    cd ..

    echo ""
    info "===================================="
    info "分发包已创建:"
    info "  ${BUILD_DIR}/${PKG_NAME}.tar.gz  (${pkg_size})"
    info "===================================="
    echo ""
    info "收包方解压后运行:"
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
