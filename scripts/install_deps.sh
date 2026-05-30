#!/usr/bin/env bash
# ============================================================================
# DinoRunner — Dependency Installer
# ============================================================================
# Installs all build dependencies for the DinoRunner game on Linux or Windows.
#
# Usage:
#   chmod +x scripts/install_deps.sh
#   ./scripts/install_deps.sh
#
# Supported platforms:
#   - Linux (Debian/Ubuntu, Fedora, Arch Linux)
#   - Windows (MSYS2 UCRT64 / MinGW-w64)
#   - WSL (Windows Subsystem for Linux)
#
# What gets installed:
#   - C++17 compiler (GCC/Clang)
#   - CMake ≥ 3.16
#   - Make / Ninja
#   - SFML 2.6 development libraries and dependencies
#   - FreeType
#   - OpenGL, X11, audio libraries (Linux)
# ============================================================================

set -euo pipefail

# ──────────────────────────────────────────────────────────────────────────
# Color output helpers
# ──────────────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

info()    { echo -e "${CYAN}[INFO]${NC}  $*"; }
success() { echo -e "${GREEN}[OK]${NC}    $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $*"; }
error()   { echo -e "${RED}[ERROR]${NC} $*"; }

# ──────────────────────────────────────────────────────────────────────────
# Detect platform
# ──────────────────────────────────────────────────────────────────────────
detect_platform() {
    case "$(uname -s)" in
        Linux*)  echo "linux" ;;
        Darwin*) echo "macos" ;;
        MINGW*|MSYS*|UCRT*) echo "windows_msys2" ;;
        CYGWIN*) echo "windows_cygwin" ;;
        *)       echo "unknown" ;;
    esac
}

PLATFORM=$(detect_platform)

# ──────────────────────────────────────────────────────────────────────────
# Root check helper (Linux only — not needed on MSYS2)
# ──────────────────────────────────────────────────────────────────────────
require_root() {
    if [[ $EUID -ne 0 ]]; then
        # Try sudo if available
        if command -v sudo &>/dev/null; then
            exec sudo "$0" "$@"
        else
            error "This script must be run as root (or with sudo) on Linux."
            echo "  Try: sudo $0"
            exit 1
        fi
    fi
}

# ──────────────────────────────────────────────────────────────────────────
# Check if a command exists
# ──────────────────────────────────────────────────────────────────────────
cmd_exists() {
    command -v "$1" &>/dev/null
}

# ──────────────────────────────────────────────────────────────────────────
# Linux CMakeLists.txt warning (called after each Linux distro install)
# ──────────────────────────────────────────────────────────────────────────
linux_cmake_warning() {
    echo ""
    echo "  ╔══════════════════════════════════════════════════════════════════╗"
    echo "  ║  LINUX BUILD NOTICE                                            ║"
    echo "  ╠══════════════════════════════════════════════════════════════════╣"
    echo "  ║  The default CMakeLists.txt is configured for Windows           ║"
    echo "  ║  (MinGW static SFML + Win32 waveOut audio). To build on         ║"
    echo "  ║  Linux, you need to:                                            ║"
    echo "  ║                                                                ║"
    echo "  ║  1. Use system SFML instead of the bundled MinGW libs           ║"
    echo "  ║  2. Provide a Linux audio backend (the game uses Win32           ║"
    echo "  ║     waveOut which is Windows-only)                              ║"
    echo "  ║                                                                ║"
    echo "  ║  See scripts/CMakeLists-linux.txt for a reference that:         ║"
    echo "  ║  - Uses find_package(SFML) with system-installed SFML           ║"
    echo "  ║  - Stubs out the Windows audio for now                          ║"
    echo "  ║  - Removes MinGW-specific link flags                            ║"
    echo "  ║                                                                ║"
    echo "  ║  Usage: cp scripts/CMakeLists-linux.txt CMakeLists.txt          ║"
    echo "  ║         (then AudioManager will be a stub — no sound)           ║"
    echo "  ╚══════════════════════════════════════════════════════════════════╝"
    echo ""
    info "See scripts/CMakeLists-linux.txt for a Linux-ready build config."
}

