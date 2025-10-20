cmake_minimum_required(VERSION 3.23)
project(ManagedScripts)

add_custom_target(BuildManagedScripts ALL
    COMMAND dotnet build ManagedScripts.csproj -c Release
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
