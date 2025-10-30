# ====================================
# Game Executable Configuration
# ====================================
message(STATUS "Configuring Game executable...")

# Create game executable
add_executable(GameEngine
    Main.cpp
    Game.cpp
    Game.h
)

# Set target properties
set_target_properties(GameEngine PROPERTIES
    FOLDER "Game"
    OUTPUT_NAME "GameEngine"
)

# Link engine library (which transitively links External and Mono)
target_link_libraries(GameEngine PRIVATE 
    EngineLib
    ${MONO_LIBRARIES}
)

# Set as startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} 
             PROPERTY VS_STARTUP_PROJECT GameEngine)

# ====================================
# Windows-specific Setup
# ====================================
if(WIN32)
    # ====================================
    # Copy FMOD DLLs
    # ====================================
    if(FMOD_FOUND)
        add_custom_command(TARGET GameEngine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/External/fmod/dll/x64/fmod.dll"
                "$<TARGET_FILE_DIR:GameEngine>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/External/fmod/dll/x64/fmodL.dll"
                "$<TARGET_FILE_DIR:GameEngine>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/External/fmod/dll/x64/fmodstudio.dll"
                "$<TARGET_FILE_DIR:GameEngine>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/External/fmod/dll/x64/fmodstudioL.dll"
                "$<TARGET_FILE_DIR:GameEngine>"
            COMMENT "Copying FMOD DLLs to output directory"
        )
        message(STATUS "  FMOD DLLs will be copied")
    else()
        message(STATUS "  FMOD DLLs not copied (FMOD not found)")
    endif()
    
    # ====================================
    # Copy Bundled Mono Runtime
    # ====================================
    # Create mono directory structure
    add_custom_command(TARGET GameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:GameEngine>/mono/bin"
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:GameEngine>/mono/lib/mono/4.5"
        COMMENT "Creating Mono directories..."
    )
    
    # Copy Mono runtime DLLs (mono-2.0-sgen.dll, MonoPosixHelper.dll)
    add_custom_command(TARGET GameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MONO_ROOT}/bin/mono-2.0-sgen.dll"
            "$<TARGET_FILE_DIR:GameEngine>/mono/bin/mono-2.0-sgen.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MONO_ROOT}/bin/MonoPosixHelper.dll"
            "$<TARGET_FILE_DIR:GameEngine>/mono/bin/MonoPosixHelper.dll"
        COMMENT "Copying Mono runtime DLLs..."
    )
    
    # Copy .NET 4.5 BCL assemblies (mscorlib.dll, System.dll, etc.)
    add_custom_command(TARGET GameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MONO_ROOT}/lib/mono/4.5"
            "$<TARGET_FILE_DIR:GameEngine>/mono/lib/mono/4.5"
        COMMENT "Copying .NET 4.5 BCL assemblies..."
    )
    
    message(STATUS "  Bundled Mono (from ${MONO_ROOT}) will be copied to: bin/mono/")
    
    # ====================================
    # Copy C# GameScripts DLL
    # ====================================
    set(GAMESCRIPTS_SEARCH_PATHS
        "${CMAKE_BINARY_DIR}/bin/GameScripts.dll"
        "${CMAKE_BINARY_DIR}/GameScripts.dll"
        "${CMAKE_SOURCE_DIR}/Scripts/bin/Debug/net6.0/GameScripts.dll"
        "${CMAKE_SOURCE_DIR}/Scripts/bin/Release/net6.0/GameScripts.dll"
        "${CMAKE_SOURCE_DIR}/build/bin/GameScripts.dll"
    )
    
    set(FOUND_GAMESCRIPTS FALSE)
    foreach(SCRIPT_PATH ${GAMESCRIPTS_SEARCH_PATHS})
        if(EXISTS "${SCRIPT_PATH}")
            add_custom_command(TARGET GameEngine POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SCRIPT_PATH}"
                    "$<TARGET_FILE_DIR:GameEngine>/GameScripts.dll"
                COMMENT "Copying GameScripts.dll from: ${SCRIPT_PATH}"
            )
            message(STATUS "  GameScripts.dll will be copied from: ${SCRIPT_PATH}")
            set(FOUND_GAMESCRIPTS TRUE)
            break()
        endif()
    endforeach()
    
    if(NOT FOUND_GAMESCRIPTS)
        message(WARNING "  GameScripts.dll not found in search paths - build may fail at runtime")
    endif()

endif()

# ====================================
# Copy Resources
# ====================================
add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Resources"
        "$<TARGET_FILE_DIR:GameEngine>/Resources"
    COMMENT "Copying Resources..."
)

# Create asset pipeline directories
add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory 
        "$<TARGET_FILE_DIR:GameEngine>/Resources/Descriptors"
    COMMAND ${CMAKE_COMMAND} -E make_directory 
        "$<TARGET_FILE_DIR:GameEngine>/Resources/Compiled"
    COMMAND ${CMAKE_COMMAND} -E make_directory 
        "$<TARGET_FILE_DIR:GameEngine>/Resources/DB"
    COMMENT "Creating asset pipeline directories..."
)

message(STATUS "✓ Game executable configured successfully")