# ──────────────────────────────────────────────────────────────────────────
# Linux — Debian / Ubuntu / Linux Mint / Pop!_OS
# ──────────────────────────────────────────────────────────────────────────
install_debian() {
    info "Detected Debian/Ubuntu-based distribution."
    require_root
    info "Updating package lists..."
    apt-get update -qq

    info "Installing build tools and dependencies..."
    apt-get install -y -qq \
        build-essential \
        cmake \
        make \
        pkg-config \
        libsfml-dev \
        libfreetype-dev \
        libgl1-mesa-dev \
        libglu1-mesa-dev \
        libx11-dev \
        libxrandr-dev \
        libxcb-randr0-dev \
        libxcb-image0-dev \
        libxcb-icccm4-dev \
        libxcb-keysyms1-dev \
        libxcb-shape0-dev \
        libxcb-xfixes0-dev \
        libxcb-sync-dev \
        libxcb-xinerama0-dev \
        libxcb-xkb-dev \
        libxkbcommon-dev \
        libxkbcommon-x11-dev \
        libudev-dev \
        libopenal-dev \
        libvorbis-dev \
        libflac-dev \
        libogg-dev

    success "All Linux (Debian/Ubuntu) dependencies installed."
    echo ""
    linux_cmake_warning
}

# ──────────────────────────────────────────────────────────────────────────
# Linux — Fedora / RHEL / CentOS
# ──────────────────────────────────────────────────────────────────────────
install_fedora() {
    info "Detected Fedora/RHEL-based distribution."
    require_root
    info "Installing build tools and dependencies..."
    dnf install -y \
        gcc-c++ \
        cmake \
        make \
        pkgconf-pkg-config \
        SFML-devel \
        freetype-devel \
        mesa-libGL-devel \
        mesa-libGLU-devel \
        libX11-devel \
        libXrandr-devel \
        libxcb-devel \
        xcb-util-devel \
        xcb-util-image-devel \
        xcb-util-icccm-devel \
        xcb-util-keysyms-devel \
        xcb-util-wm-devel \
        xcb-util-cursor-devel \
        libxkbcommon-devel \
        libxkbcommon-x11-devel \
        systemd-devel \
        openal-soft-devel \
        libvorbis-devel \
        flac-devel \
        libogg-devel

    success "All Linux (Fedora/RHEL) dependencies installed."
    echo ""
    linux_cmake_warning
}

# ──────────────────────────────────────────────────────────────────────────
# Linux — Arch Linux / Manjaro
# ──────────────────────────────────────────────────────────────────────────
install_arch() {
    info "Detected Arch Linux-based distribution."
    require_root
    info "Installing build tools and dependencies..."
    pacman -S --noconfirm \
        base-devel \
        cmake \
        make \
        pkgconf \
        sfml \
        freetype2 \
        mesa \
        libx11 \
        libxrandr \
        libxcb \
        xcb-util \
        xcb-util-image \
        xcb-util-icccm \
        xcb-util-keysyms \
        xcb-util-wm \
        xcb-util-cursor \
        libxkbcommon \
        libxkbcommon-x11 \
        libglvnd \
        openal \
        libvorbis \
        flac \
        libogg

    success "All Linux (Arch) dependencies installed."
    echo ""
    linux_cmake_warning
}

# ──────────────────────────────────────────────────────────────────────────
# Linux — Generic fallback (OpenSUSE, Alpine, etc.)
# ──────────────────────────────────────────────────────────────────────────
install_linux_generic() {
    warn "Unknown Linux distribution. Attempting generic install..."
    warn "Please ensure you have the following packages installed manually:"
    echo ""
    echo "  Required:"
    echo "    - g++ (or clang++) with C++17 support"
    echo "    - cmake >= 3.16"
    echo "    - make (or ninja)"
    echo "    - pkg-config"
    echo ""
    echo "  SFML 2.6 + dependencies:"
    echo "    - libsfml-dev / SFML-devel"
    echo "    - freetype / freetype2"
    echo "    - OpenGL / Mesa GL"
    echo "    - X11, Xrandr, XCB libraries"
    echo "    - OpenAL Soft"
    echo "    - libvorbis, flac, libogg"
    echo ""
    echo "  On your distribution, try:"
    echo ""
    echo "  Debian/Ubuntu:  apt install build-essential cmake libsfml-dev ..."
    echo "  Fedora:         dnf install gcc-c++ cmake SFML-devel ..."
    echo "  Arch:           pacman -S base-devel cmake sfml ..."
    echo "  Alpine:         apk add build-base cmake sfml-dev ..."
    echo "  OpenSUSE:       zypper install gcc-c++ cmake libsfml-devel ..."
    echo ""
}

