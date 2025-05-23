#define _CRTDBG_MAP_ALLOC
#include <iostream>
#include <crtdbg.h>

#include "IOCPServer.h"
#include <Windows.h>

int main() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(193);

    IOCPserver server;
    if (!server.Init(8, 5555)) {
        return 0;
    }

    server.Run();

    while (true) {
        std::string input;
        std::cin >> input;
        if (input == "q") {
            server.Stop();
            break;
        }
    }

    return 0;
}