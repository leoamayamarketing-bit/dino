#!/usr/bin/env bash
# ============================================================================
# DinoRunner — Build & Package Script (Bash / MSYS2)
# ============================================================================
# Cleans the build directory, recompiles from scratch, and packages the
# executable into a dist/ folder ready for distribution.
#
# Usage:
#   cd dino_game
#   bash scripts/package.sh
#
# Output:
#   dist/
#   ├── DinoRunner.exe      # Standalone executable (no DLLs needed)
#   └── README.txt           # Quick-start instructions
# ============================================================================

set -euo pipefail

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

info()    { echo -e "${CYAN}[INFO]${NC}  $*"; }
success() { echo -e "${GREEN}[OK]${NC}    $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $*"; }
error()   { echo -e "${RED}[ERROR]${NC} $*"; }

# ──────────────────────────────────────────────────────────────────────────
# Configuration
# ──────────────────────────────────────────────────────────────────────────
PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
DIST_DIR="$PROJECT_DIR/dist"
EXECUTABLE="DinoRunner.exe"
CMAKE_GENERATOR="MinGW Makefiles"
BUILD_JOBS="${NUMBER_OF_PROCESSORS:-$(nproc 2>/dev/null || echo 4)}"

# ──────────────────────────────────────────────────────────────────────────
# Step 1: Clean build directory
# ──────────────────────────────────────────────────────────────────────────
step_clean() {
    echo ""
    echo "═══════════════════════════════════════════════════════════════"
    info "Step 1/4: Cleaning build directory..."
    echo "═══════════════════════════════════════════════════════════════"

    if [[ -d "$BUILD_DIR" ]]; then
        rm -rf "$BUILD_DIR"
        success "Build directory removed: $BUILD_DIR"
    else
        info "Build directory does not exist — nothing to clean."
    fi

    mkdir -p "$BUILD_DIR"
    success "Build directory created: $BUILD_DIR"
}

# ──────────────────────────────────────────────────────────────────────────
# Step 2: CMake configure
# ──────────────────────────────────────────────────────────────────────────
step_configure() {
    echo ""
    echo "═══════════════════════════════════════════════════════════════"
    info "Step 2/4: Configuring with CMake..."
    echo "═══════════════════════════════════════════════════════════════"

    cd "$BUILD_DIR"
    cmake "$PROJECT_DIR" -G "$CMAKE_GENERATOR" 2>&1

    success "CMake configuration complete."
}

# ──────────────────────────────────────────────────────────────────────────
# Step 3: Build
# ──────────────────────────────────────────────────────────────────────────
step_build() {
    echo ""
    echo "═══════════════════════════════════════════════════════════════"
    info "Step 3/4: Building DinoRunner ($BUILD_JOBS parallel jobs)..."
    echo "═══════════════════════════════════════════════════════════════"

    cd "$BUILD_DIR"
    mingw32-make -j"$BUILD_JOBS" 2>&1

    if [[ ! -f "$BUILD_DIR/$EXECUTABLE" ]]; then
        error "Build succeeded but $EXECUTABLE not found!"
        exit 1
    fi

    success "Build complete: $EXECUTABLE"
}