# ──────────────────────────────────────────────────────────────────────────
# Linux — detect distro and dispatch
# ──────────────────────────────────────────────────────────────────────────
install_linux() {
    info "Detecting Linux distribution..."

    if   cmd_exists apt-get; then install_debian
    elif cmd_exists dnf;     then install_fedora
    elif cmd_exists pacman;  then install_arch
    else                          install_linux_generic
    fi

    # Make script executable (just in case)
    chmod +x "$0" 2>/dev/null || true
}

# ──────────────────────────────────────────────────────────────────────────
# Windows — MSYS2 / MinGW-w64
# ──────────────────────────────────────────────────────────────────────────
install_windows_msys2() {
    info "Detected MSYS2 / MinGW-w64 environment."
    echo ""
    echo "  This script detected Windows with MSYS2. Dependencies should be"
    echo "  installed using MSYS2's pacman."
    echo ""

    # Check that we are in an MSYS2 shell (MSYSTEM is set)
    if [[ -z "${MSYSTEM:-}" ]]; then
        warn "MSYSTEM variable not set. Are you sure you're in an MSYS2 shell?"
        warn "  Open 'MSYS2 UCRT64' or 'MSYS2 MINGW64' from Start Menu."
        echo ""
    else
        info "MSYS2 environment: $MSYSTEM"
    fi

    echo "  Installing packages for: ${MSYSTEM:-MINGW64/UCRT64}"
    echo ""

    # Determine the correct package prefix
    local prefix
    case "${MSYSTEM:-MINGW64}" in
        UCRT64)  prefix="mingw-w64-ucrt-x86_64" ;;
        CLANG64)  prefix="mingw-w64-clang-x86_64" ;;
        MINGW64|*) prefix="mingw-w64-x86_64" ;;
    esac

    info "Using package prefix: ${prefix}"

    # Install via pacman (MSYS2's package manager)
    pacman -S --noconfirm \
        "${prefix}-gcc" \
        "${prefix}-cmake" \
        "${prefix}-make" \
        "${prefix}-pkg-config" \
        "${prefix}-freetype" \
        "${prefix}-openal" \
        "${prefix}-flac" \
        "${prefix}-libvorbis" \
        "${prefix}-libogg"

    echo ""
    info "SFML 2.6 is bundled with the project source (sfml/ directory)."
    info "No need to install SFML separately on Windows."
    echo ""

    # Download SFML if not already present (check the lib directory)
    if [[ ! -f "sfml/lib/libsfml-graphics-s.a" ]]; then
        warn "Bundled SFML static libraries not found in sfml/lib/."
        warn "Please ensure the sfml/ directory is intact from the source distribution."
    else
        success "Bundled SFML found at sfml/lib/ — ready to build."
    fi

    # Verify g++ is available
    if cmd_exists g++; then
        success "g++ found: $(g++ --version | head -1)"
    else
        error "g++ not found after installation. Check your MSYS2 setup."
        echo "  Try: pacman -S ${prefix}-gcc"
        exit 1
    fi

    # Verify cmake is available
    if cmd_exists cmake; then
        success "CMake found: $(cmake --version | head -1)"
    else
        error "CMake not found after installation."
        exit 1
    fi

    # Verify mingw32-make is available
    if cmd_exists mingw32-make; then
        success "mingw32-make found."
    elif cmd_exists make; then
        success "make found (use instead of mingw32-make)."
    else
        warn "Neither make nor mingw32-make found."
    fi

    echo ""
    info "Build the game with:"
    echo ""
    echo "    cd dino_game"
    echo "    mkdir -p build && cd build"
    echo "    cmake .. -G \"MinGW Makefiles\""
    echo "    mingw32-make -j\$(nproc 2>/dev/null || echo 4)"
    echo ""
    echo "  Then run:"
    echo "    ./DinoRunner.exe"
    echo ""

    success "Windows (MSYS2) dependencies installed."
}

