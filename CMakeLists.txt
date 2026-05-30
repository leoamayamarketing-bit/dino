cmake_minimum_required(VERSION 3.16)
project(DinoRunner VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Usar SFML estático
set(SFML_STATIC_LIBRARIES TRUE)
add_compile_definitions(SFML_STATIC)

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
    src/ui/GameOverScreen.cpp
    src/abi_compat.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

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

# Enlazar estáticamente las runtime de GCC (libgcc, libstdc++, libwinpthread)
# para que el .exe no dependa de DLLs externas de MinGW al ejecutarse.
# Esto evita errores como "cidec_cvt_utf8 no encontrado en libstdc++-6.dll"
# cuando se ejecuta fuera del shell MSYS2.
target_link_options(${PROJECT_NAME} PRIVATE
    -static-libgcc
    -static-libstdc++
    -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
)

