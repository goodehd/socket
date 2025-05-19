#include "IOCPServer.h"
#include "Socket.h"

int main() {
	IOCPserver server;
	
	if (!server.Init()) {
		return 0;
	}


}