# ──────────────────────────────────────────────────────────────────────────
# Windows — Cygwin (not fully supported)
# ──────────────────────────────────────────────────────────────────────────
install_windows_cygwin() {
    warn "Cygwin detected. DinoRunner is designed for MSYS2/MinGW-w64."
    echo ""
    echo "  For Cygwin, please install:"
    echo "    - gcc-g++"
    echo "    - cmake"
    echo "    - make"
    echo "    - libsfml-devel (if available)"
    echo "    - libfreetype-devel"
    echo ""
    echo "  Using Cygwin's setup.exe or:"
    echo "    apt-cyg install gcc-g++ cmake make libsfml-devel libfreetype-devel"
    echo ""
    echo "  NOTE: Audio uses Win32 waveOut API which works under Cygwin,"
    echo "  but SFML static linking may require additional configuration."
}

# ──────────────────────────────────────────────────────────────────────────
# macOS — (experimental / not yet supported)
# ──────────────────────────────────────────────────────────────────────────
install_macos() {
    warn "macOS detected — not officially supported."
    echo ""
    echo "  The game uses Win32 waveOut API for audio and MinGW-specific"
    echo "  ABI workarounds. A full port would require replacing audio"
    echo "  with OpenAL or SDL, and using native Clang + SFML frameworks."
    echo ""
    echo "  For development/testing, you can install dependencies:"
    echo ""
    if cmd_exists brew; then
        echo "    brew install cmake sfml freetype openal-soft"
        echo "    brew install libvorbis flac libogg"
    else
        echo "  Install Homebrew first: https://brew.sh"
    fi
    echo ""
    echo "  Then modify CMakeLists.txt to use system SFML instead of bundled."
    echo "  Audio will need to be ported from waveOut to CoreAudio or OpenAL."
}

# ──────────────────────────────────────────────────────────────────────────
# Post-install verification (Linux and MSYS2)
# ──────────────────────────────────────────────────────────────────────────
post_install_check() {
    echo ""
    echo "═══════════════════════════════════════════════════════════════"
    info "Verifying installation..."
    echo "═══════════════════════════════════════════════════════════════"
    echo ""

    local errors=0

    # Check compiler
    if cmd_exists g++; then
        success "g++:         $(g++ --version | head -1)"
    elif cmd_exists clang++; then
        success "clang++:     $(clang++ --version | head -1)"
    else
        error "No C++17 compiler found (g++ or clang++)"
        errors=$((errors + 1))
    fi

    # Check CMake
    if cmd_exists cmake; then
        local cmake_ver
        cmake_ver=$(cmake --version | head -1 | grep -oP '\d+\.\d+' | head -1)
        # Compare using sort -V (no external bc needed)
        if [[ -n "$cmake_ver" && "$(printf '%s\n' '3.16' "$cmake_ver" | sort -V | head -1)" == '3.16' ]]; then
            success "CMake:       $(cmake --version | head -1)"
        else
            warn "CMake version may be too old (need ≥ 3.16): $(cmake --version | head -1)"
        fi
    else
        error "CMake not found."
        errors=$((errors + 1))
    fi

    # Check make
    if cmd_exists mingw32-make; then
        success "mingw32-make: $(mingw32-make --version 2>&1 | head -1)"
    elif cmd_exists make; then
        success "make:        $(make --version 2>&1 | head -1)"
    else
        error "No make/mingw32-make found."
        errors=$((errors + 1))
    fi

    # Check pkg-config (Linux)
    if [[ "$PLATFORM" == "linux" ]]; then
        if cmd_exists pkg-config; then
            success "pkg-config:  found"
            # Check SFML via pkg-config (optional)
            if pkg-config --exists sfml-graphics 2>/dev/null; then
                success "SFML:        $(pkg-config --modversion sfml-graphics)"
            else
                warn "SFML not found via pkg-config. May need manual CMake path config."
            fi
        else
            warn "pkg-config not found."
            errors=$((errors + 1))
        fi
    fi

    # Check bundled SFML (Windows)
    if [[ "$PLATFORM" == "windows_msys2" ]]; then
        local sfml_dir
        sfml_dir="$(cd "$(dirname "$0")/.." && pwd)/sfml"
        if [[ -f "$sfml_dir/lib/libsfml-graphics-s.a" ]]; then
            success "SFML static: $sfml_dir (bundled, ready)"
        else
            warn "Bundled SFML not found. Did you clone the full repository?"
        fi
    fi

    echo ""
    if [[ $errors -eq 0 ]]; then
        success "All required tools found!"
        echo ""
        echo "  Next steps:"
        echo ""
        echo "    cd dino_game"
        echo "    mkdir -p build && cd build"
        echo ""
        if [[ "$PLATFORM" == "windows_msys2" ]]; then
            echo "    cmake .. -G \"MinGW Makefiles\""
            echo "    mingw32-make -j\$(nproc 2>/dev/null || echo 4)"
        else
            echo "    cmake .."
            echo "    make -j\$(nproc)"
        fi
        echo ""
        echo "    ./DinoRunner    # or DinoRunner.exe on Windows"
        echo ""
    else
        error "$errors critical dependency/dependencies missing. Please install them."
        exit 1
    fi
}

