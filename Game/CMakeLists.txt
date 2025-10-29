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

# Link engine library (which transitively links External)
target_link_libraries(GameEngine PRIVATE EngineLib)

# Set as startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} 
             PROPERTY VS_STARTUP_PROJECT GameEngine)

# ====================================
# Windows-specific DLL copying
# ====================================
if(WIN32)
    # Copy FMOD DLLs to executable directory
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
    else()
        message(STATUS "  FMOD DLLs not copied (FMOD not found)")
    endif()
    
    # ====================================
    # Copy Mono Runtime DLLs
    # ====================================
    set(MONO_ROOT "C:/Program Files/Mono" CACHE PATH "Path to Mono installation")
    
    if(EXISTS "${MONO_ROOT}/bin")
        # Copy essential Mono runtime DLLs
        set(MONO_DLLS
            "mono-2.0-sgen.dll"
            "MonoPosixHelper.dll"
        )
        
        foreach(DLL ${MONO_DLLS})
            if(EXISTS "${MONO_ROOT}/bin/${DLL}")
                add_custom_command(TARGET GameEngine POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MONO_ROOT}/bin/${DLL}"
                        "$<TARGET_FILE_DIR:GameEngine>"
                    COMMENT "Copying ${DLL} to executable directory"
                )
            else()
                message(WARNING "Mono DLL not found: ${MONO_ROOT}/bin/${DLL}")
            endif()
        endforeach()
        
        # Copy Mono library directory for .NET assemblies
        if(EXISTS "${MONO_ROOT}/lib/mono/4.5")
            add_custom_command(TARGET GameEngine POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:GameEngine>/mono/lib/4.5"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${MONO_ROOT}/lib/mono/4.5"
                    "$<TARGET_FILE_DIR:GameEngine>/mono/lib/4.5"
                COMMENT "Copying Mono .NET assemblies to executable directory"
            )
        else()
            message(WARNING "Mono lib directory not found: ${MONO_ROOT}/lib/mono/4.5")
        endif()
        
        message(STATUS "  Mono runtime files will be copied from: ${MONO_ROOT}")
    else()
        message(WARNING "  Mono installation not found at: ${MONO_ROOT}")
        message(WARNING "  Please install Mono or set MONO_ROOT to correct path")
    endif()
endif()

# ====================================
# Copy Resources and Create Directories
# ====================================

# Copy Resources folder to build directory
add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying Resources to: $<TARGET_FILE_DIR:GameEngine>/Resources"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Resources"
        "$<TARGET_FILE_DIR:GameEngine>/Resources"
    COMMENT "Copying Resources to executable directory"
)

# Create asset pipeline directories (even if empty)
add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory 
        "$<TARGET_FILE_DIR:GameEngine>/Resources/Descriptors"
    COMMAND ${CMAKE_COMMAND} -E make_directory 
        "$<TARGET_FILE_DIR:GameEngine>/Resources/Compiled"
    COMMAND ${CMAKE_COMMAND} -E make_directory 
        "$<TARGET_FILE_DIR:GameEngine>/Resources/DB"
    COMMENT "Creating asset pipeline directories"
)

message(STATUS "Game executable configured successfully")
