#include "Game.h"
#include "Utility/Logger.h"

int main(int argc, char** argv) {
    // Suppress unused parameter warnings
    (void)argc;
    (void)argv;

    // Set log level for development - TRACE shows everything
    Engine::Logger::Get().SetLogLevel(Engine::LogLevel::Trace);

    // Enable file logging to capture crash info
    Engine::Logger::Get().EnableFileLogging("engine_log.txt");

    try {
        // Create and run the game
        Game game;
        game.Run();
    }
    catch (const std::exception& e) {
        Engine::Logger::Get().Critical("FATAL EXCEPTION: ", e.what());
        return -1;
    }
    catch (...) {
        Engine::Logger::Get().Critical("FATAL UNKNOWN EXCEPTION");
        return -1;
    }

    return 0;
}