# ──────────────────────────────────────────────────────────────────────────
# Main entry point
# ──────────────────────────────────────────────────────────────────────────
main() {
    echo ""
    echo " ██████╗ ██╗███╗   ██╗ ██████╗ ██████╗ ██╗   ██╗███╗   ██╗███╗   ██╗███████╗██████╗ "
    echo " ██╔══██╗██║████╗  ██║██╔════╝ ██╔══██╗██║   ██║████╗  ██║████╗  ██║██╔════╝██╔══██╗"
    echo " ██║  ██║██║██╔██╗ ██║██║  ███╗██████╔╝██║   ██║██╔██╗ ██║██╔██╗ ██║█████╗  ██████╔╝"
    echo " ██║  ██║██║██║╚██╗██║██║   ██║██╔══██╗██║   ██║██║╚██╗██║██║╚██╗██║██╔══╝  ██╔══██╗"
    echo " ██████╔╝██║██║ ╚████║╚██████╔╝██║  ██║╚██████╔╝██║ ╚████║██║ ╚████║███████╗██║  ██║"
    echo " ╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝"
    echo "                                    Dependency Installer"
    echo ""

    info "Detected platform: ${PLATFORM}"
    echo ""

    case "$PLATFORM" in
        linux)
            install_linux
            post_install_check
            ;;
        windows_msys2)
            # For MSYS2, the script needs to be run from inside the dino_game directory
            # so the bundled SFML check works. cd to the script's directory.
            cd "$(dirname "$0")/.."
            install_windows_msys2
            post_install_check
            ;;
        windows_cygwin)
            install_windows_cygwin
            ;;
        macos)
            install_macos
            ;;
        *)
            error "Unknown platform '${PLATFORM}'."
            echo ""
            echo "  Supported platforms:"
            echo "    Linux  (Debian/Ubuntu, Fedora, Arch)"
            echo "    Windows (MSYS2 UCRT64 / MINGW64)"
            echo ""
            echo "  You can also manually install:"
            echo "    - C++17 compiler (g++ or clang++)"
            echo "    - CMake >= 3.16"
            echo "    - GNU Make"
            echo "    - SFML 2.6 development libraries"
            echo "    - FreeType"
            exit 1
            ;;
    esac

    echo ""
    success "DinoRunner dependency setup complete!"
    echo ""
}

# Run
main "$@"
