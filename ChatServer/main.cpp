#include "IOCPServer.h"

int main() {
	IOCPserver server;
	
	if (!server.Init(8, 5555)) {
		return 0;
	}

	server.Run();

	while (true) {
		Sleep(100);
	}
}