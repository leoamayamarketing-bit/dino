# =============================================================================
# DinoRunner — Unified CMake Configuration (Windows + Linux)
# =============================================================================
# This CMakeLists.txt auto-detects the operating system and configures
# the build accordingly:
#
#   Windows (MinGW-w64 / MSYS2):
#     - Uses bundled SFML 2.6 static libraries (in sfml/)
#     - Links Win32 libraries (winmm, ws2_32, opengl32, etc.)
#     - Compiles abi_compat.cpp for MinGW ABI compatibility
#     - Audio: Win32 waveOut API (via winmm)
#
#   Linux:
#     - Uses system-installed SFML via find_package()
#     - Compiles real AudioManager.cpp with SFML Audio backend (wraps OpenAL)
#     - Excludes abi_compat.cpp (MinGW-specific)
#     - Links sfml-audio, pthread, dl
#     - Audio: SFML Audio (OpenAL) — procedural sounds + 4 music themes
#
# =============================================================================

cmake_minimum_required(VERSION 3.16)
project(DinoRunner VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ─────────────────────────────────────────────────────────────────────────────
# Source files (common to both platforms)
# ─────────────────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/core/Game.cpp
    src/core/InputManager.cpp
    src/core/KeyBindings.cpp
    src/core/AssetManager.cpp
    src/core/AudioManager.cpp
    src/systems/RenderSystem.cpp
    src/systems/PhysicsSystem.cpp
    src/systems/CollisionSystem.cpp
    src/systems/AnimationSystem.cpp
    src/systems/PlayerControlSystem.cpp
    src/systems/EnemyAISystem.cpp
    src/systems/CoinSystem.cpp
    src/systems/PowerUpSystem.cpp
    src/systems/ParticleSystem.cpp
    src/systems/ParallaxSystem.cpp
    src/entities/PlayerFactory.cpp
    src/entities/EnemyFactory.cpp
    src/entities/ObstacleFactory.cpp
    src/levels/LevelManager.cpp
    src/levels/DesertDayLevel.cpp
    src/levels/DesertNightLevel.cpp
    src/levels/CaveLevel.cpp
    src/levels/VolcanoLevel.cpp
    src/levels/InfiniteLevel.cpp
    src/ui/HUD.cpp
    src/ui/Menu.cpp
    src/ui/PauseMenu.cpp
    src/ui/GameOverScreen.cpp
    src/core/AchievementSystem.cpp
    src/core/HighScoreManager.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# ─────────────────────────────────────────────────────────────────────────────
# Platform detection
# ─────────────────────────────────────────────────────────────────────────────
if(WIN32)
    message(STATUS "Detected platform: Windows — using bundled SFML + Win32 audio")

    # Use SFML estático
    set(SFML_STATIC_LIBRARIES TRUE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SFML_STATIC)

    # Directorios de inclusión
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/sfml/include
    )

    # Enlazar librerías SFML (estáticas, solo las necesarias)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/sfml/lib/libsfml-graphics-s.a
        ${CMAKE_SOURCE_DIR}/sfml/lib/libsfml-window-s.a
        ${CMAKE_SOURCE_DIR}/sfml/lib/libsfml-system-s.a
        # Dependencias de SFML
        ${CMAKE_SOURCE_DIR}/sfml/lib/libfreetype.a
    )

    # Audio: usamos Win32 waveOut API directamente (en winmm.dll)
    # No se necesitan las librerías SFML audio ni sus codecs

    # Librerías de Windows necesarias
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ws2_32
        winmm
        opengl32
        gdi32
        ole32
        oleaut32
        uuid
    )

    # Opciones de compilación
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)

    # ABI compat para MinGW
    target_sources(${PROJECT_NAME} PRIVATE src/abi_compat.cpp)

    # Enlazar estáticamente las runtime de GCC (libgcc, libstdc++, libwinpthread)
    target_link_options(${PROJECT_NAME} PRIVATE
        -static-libgcc
        -static-libstdc++
        -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
    )

elseif(UNIX)
    message(STATUS "Detected platform: Linux — using system SFML + SFML Audio (OpenAL)")

    # ─────────────────────────────────────────────────────────────────────────
    # Find system SFML (installed via package manager)
    #   sfml-audio uses OpenAL under the hood for PCM playback
    # ─────────────────────────────────────────────────────────────────────────
    find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
    )

    # ─────────────────────────────────────────────────────────────────────────
    # Linux system libraries
    # ─────────────────────────────────────────────────────────────────────────
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FREETYPE freetype2 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${FREETYPE_LIBRARIES}
        pthread
        dl
    )

    # ─────────────────────────────────────────────────────────────────────────
    # Compiler options
    # ─────────────────────────────────────────────────────────────────────────
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)

    message("")
    message("╔══════════════════════════════════════════════════════════════╗")
    message("║  DinoRunner — Linux build (SFML Audio enabled)             ║")
    message("╠══════════════════════════════════════════════════════════════╣")
    message("║  Audio backend: SFML Audio (OpenAL wrapped via sfml-audio) ║")
    message("║  Procedural sound effects and 4 music themes active.       ║")
    message("╚══════════════════════════════════════════════════════════════╝")
    message("")

else()
    message(FATAL_ERROR "Unsupported platform. DinoRunner supports Windows and Linux only.")
endif()
