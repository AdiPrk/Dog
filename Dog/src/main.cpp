#include <PCH/pch.h>
#include "Engine.h"

int main() {
    Engine engine{};

    try {
        engine.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}