# ──────────────────────────────────────────────────────────────────────────
# Step 4: Package
# ──────────────────────────────────────────────────────────────────────────
step_package() {
    echo ""
    echo "═══════════════════════════════════════════════════════════════"
    info "Step 4/4: Packaging into dist/..."
    echo "═══════════════════════════════════════════════════════════════"

    # Clean dist folder
    rm -rf "$DIST_DIR"
    mkdir -p "$DIST_DIR"

    # Copy executable
    cp "$BUILD_DIR/$EXECUTABLE" "$DIST_DIR/$EXECUTABLE"
    success "Copied: $EXECUTABLE"

    # Get build date
    local build_date
    build_date=$(date +%Y-%m-%d 2>/dev/null || echo "$(date)")

    # Create README.txt
    cat > "$DIST_DIR/README.txt" << EOF
============================================
      DinoRunner: Primal Rush
             v1.0.0
============================================

HOW TO RUN:
  Double-click DinoRunner.exe
  No installation needed.

CONTROLS:
  Space / W / Up    - Jump (hold for higher jump)
  W + W (mid-air)   - Double jump
  S / Down          - Crouch / Slide
  Shift             - Dash
  Escape            - Pause / Menu
  Enter             - Confirm / Select

FEATURES:
  - 3 playable dinosaurs (T-Rex, Raptor, Triceratops)
  - 5 levels (Desert Day, Desert Night, Cave, Volcano, Infinite)
  - Power-ups: Shield, Magnet, Double Points, Speed Boost, Extra Life
  - Configurable key bindings (from the Options menu)
  - Procedurally generated graphics and audio
  - Everything is in the .exe - no external files needed!

REQUIREMENTS:
  - Windows 7 or later (64-bit)
  - OpenGL-compatible graphics card
  - No additional DLLs or runtime dependencies

BUILD INFO:
  - C++17, SFML 2.6 (static), MinGW-w64 GCC
  - Compiled: ${build_date}

TIP: Press F11 to toggle fullscreen.
EOF

    success "Created: README.txt"

    # Display package info
    local exe_size
    exe_size=$(du -h "$DIST_DIR/$EXECUTABLE" | cut -f1)
    local dist_size
    dist_size=$(du -h "$DIST_DIR" | tail -1 | cut -f1)

    echo ""
    echo "  Package contents:"
    echo "    $DIST_DIR/"
    echo "    ├── $EXECUTABLE  ($exe_size)"
    echo "    └── README.txt"
    echo ""
    success "Package size: $dist_size"
}

# ──────────────────────────────────────────────────────────────────────────
# Verify: check DLL dependencies
# ──────────────────────────────────────────────────────────────────────────
step_verify() {
    echo ""
    echo "═══════════════════════════════════════════════════════════════"
    info "Verifying executable..."
    echo "═══════════════════════════════════════════════════════════════"

    # Check file exists
    if [[ ! -f "$DIST_DIR/$EXECUTABLE" ]]; then
        error "Package verification failed: $EXECUTABLE not found in dist/"
        exit 1
    fi

    # Check DLL dependencies (MinGW DLLs should NOT be present)
    if command -v objdump &>/dev/null; then
        local dll_warnings=0
        for dll in libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll; do
            if objdump -p "$DIST_DIR/$EXECUTABLE" 2>/dev/null | grep -qi "$dll"; then
                warn "$dll is still a dependency! Rebuild with static linking."
                dll_warnings=$((dll_warnings + 1))
            fi
        done
        if [[ $dll_warnings -eq 0 ]]; then
            success "No MinGW DLL dependencies — executable is fully standalone."
        fi
    else
        info "objdump not available — skipping DLL verification."
    fi

    echo ""
    success "Package ready: $DIST_DIR"
    echo ""
    info "You can now distribute the contents of: $DIST_DIR"
    echo ""
}

# ──────────────────────────────────────────────────────────────────────────
# Main
# ──────────────────────────────────────────────────────────────────────────
main() {
    echo ""
    echo " ██████╗ ██╗███╗   ██╗ ██████╗ ██████╗ ██╗   ██╗███╗   ██╗███╗   ██╗███████╗██████╗ "
    echo " ██╔══██╗██║████╗  ██║██╔════╝ ██╔══██╗██║   ██║████╗  ██║████╗  ██║██╔════╝██╔══██╗"
    echo " ██║  ██║██║██╔██╗ ██║██║  ███╗██████╔╝██║   ██║██╔██╗ ██║██╔██╗ ██║█████╗  ██████╔╝"
    echo " ██║  ██║██║██║╚██╗██║██║   ██║██╔══██╗██║   ██║██║╚██╗██║██║╚██╗██║██╔══╝  ██╔══██╗"
    echo " ██████╔╝██║██║ ╚████║╚██████╔╝██║  ██║╚██████╔╝██║ ╚████║██║ ╚████║███████╗██║  ██║"
    echo " ╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝"
    echo "                                     Build & Package Script"
    echo ""

    info "Project directory: $PROJECT_DIR"
    info "Build directory:   $BUILD_DIR"
    info "Output directory:  $DIST_DIR"
    info "Build jobs:        $BUILD_JOBS"
    echo ""

    step_clean
    step_configure
    step_build
    step_package
    step_verify

    echo ""
    success "DinoRunner packaged successfully!"
    echo ""
}

main "$@"
