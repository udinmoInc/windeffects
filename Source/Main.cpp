#include "Editor/Editor.hpp"
#include "Core/Logger.hpp"
#include <iostream>
#include <exception>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    HouseEngine::Logger::Init();
    
    try {
        HE_INFO("WindEffects Engine bootstrapping...");
        HouseEngine::Editor editor;
        editor.Run();
    } catch (const std::exception& e) {
        // Report fatal exception with a native dialog popup
        HouseEngine::Logger::ReportError(
            "Engine Crash",
            std::string("A fatal exception occurred: ") + e.what(),
            true
        );
        return -1;
    }
    
    HouseEngine::Logger::Shutdown();
    return 0;
}
