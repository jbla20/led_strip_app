#include <iostream>
#include "app.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    App app = App();
    if (!app.init())
    {
        std::cout << "[Fatal] Failed to create the app." << std::endl;
        return EXIT_FAILURE;
    }

    app.run();
    return EXIT_SUCCESS;
}
